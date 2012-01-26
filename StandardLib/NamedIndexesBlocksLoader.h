#ifndef __NAMED_INDEXES_BLOCKS_LOADER_H__
#define __NAMED_INDEXES_BLOCKS_LOADER_H__
#include "BaseLib/Define.h"


class CNamedIndexesBlocks;
class CIndexedFile;
class CNamedIndexesBlocksLoader
{
protected:
	CNamedIndexesBlocks*	mpcBlocks;
	void*					mpvTemp;
	void*					mpvBlock;
	CIndexedFile*			mpcFile;
	int						miNumBlocks;
	int						miLength;
	int						miTempSize;
	int						miBlockWidth;

public:
	void Init(CNamedIndexesBlocks* pcBlocks);
	void Kill(void);

	BOOL Load(void);
};


#endif // __NAMED_INDEXES_BLOCKS_LOADER_H__

