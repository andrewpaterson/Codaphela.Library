#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "IndexedFile.h"
#include "IndexTreeFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName)
{
	return Init(pcDurableFileControl, szRootFileName, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc)
{
	//The DurableFileControl must be begun before .Init is called and should be ended afterwards.

	BOOL	bResult;

	CIndexTree::Init(pcMalloc, sizeof(CIndexTreeNodeFile), sizeof(SIndexTreeChildFile));

	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, "IDAT", "Index.IDX", "_Index.IDX");

	bResult = InitRoot(szRootFileName);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::FakeInit(void)
{
	CIndexTree::Init(&gcSystemAllocator, sizeof(CIndexTreeNodeFile), sizeof(SIndexTreeChildFile));
	mpcRoot = NULL;
	mpcDurableFileControl = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::Kill(void)
{
	mszRootFileName.Kill();
	mcIndexFiles.Kill();
	mpcDurableFileControl = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::InitRoot(char* szRootFileName)
{
	BOOL			bRootIndexExists;
	CFileIndex		cRootFileIndex;
	CIndexedFile*	pcRootIndexFile;
	char			pvBuffer[8 KB];
	int				iNodeSize;
	BOOL			bResult;
	filePos			iFilePos;

	mszRootFileName.Init(szRootFileName);
	cRootFileIndex = LoadRootFileIndex(szRootFileName);

	bRootIndexExists = cRootFileIndex.HasFile();
	if (bRootIndexExists)
	{
		//The data size on the root is always zero.
		mpcRoot = AllocateRoot(cRootFileIndex);

		iNodeSize = mpcRoot->CalculateBufferSize();
		if (iNodeSize > 8 KB)
		{
			return FALSE;
		}

		pcRootIndexFile = mcIndexFiles.GetOrCreateFile(iNodeSize);  //Dangerous.  Should be a Get.
		pcRootIndexFile->Read(cRootFileIndex.mulliFilePos, pvBuffer, iNodeSize);

		mpcRoot->Init(this, NULL, pvBuffer, 8 KB, cRootFileIndex);

		return TRUE;
	}
	else
	{
		mpcRoot = AllocateRoot();
		bResult = mpcRoot->WriteToBuffer(pvBuffer, 8 KB);
		if (!bResult)
		{
			return FALSE;
		}

		iNodeSize = mpcRoot->CalculateBufferSize();
		pcRootIndexFile = mcIndexFiles.GetOrCreateFile(iNodeSize);
		if (!pcRootIndexFile)
		{
			return TRUE;
		}
		iFilePos = pcRootIndexFile->Write(pvBuffer);
		mpcRoot->SetFileIndex(pcRootIndexFile->GetFileIndex(), iFilePos);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIndex CIndexTreeFile::LoadRootFileIndex(char* szRootFileName)
{
	CFileBasic	cFileBasic;
	CDiskFile	cDiskFile;
	CFileUtil	cFileUtil;
	CFileIndex	cIndex;
	filePos		iSize;

	iSize = cFileUtil.Size(szRootFileName);
	if (iSize != -1)
	{
		if (iSize == sizeof(CFileIndex))
		{
			cDiskFile.Init(szRootFileName);
			cFileBasic.Init(&cDiskFile);

			cFileBasic.Open(EFM_Read);
			cFileBasic.Read(&cIndex, 0, sizeof(CFileIndex));
			cFileBasic.Close();

			cFileBasic.Kill();
			cDiskFile.Kill();

			return cIndex;
		}
		else
		{
			gcLogger.Error2(__METHOD__, "Index Root file size [", LongLongToString(iSize), "] is incorrect.  Should be SizeOf(CFileIndex).", NULL);
			cIndex.Init();
			return cIndex;
		}
	}
	else
	{
		cIndex.Init();
		return cIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateRoot(void)
{
	CIndexTreeNodeFile*		pcNode;
	int						iAdditionalSize;

	iAdditionalSize = CalculateRootNodeSize();
	pcNode = (CIndexTreeNodeFile*)Malloc(SizeofNode() + iAdditionalSize);
	pcNode->Init(this, NULL, 0, MAX_UCHAR);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateRoot(CFileIndex cFileIndex)
{
	CIndexTreeNodeFile*		pcNode;
	int						iAdditionalSize;

	iAdditionalSize = CalculateRootNodeSize();
	pcNode = (CIndexTreeNodeFile*)Malloc(SizeofNode() + iAdditionalSize);
	pcNode->Init(this, NULL, 0, MAX_UCHAR, cFileIndex);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexTreeFile::CalculateRootNodeSize(void)
{
	int						iAdditionalSize;

	iAdditionalSize = (MAX_UCHAR + 1) * SizeofNodePtr();
	return SizeofNode() + iAdditionalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::AllocateNode(CIndexTreeNodeFile* pcParent)
{
	CIndexTreeNodeFile*	pcNode;

	pcNode = (CIndexTreeNodeFile*)Malloc(SizeofNode());
	pcNode->Init(this, pcParent);
	return pcNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeFile::Get(char* pszKey)
{
	int		iKeySize;

	if (StrEmpty(pszKey))
	{
		return NULL;
	}

	iKeySize = strlen(pszKey);
	return Get(pszKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeFile::Get(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile* pcNode;
	void*				pv;

	pcNode = GetIndexNode(pvKey, iKeySize);
	if (pcNode == NULL)
	{
		return NULL;
	}
	else
	{
		if (pcNode->GetObjectSize() == 0)
		{
			return NULL;
		}
		pv = pcNode->GetObjectPtr();
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeFile::GetIndexNode(void* pvKey, int iKeySize)
{
	CIndexTreeNodeFile*		pcCurrent;
	CIndexTreeChildNode*	pcChild;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	for (int i = 0; i < iKeySize; i++)
	{
		char c = ((char*)pvKey)[i];

		pcChild = pcCurrent->Get(c);
		if (pcChild->IsMemory())
		{
			pcCurrent = pcChild->u.mpcMemory;
		}
		else if (pcChild->IsFile())
		{
			// Load that shit!
		}
		else
		{
			return NULL;
		}
	}
	return pcCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexTreeFile::NumElements(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(char* pszKey, void* pvObject, unsigned char uiObjectSize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Put(void* pvKey, int iKeySize, unsigned char uiObjectSize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::PutPtr(void* pvKey, int iKeySize, void* pvPointer)
{
	//Just for testing, writing pointers out to file is... pointless.
	return Put(pvKey, iKeySize, &pvPointer, sizeof(void*));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::PutPtr(char* pszKey, void* pvPointer)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return PutPtr(pszKey, iKeySize, pvPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::PutLong(char* pszKey, int64 lliIndex)
{
	int iKeySize;

	if (StrEmpty(pszKey))
	{
		return FALSE;
	}

	iKeySize = strlen(pszKey);
	return PutLong(pszKey, iKeySize, lliIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::PutLong(void* pvKey, int iKeySize, int64 lliIndex)
{
	return Put(pvKey, iKeySize, &lliIndex, sizeof(int64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Remove(char* pszKey)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::Remove(void* pvKey, int iKeySize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::HasKey(char* pszKey)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFile::HasKey(void* pvKey, int iKeySize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeFile::FindAll(CArrayVoidPtr* papvElements)
{
}
