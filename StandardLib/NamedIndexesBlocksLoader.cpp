#include "BaseLib/PointerFunctions.h"
#include "CoreLib/IndexedFiles.h"
#include "NamedIndexesBlocks.h"
#include "NamedIndexesBlocksLoader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexesBlocksLoader::Init(CNamedIndexesBlocks* pcBlocks)
{
	mpcFile = pcBlocks->mpcFiles->GetFile(pcBlocks->miBlockWidth, pcBlocks->miFileNumber);
	if (!mpcFile)
	{
		mpvTemp = NULL;
		mpvBlock = NULL;
		return;
	}

	miNumBlocksInFile = mpcFile->miNumDatas;
	mpcBlocks = pcBlocks;

	if (miNumBlocksInFile == 0)
	{
		mpvTemp = NULL;
		mpvBlock = NULL;
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
	mpvBlock = malloc(miNewNumBlocks * miBlockWidth);
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
	int						iRemaining;
	int						iChunk;
	int						iNames;
	int						iModulous;
	int						i;
	CNamedIndexedBlock*		psName;
	filePos					ulliPos;
	int						iOffset;
	int						iChunks;

	ulliPos = 0LL;
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
			psName = (CNamedIndexedBlock*)RemapSinglePointer(mpvTemp, iOffset);
			mpcBlocks->AddNewBlock(miBlockWidth, psName, miNewNumBlocks, ulliPos + iOffset);
		}

		ulliPos += iChunk;
		iRemaining -= iChunk;
	}

	return TRUE;
}

