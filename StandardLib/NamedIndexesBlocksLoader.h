#ifndef __NAMED_INDEXES_BLOCKS_LOADER_H__
#define __NAMED_INDEXES_BLOCKS_LOADER_H__
#include "BaseLib/Define.h"
#include "NamedIndexedBlock.h"


class CNamedIndexesBlocks;
class CIndexedFile;
class CNamedIndexesBlocksLoader
{
protected:
	CNamedIndexesBlocks*	mpcBlocks;
	void*					mpvTemp;
	void*					mpvBlock;
	CIndexedFile*			mpcFile;
	filePos					miNumBlocksInFile;
	filePos					miLength;
	filePos					miTempSize;
	int						miBlockWidth;
	filePos					miMaxNames;
	filePos					miBlockChunkSize;
	filePos					miCurrent;
	BOOL					mbBlockSorted;

public:
	void	Init(CNamedIndexesBlocks* pcBlocks, int iFileNumber);
	void	Kill(void);

	BOOL	Load(void);
};


#endif // __NAMED_INDEXES_BLOCKS_LOADER_H__

