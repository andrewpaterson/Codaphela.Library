#ifndef __MAP_BLOCK_H__
#define __MAP_BLOCK_H__
#include "ArrayBlockSorted.h"


class CMapBlock;
struct SMNode
{
	int			iKeySize;
	int			iDataSize;
	CMapBlock*	pcMapBlock;
};


class CMapBlock
{
protected:
	CArrayBlockSorted	mapArray;
	CMallocator*		mpcMalloc;
	int					miLargestKeySize;

public:
	int					(*Func)(const void*, const void*);

public:
	void	Init(CMallocator* pcMalloc, int iChunkSize, int(*Func)(const void*, const void*));
	void	Kill(void);

	BOOL	Get(void* pvKey, void** ppvData, int* piDataSize);
	BOOL	Put(void* psKey, int iKeySize, void* pvData, int iDataSize);

	int		NumElements(void);
};


#endif // __MAP_BLOCK_H__

