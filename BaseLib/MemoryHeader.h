#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__
#include "FreeList.h"
#include "PointerRemapper.h"
#include "ArrayVoidPtr.h"
#include "LinkedListBlockAligned.h"


struct SDataMemoryAllocation
{
	SFNode*			psFreeListNode;
	unsigned int	uiSize;
};


struct SGeneralMemoryAllocation : public SDataMemoryAllocation
{
	unsigned int	uiAllocCount;
	char			szDebug[4];
};


struct SFreeListDesc
{
	CFreeList*		pcFreeList;
	unsigned int	iStride;

	void Init(unsigned int iStride);
	void Init(CFreeList* pcFreeList, int iStride);
};


//Not using a SAlignedFreeListDesc does not imply the elements in the list are not aligned.
//Just that they all have the same alignment and offset.
struct SAlignedFreeListDesc : SFreeListDesc
{
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


//Memory Iterator result.
struct SMemory
{
	BOOL			bValid;
	void*			pvMem;
	unsigned int	uiAllocCount;
	unsigned int	uiSize;

	void Init(void);
	void Set(SGeneralMemoryAllocation* psAllocation);
	void Set(SDataMemoryAllocation* psAllocation);
};


typedef CArrayTemplateMinimal<SFreeListDesc>			CArrayFreeListDesc;
typedef CArrayTemplate<SAlignedFreeListDesc>	CArrayFreeListAlignedDesc;

int CompareAlignedFreeListDesc(const void* arg1, const void* arg2);
int CompareFreeListDesc(const void* arg1, const void* arg2);


#endif // __MEMORY_HEADER_H__

