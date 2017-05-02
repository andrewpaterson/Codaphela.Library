#ifndef __LIST_BLOCK_H__
#define __LIST_BLOCK_H__
#include "ArrayVoidPtr.h"
#include "FreeList.h"


class CListBlock
{
protected:
	CFreeList		mcFreeList;
	CArrayVoidPtr	mapIndices;

public:
	void	Init(int iChunkSize, int iElementSize);
	void	Init(int iChunkSize, int iElementSize, int iAlignment);
	void	Init(int iChunkSize, int iElementSize, int iAlignment, int iOffset);
	void	Kill(void);

	int		NumElements(void);
	BOOL	IsEmpty(void);
	BOOL	IsNotEmpty(void);
	int 	ElementSize(void);
	int		NumBlocks(void);
	int		NumElementsFromFreeList(void);

	void*	Add(void);
	void*	Add(void* pvData);
	void* 	AddGetIndex(int* piIndex);

	void*	Get(int iIndex);
	void*	SafeGet(int iIndex);
	int		GetIndex(void* pvElement);
	void*	Tail(void);

	void* 	InsertAt(int iIndex);
	void* 	InsertAt(void* pvData, int iIndex);

	void	Pop(void* pvData);
	void	Pop(void);
	void* 	Push(void* pvElement);
	void*	Push(void);

	void	Reverse(void);

	BOOL	Contains(void* pData);

	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder = TRUE);
	void 	RemoveTail(void);

	BOOL	Set(int iIndex, void* pvData);
	BOOL	SafeSet(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);

protected:
	void*	Dereference(void** pv);
};


#endif // __LIST_BLOCK_H__

