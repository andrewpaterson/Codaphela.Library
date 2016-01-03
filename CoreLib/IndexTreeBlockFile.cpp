#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "IndexTreeBlockFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName)
{
	Init(pcDurableFileControl, szRootFileName, &gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeBlockFile::Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc)
{
	CIndexTreeBlock::Init(pcMalloc, sizeof(CIndexTreeNodeFile), sizeof(SIndexTreeChildFile));

	InitRoot(szRootFileName);

	mpcDurableFileControl = pcDurableFileControl;
	mcIndexFiles.Init(mpcDurableFileControl, "IDAT", "Index.IDX", "_Index.IDX");
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
void CIndexTreeBlockFile::InitRoot(char* szRootFileName)
{
	BOOL		bRootIndexExists;
	CFileIndex	cRootFileIndex;

	mszRootFileName.Init(szRootFileName);
	cRootFileIndex = LoadRootFileIndex(szRootFileName);

	bRootIndexExists = cRootFileIndex.HasFile();
	if (bRootIndexExists)
	{
		mpcRoot->Init(this, NULL, cRootFileIndex);
	}
	else
	{
		mpcRoot = AllocateRoot();
	}
	mpcDurableFileControl->Begin();


	mpcDurableFileControl->End();

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
	CIndexTreeNodeFile* pcCurrent;

	if ((iKeySize == 0) || (pvKey == NULL))
	{
		return NULL;
	}

	pcCurrent = mpcRoot;
	for (int i = 0; i < iKeySize; i++)
	{
		char c = ((char*)pvKey)[i];

		pcCurrent = pcCurrent->Get(c);
		if (pcCurrent == NULL)
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

