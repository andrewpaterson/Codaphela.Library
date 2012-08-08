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
	TRISTATE				tResult;

	ReturnOnFalse(OpenFiles());
	AssignBlockNumbers();

	tTotalResult = TRIFALSE;
	for (i = 0; i < mpacBlocks->NumElements(); i++)
	{
		pcBlocks = mpacBlocks->Get(i);
		pcIndexedFile = GetFile(pcBlocks->GetDataSize(), pcBlocks->GetFileNumber());
		tResult = OptimiseBlock(pcBlocks, pcIndexedFile);

		if (tResult == TRIERROR)
		{
			tTotalResult = TRIERROR;
			break;
		}
		if (tResult == TRITRUE)
		{
			tTotalResult = TRITRUE;
		}
	}

	return CloseFiles(tTotalResult);
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
BOOL CNamedIndexesOptimiser::CloseFiles(TRISTATE tOptimiseResult)
{
	mpcFiles->Close();

	if (tOptimiseResult == TRITRUE)
	{
		mpcFiles->CopyPrimaryToBackup();
		return TRUE;
	}
	else if (tOptimiseResult == TRIERROR)
	{
		mpcFiles->CopyBackupToPrimary();
		return FALSE;
	}
	else
	{
		//All files were already optimised.
		return TRUE;
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
TRISTATE CNamedIndexesOptimiser::OptimiseBlock(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile)
{
	int								i;
	CNamesIndexedSorterSource*		pcSource;
	TRISTATE						tResult;

	tResult = AllocateSources(pcBlocks, pcIndexedFile);
	ReturnOnErrorAndFalse(tResult);



	KillSources();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CNamedIndexesOptimiser::AllocateSources(CNamedIndexesBlocks* pcBlocks, CIndexedFile* pcIndexedFile)
{
	CFileBasic*						pcFile;
	filePos							iSize;
	filePos							iNumBlocks;
	int								iChunks;
	filePos							iModulous;

	int								i;
	CNamesIndexedSorterSource*		pcSource;
	filePos							iPosition;

	if (!pcIndexedFile)
	{
		return TRIFALSE;
	}

	pcFile = pcIndexedFile->GetPrimaryFile();

	iSize = pcFile->GetFileSize();
	iNumBlocks = iSize / pcBlocks->GetDataSize();
	iModulous = iSize % pcBlocks->GetDataSize();
	if (iModulous != 0)
	{
		return TRIERROR;
	}

	iChunks = (int)(iNumBlocks / pcBlocks->GetNumBlocks());
	iModulous = iNumBlocks % pcBlocks->GetNumBlocks();
	if (iModulous != 0)
	{
		return TRIERROR;
	}

	macSources.Allocate(iChunks);

	for (i = 0; i < iChunks; i++)
	{
		iPosition = i * (pcBlocks->GetDataSize() * pcBlocks->GetNumBlocks());
		pcSource = macSources.Get(i);
		pcSource->Init(pcBlocks->GetMaxNameLength(), iPosition);
	}

	return TRITRUE;
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

