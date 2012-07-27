#include "BaseLib/PointerFunctions.h"
#include "CoreLib/IndexedFiles.h"
#include "NamedIndexesBlocks.h"
#include "NamedIndexes.h"
#include "NamedIndexesBlocksLoader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocksLoader::Init(CNamedIndexesBlocks* pcBlocks, int iFileNumber)
{
	if (iFileNumber != -1)
	{
		mpcFile = pcBlocks->mpcNamedIndexes->GetFile(pcBlocks->miBlockWidth, iFileNumber);
	}
	else
	{
		mpcFile = NULL;
	}

	if (!mpcFile)
	{
		mpvTemp = NULL;
		mpvBlock = NULL;
		miLength = 0;
		return;
	}

	pcBlocks->SetFileNumber(iFileNumber);
	miNumBlocksInFile = mpcFile->miNumDatas;
	mpcBlocks = pcBlocks;

	if (miNumBlocksInFile == 0)
	{
		mpvTemp = NULL;
		mpvBlock = NULL;
		miLength = 0;
		return;
	}
	
	miBlockWidth = pcBlocks->miBlockWidth;
	miLength = miNumBlocksInFile * miBlockWidth;
	miNewNumBlocks = mpcBlocks->miNewNumBlocks;
	miTempSize = ((10 MB) / (miNewNumBlocks * miBlockWidth)) * miNewNumBlocks * miBlockWidth;
	if (miLength <= miTempSize)
	{
		miTempSize = miLength;
	}

	mpvTemp = malloc((int)miTempSize);
	mpvBlock = malloc((size_t)(miNewNumBlocks * miBlockWidth));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocksLoader::Kill(void)
{
	mpcFile = NULL;
	SafeFree(mpvTemp);
	SafeFree(mpvBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexesBlocksLoader::Load(void)
{
	filePos					iRemaining;
	filePos					iChunk;
	filePos					iNames;
	filePos					iModulous;
	int						i;
	CNamedIndexedBlock*		psName;
	filePos					iPos;
	filePos					iOffset;
	filePos					iChunks;
	filePos					iDataIndex;

	iPos = 0LL;
	iRemaining = miLength;
	miCurrent = 0;
	mbBlockSorted = TRUE;

	if (!mpcFile)
	{
		return TRUE;
	}

	while (iRemaining != 0)
	{
		if (iRemaining > miTempSize)
		{
			iChunk = miTempSize;
		}
		else
		{
			iChunk = (int) iRemaining;
		}
		if (!mpcFile->mcFile.ReadFromFile(mpvTemp, iChunk, 1))
		{
			return FALSE;
		}

		iNames = iChunk / miBlockWidth;
		iModulous = iChunk % miBlockWidth;
		if (iModulous != 0)
		{
			return FALSE;
		}

		iChunks = iNames / miNewNumBlocks;
		iModulous = iNames % miNewNumBlocks;
		if (iModulous != 0)
		{
			return FALSE;
		}

		for (i = 0; i < iChunks; i++)
		{
			iOffset = miBlockWidth * i * miNewNumBlocks;
			psName = (CNamedIndexedBlock*)RemapSinglePointer(mpvTemp, (size_t)iOffset);
			iDataIndex = (iPos + iOffset)/miBlockWidth;
			mpcBlocks->AddNewBlock(miBlockWidth, psName, miNewNumBlocks, iDataIndex);
		}

		iPos += iChunk;
		iRemaining -= iChunk;
	}

	return TRUE;
}

