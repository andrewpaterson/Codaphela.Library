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
	miNumBlocks = mpcFile->miNumDatas;
	mpcBlocks = pcBlocks;

	if (miNumBlocks == 0)
	{
		mpvTemp = NULL;
		mpvBlock = NULL;
		return;
	}
	
	miBlockWidth = pcBlocks->miBlockWidth;
	miLength = miNumBlocks * miBlockWidth;
	miTempSize = ((10 MB) / (pcBlocks->miNewNumBlocks * miBlockWidth)) * pcBlocks->miNewNumBlocks * miBlockWidth;
	if (miLength <= miTempSize)
	{
		miTempSize = miLength;
	}
	mpvTemp = malloc((int)miTempSize);
	mpvBlock = malloc(pcBlocks->miMaxNumBlocks * miBlockWidth);
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
	int					iRemaining;
	int					iChunk;
	int					iNames;
	int					iModulous;
	int					i;
	CNamedIndexesBlock*	pcBlock;
	int					iName;

	iRemaining = miLength;
	pcBlock = NULL;
	iName = 0;
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
		iNames = iChunk/miBlockWidth;
		iModulous = iChunk%miBlockWidth;
		if (iModulous != 0)
		{
			return FALSE;
		}

		for (i = 0; i < iNames; i++)
		{
			if (pcBlock == NULL)
			{
				pcBlock = mpcBlocks->macBlocks.Add();
				pcBlock->Init(miBlockWidth, 0);
			}

		}

		iRemaining -= iChunk;
	}

	return TRUE;
}

