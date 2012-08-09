#include "BaseLib/DiskFile.h"
#include "BaseLib/BufferedFile.h"
#include "CoreLib/IndexedFile.h"
#include "NamedIndexesBlocks.h"
#include "NamedIndexes.h"
#include "NamedIndexesOptimiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesOptimiser::Init(CNamedIndexes* pcNamedIndexes)
{
	macSources.Init(512);
	mpcFiles = pcNamedIndexes->GetFiles();
	mpacBlocks = pcNamedIndexes->GetBlocks();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesOptimiser::Kill(void)
{
	mpcFiles = NULL;
	mpacBlocks = NULL;
	macSources.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::Optimise(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlocks;
	CIndexedFile*			pcIndexedFile;
	TRISTATE				tTotalResult;
	BOOL					bResult;

	ReturnOnFalse(OpenFiles());
	AssignBlockNumbers();

	tTotalResult = TRIFALSE;
	for (i = 0; i < mpacBlocks->NumElements(); i++)
	{
		pcBlocks = mpacBlocks->Get(i);
		pcIndexedFile = GetFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
		bResult = OptimiseBlock(pcBlocks, pcIndexedFile);
		if (!bResult)
		{
			return CloseFiles(FALSE);
		}
	}
	return CloseFiles(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::OpenFiles(void)
{
	if (mpcFiles->IsDurable())
	{
		return FALSE;
	}

	return mpcFiles->Open();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::CloseFiles(BOOL bCopy)
{
	int						i;
	CNamedIndexesBlocks*	pcBlocks;
	CIndexedFile*			pcIndexedFile;
	BOOL					bResult;

	mpcFiles->Close();

	if (!bCopy)
	{
		DeleteFiles();
		return FALSE;
	}
	else
	{
		for (i = 0; i < mpacBlocks->NumElements(); i++)
		{
			pcBlocks = mpacBlocks->Get(i);
			pcIndexedFile = GetFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
			if (pcIndexedFile)
			{
				bResult = CopyFile(pcIndexedFile->GetFileName(), pcIndexedFile->GetFileName());
				if (!bResult)
				{
					return FALSE;
				}
			}
		}

		for (i = 0; i < mpacBlocks->NumElements(); i++)
		{
			pcBlocks = mpacBlocks->Get(i);
			pcIndexedFile = GetFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
			if (pcIndexedFile)
			{
				bResult = CopyFile(pcIndexedFile->mszRewriteName.Text(), pcIndexedFile->GetFileName());
				if (!bResult)
				{
					return FALSE;
				}
			}
		}
		DeleteFiles();
		return TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::CopyFile(char* szDestName, char* szSourceName)
{
	CFileUtil				cFileUtil;
	CChars					szFullName;
	BOOL					bResult;

	bResult = cFileUtil.Copy(TempFileName(&szFullName, szSourceName), szDestName);
	szFullName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesOptimiser::DeleteFiles(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlocks;
	CIndexedFile*			pcIndexedFile;
	CFileUtil				cFileUtil;
	CChars					szFullName;

	for (i = 0; i < mpacBlocks->NumElements(); i++)
	{
		pcBlocks = mpacBlocks->Get(i);
		pcIndexedFile = GetFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
		if (pcIndexedFile)
		{
			cFileUtil.Delete(TempFileName(&szFullName, pcIndexedFile->GetFileName()));
			szFullName.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesOptimiser::AssignBlockNumbers(void)
{
	int						i;
	CNamedIndexesBlocks*	pcBlocks;
	int						iFileNumber;
	
	for (i = 0; i < mpacBlocks->NumElements(); i++)
	{
		pcBlocks = mpacBlocks->Get(i);
		iFileNumber = mpcFiles->GetUniqueFileNumber(pcBlocks->GetDataSize());
		pcBlocks->SetFileNumber(iFileNumber);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::OptimiseBlock(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile)
{
	int								i;
	CNamesIndexedSorterSource*		pcSource;
	CFileBasic						cDestFile;
	BOOL							bResult;
	filePos							iNumNames;
	CNamedIndexedBlock				cBlock;

	if (!pcIndexedFile)
	{
		return TRUE;
	}

	bResult = AllocateSources(pcBlocks, pcIndexedFile);
	ReturnOnFalse(bResult);
	bResult = LoadInitialSources(pcIndexedFile, pcBlocks->GetDataSize());
	ReturnOnFalse(bResult);

	bResult = OpenDestinationFile(&cDestFile, pcIndexedFile->GetFileName());
	if (!bResult)
	{
		KillSources();
		return FALSE;
	}

	iNumNames = macSources.NumElements() * pcBlocks->GetNumBlocks();
	for (i = 0; i < iNumNames; i++)
	{
		pcSource = GetSmallestSource(pcBlocks->GetNumBlocks());
		if (!pcSource)
		{
			break;
		}

		bResult = cDestFile.WriteData(pcSource->mpcCurrent, pcBlocks->GetDataSize());
		if (!bResult)
		{
			cDestFile.Close();
			cDestFile.Kill();
			KillSources();
			return FALSE;
		}

		bResult = pcSource->ReadNext(pcIndexedFile->GetPrimaryFile(), pcBlocks->GetDataSize());
		if (!bResult)
		{
			cDestFile.Close();
			cDestFile.Kill();
			KillSources();
			return TRIERROR;
		}
	}

	memset_fast(&cBlock, 0, pcBlocks->GetDataSize());
	for (; i < iNumNames; i++)
	{
		cDestFile.WriteData(&cBlock, pcBlocks->GetDataSize());
	}

	cDestFile.Close();
	cDestFile.Kill();
	KillSources();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamesIndexedSorterSource* CNamedIndexesOptimiser::GetSmallestSource(int iBlockChunkSize)
{
	int							i;
	CNamesIndexedSorterSource*	pcSource;
	CNamesIndexedSorterSource*	pcSmallestSource;

	if (macSources.IsEmpty())
	{
		return NULL;
	}

	pcSmallestSource = NULL;
	for (i = 0; i < macSources.NumElements(); i++)
	{
		pcSource = macSources.Get(i);
		if (pcSource->HasRemaining(iBlockChunkSize))
		{
			if (pcSource->IsSmallerThan(pcSmallestSource))
			{
				pcSmallestSource = pcSource;
			}
		}
	}

	return pcSmallestSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::LoadInitialSources(CIndexedFile* pcIndexedFile, int iDataSize)
{
	int							i;
	CNamesIndexedSorterSource*	pcSource;
	CNamedIndexedBlock*			pcBlock;
	CFileBasic*					pcFile;
	BOOL						bResult;

	pcFile = pcIndexedFile->GetPrimaryFile();
	pcBlock = NULL;
	for (i = 0; i < macSources.NumElements(); i++)
	{
		pcSource = macSources.Get(i);
		bResult = pcSource->ReadNext(pcFile, iDataSize);
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::OpenDestinationFile(CFileBasic* pcDestFile, char* szName)
{
	CChars							szFullName;
	
	pcDestFile->Init(BufferedFile(DiskFile(TempFileName(&szFullName, szName)), 10 MB));
	szFullName.Kill();
	return pcDestFile->Open(EFM_Write_Create);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNamedIndexesOptimiser::TempFileName(CChars* pszDest, char* szName)
{
	pszDest->Init(szName);
	pszDest->Append(".TMP");
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesOptimiser::AllocateSources(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile)
{
	CFileBasic*						pcFile;
	filePos							iSize;
	filePos							iNumBlocks;
	int								iChunks;
	filePos							iModulous;

	int								i;
	CNamesIndexedSorterSource*		pcSource;
	filePos							iPosition;

	pcFile = pcIndexedFile->GetPrimaryFile();

	iSize = pcFile->GetFileSize();
	iNumBlocks = iSize / pcBlocks->GetDataSize();
	iModulous = iSize % pcBlocks->GetDataSize();
	if (iModulous != 0)
	{
		return FALSE;
	}

	iChunks = (int)(iNumBlocks / pcBlocks->GetNumBlocks());
	iModulous = iNumBlocks % pcBlocks->GetNumBlocks();
	if (iModulous != 0)
	{
		return FALSE;
	}

	macSources.Allocate(iChunks);

	for (i = 0; i < iChunks; i++)
	{
		iPosition = i * (pcBlocks->GetDataSize() * pcBlocks->GetNumBlocks());
		pcSource = macSources.Get(i);
		pcSource->Init(pcBlocks->GetDataSize(), iPosition);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesOptimiser::KillSources(void)
{
	int								i;
	CNamesIndexedSorterSource*		pcSource;

	for (i = 0; i < macSources.NumElements(); i++)
	{
		pcSource = macSources.Get(i);
		pcSource->Kill();
	}

	macSources.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CNamedIndexesOptimiser::GetFile(int iDataSize, int iFileNumber)
{
	return mpcFiles->GetFile(iDataSize, iFileNumber);
}

