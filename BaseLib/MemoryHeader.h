#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__
#include "FreeList.h"
#include "PointerRemapper.h"
#include "ArrayVoidPtr.h"
#include "LinkedListBlockAligned.h"



struct SMemoryAllocation
{
	SFNode*			psFreeListNode;  //This is only valid when in a free list.
	unsigned int	uiAllocCount;
	unsigned int	uiSize;
	char			szDebug[4];
};


struct SFreeListDesc
{
	CFreeList*		pcFreeList;
	unsigned int	iStride;
	int				iAlignment;
	int				iOffset;

	void Init(unsigned int iStride, int iAlignment, int iOffset);
	void Init(CFreeList* pcFreeList, int iStride, int iAlignment, int iOffset);
};


struct SMemoryIterator
{
	SFreeListIterator	sFreeListIterator;
	CFreeList*			pcFreeList;
};


struct SMemory
{
	BOOL			bValid;
	void*			pvMem;
	unsigned int	uiAllocCount;
	unsigned int	uiSize;

	void Init(void);
	void Set(SMemoryAllocation* psAllocation);
};


typedef CArrayTemplate<SFreeListDesc>		CArrayFreeListDesc;
int CompareFreeListDesc(const void* arg1, const void* arg2);


#endif // __MEMORY_HEADER_H__

