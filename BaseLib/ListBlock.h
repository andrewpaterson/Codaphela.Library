#ifndef __LIST_BLOCK_H__
#define __LIST_BLOCK_H__
#include "ArrayVoidPtr.h"
#include "FreeList.h"


//Elements in a list are always of the same size.  List elements are indexed from the start of the list.  
//Elements in a list can be different sizes.  List elements are indexed from the start of the list.  
class CListBlock
{
protected:
	CFreeList		mcFreeList;
	CArrayVoidPtr	mapIndices;

public:
	void	Init(int iElementSize);
	void	Init(int iElementSize, int iAlignment);
	void	Init(int iElementSize, int iAlignment, int iOffset);
	void	Kill(void);

	int		NumElements(void);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
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

	bool	Pop(void* pvData);
	bool	Pop(void);
	void* 	Push(void* pvElement);
	void*	Push(void);

	void	Reverse(void);

	bool	Contains(void* pData);

	void 	RemoveAt(int iIndex, int bPreserveOrder = true);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, bool bPreserveOrder = true);
	void 	RemoveTail(void);

	bool	Set(int iIndex, void* pvData);
	bool	SafeSet(int iIndex, void* pvData);
	void	Swap(int iIndex1, int iIndex2);

protected:
	void*	Dereference(void** pv);
};


#endif // !__LIST_BLOCK_H__

