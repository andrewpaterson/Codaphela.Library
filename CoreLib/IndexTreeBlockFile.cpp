#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "IndexedFile.h"
#include "IndexTreeBlockFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName)
{
	return Init(pcDurableFileControl, szRootFileName, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc)
{
	//The DurableFileControl must be begun before .Init is called and should be ended afterwards.

	BOOL	bResult;

	CIndexTreeBlock::Init(pcMalloc, sizeof(CIndexTreeNodeFile), sizeof(SIndexTreeChildFile));

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
void CIndexTreeBlockFile::FakeInit(void)
{
	CIndexTreeBlock::Init(&gcSystemAllocator, sizeof(CIndexTreeNodeFile), sizeof(SIndexTreeChildFile));
	mpcRoot = NULL;
	mpcDurableFileControl = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockFile::Kill(void)
{
	mszRootFileName.Kill();
	mcIndexFiles.Kill();
	mpcDurableFileControl = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::InitRoot(char* szRootFileName)
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
		iFilePos = pcRootIndexFile->Write(pvBuffer);
		mpcRoot->SetFileIndex(pcRootIndexFile->GetFileIndex(), iFilePos);

		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIndex CIndexTreeBlockFile::LoadRootFileIndex(char* szRootFileName)
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
			gcLogger.Error2("Index Root file size [", LongLongToString(iSize), "] is incorrect.  Should be SizeOf(CFileIndex).", NULL);
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
CIndexTreeNodeFile* CIndexTreeBlockFile::AllocateRoot(void)
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
CIndexTreeNodeFile* CIndexTreeBlockFile::AllocateRoot(CFileIndex cFileIndex)
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
size_t CIndexTreeBlockFile::CalculateRootNodeSize(void)
{
	int						iAdditionalSize;

	iAdditionalSize = (MAX_UCHAR + 1) * SizeofNodePtr();
	return SizeofNode() + iAdditionalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexTreeNodeFile* CIndexTreeBlockFile::AllocateNode(CIndexTreeNodeFile* pcParent)
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
void* CIndexTreeBlockFile::Get(char* pszKey)
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
void* CIndexTreeBlockFile::Get(void* pvKey, int iKeySize)
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
CIndexTreeNodeFile* CIndexTreeBlockFile::GetIndexNode(void* pvKey, int iKeySize)
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
int CIndexTreeBlockFile::NumElements(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Put(char* pszKey, void* pvObject, unsigned char uiObjectSize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexTreeBlockFile::Put(void* pvKey, int iKeySize, unsigned char uiObjectSize)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Remove(char* pszKey)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::Remove(void* pvKey, int iKeySize)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::HasKey(char* pszKey)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeBlockFile::HasKey(void* pvKey, int iKeySize)
{
	return FALSE;
}

