#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__
#include "FreeList.h"
#include "PointerRemapper.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateMinimal.h"
#include "LinkedListBlockAligned.h"
#include "ArrayTemplatePtr.h"


struct SDataMemoryAllocation
{
	size		uiSize;
	SFNode*		psFreeListNode;
};


//General memory free lists are made of slabs so the free list node is found from the allocation's address, see CFreeList::GetSlabNode.
struct SGeneralMemoryAllocation
{
	size	uiSize;
	size	uiAllocCount;
	char	szDebug[4];
#ifndef BUILD_64
	uint32	uiUnused;	//Keeps the header a multiple of 8 bytes so 8 byte aligned elements have 8 byte aligned data.
#endif // BUILD_64
};


struct SFreeListDesc
{
	CFreeList*	pcFreeList;
	size		iStride;

	void Init(size iStride);
	void Init(CFreeList* pcFreeList, int iStride);
};


//Not using a SAlignedFreeListDesc does not imply the elements in the list are not aligned.
//Just that they all have the same alignment and offset.
struct SAlignedFreeListDesc : SFreeListDesc
{
	uint8	iAlignment;
	int8	iOffset;

	void Init(size iStride, uint16 iAlignment, int16 iOffset);
	void Init(CFreeList* pcFreeList, int iStride, uint16 iAlignment, int16 iOffset);
};


struct SMemoryIterator
{
	bool				bInFreeLists;

	SFreeListIterator	sFreeListIterator;
	CFreeList*			pcFreeList;

	void*				pvLarge;
};


//Memory Iterator result.
struct SMemory
{
	bool	bValid;
	void*	pvMem;
	size	uiAllocCount;
	size	uiSize;

	void Init(void);
	void Set(SGeneralMemoryAllocation* psAllocation);
	void Set(SDataMemoryAllocation* psAllocation);
};


typedef CArrayTemplateMinimal<SFreeListDesc>	CArrayFreeListDesc;
typedef CArrayTemplate<SAlignedFreeListDesc>	CArrayFreeListAlignedDesc;
typedef CArrayTemplatePtr<CFreeList>				CArrayFreeListPtrs;

int CompareAlignedFreeListDesc(const void* arg1, const void* arg2);
int CompareFreeListDesc(const void* arg1, const void* arg2);


#endif // __MEMORY_HEADER_H__

