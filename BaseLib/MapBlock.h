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


struct SMapIterator : SArraySortedIterator
{
};


typedef int(*CompareFunc)(const void*, const void*);


class CMapBlock
{
protected:
	CArrayBlockSorted	mapArray;
	CMallocator*		mpcMalloc;
	int					miLargestKeySize;

public:
	int					(*Func)(const void*, const void*);

public:
	void				Init(CMallocator* pcMalloc, int iChunkSize, int(*Func)(const void*, const void*));
	void				Kill(void);

	BOOL				Get(void* pvKey, void** ppvData, int* piDataSize);
	void*				Put(void* pvKey, int iKeySize, int iDataSize);
	BOOL				Put(void* psKey, int iKeySize, void* pvData, int iDataSize);
	BOOL				Remove(void* pvKey);

	int					NumElements(void);
	CArrayBlockSorted*	GetArray(void);

	BOOL				StartIteration(SMapIterator* psIterator, void** pvKey, void** pvData);
	BOOL				Iterate(SMapIterator* psIterator, void** pvKey, void** pvData);

	BOOL				Write(CFileWriter* pcFileWriter);
	BOOL				Read(CFileReader* pcFileReader, int(*Func)(const void*, const void*));

	void				InsertHoldingIntoSorted(void);
	void				GetInSorted(int iIndex, void** ppvKey, void** ppvData);
};


#endif // __MAP_BLOCK_H__
