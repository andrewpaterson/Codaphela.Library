#ifndef __LIST_BLOCK_H__
#define __LIST_BLOCK_H__
#include "ArrayVoidPtr.h"
#include "FreeList.h"


//Elements in an CListBlock are always of the same size.  List elements are indexed from the start of the list.  
//Elements in a CListVariable can be different sizes.  List elements are indexed from the start of the list.  
class CListBlock
{
protected:
	CFreeList		mcFreeList;
	CArrayVoidPtr	mapIndices;

public:
	void	Init(size iElementSize);
	void	Init(size iElementSize, uint16 iAlignment);
	void	Init(size iElementSize, uint16 iAlignment, int16 iOffset);
	void	Kill(void);

	size	NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	size 	ElementSize(void);
	size	NumBlocks(void);
	size	NumElementsFromFreeList(void);

	void*	Add(void);
	void*	Add(void* pvData);
	void* 	AddGetIndex(size* piIndex);

	void*	Get(size iIndex);
	void*	SafeGet(size iIndex);
	size	GetIndex(void* pvElement);
	void*	Tail(void);

	void* 	InsertAt(size iIndex);
	void* 	InsertAt(void* pvData, size iIndex);

	bool	Pop(void* pvData);
	bool	Pop(void);
	void* 	Push(void* pvElement);
	void*	Push(void);

	void	Reverse(void);

	bool	Contains(void* pvData);

	void 	RemoveAt(size iIndex, bool bPreserveOrder = true);
	void	RemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder = true);
	void 	RemoveTail(void);

	bool	Set(size iIndex, void* pvData);
	bool	SafeSet(size iIndex, void* pvData);
	void	Swap(size iIndex1, size iIndex2);

protected:
	void*	Dereference(void** pv);
};


#endif // __LIST_BLOCK_H__

