#ifndef __MEMORY_HEADER_H__
#define __MEMORY_HEADER_H__
#include "FreeListBlock.h"
#include "PointerRemapper.h"
#include "ArrayVoidPtr.h"
#include "LinkedListBlockAligned.h"


#define MEMORY_FREE_LIST_SIZE_LIMIT	2048


struct SMemoryAllocation
{
	SFNode*			psFreeListNode;  //This is only valid when in a freelist.
	unsigned int	uiAllocCount;
	unsigned int	uiSize;
	char			szDebug[4];
};


struct SFreeListParams
{
	unsigned int	iMaxListSize;  //This is the size of the actual element in the freelist including SMemoryAllocation
	unsigned int	iMinListSize;  //This is the size of the previous element in the freelist including SMemoryAllocation +1.

	//These exist for some binary search Compare function stuffs.
	unsigned int	iMaxElementSize;
	unsigned int	iMinElementSize;

	int				iChunkSize;

	SFreeListParams*	Init(unsigned int iFreeListSize, int iPrevSize, int iChunkSize);
};


typedef CArrayTemplate<SFreeListParams>		CArrayFreeListParams;
int CompareFreeListParam(const void* arg1, const void* arg2);


struct SFreeListDesc
{
	CFreeListBlock*		pcFreeList;
	unsigned int		iStride;
	int					iAlignment;
	int					iOffset;

	void Init(unsigned int iStride, int iAlignment, int iOffset);
	void Init(CFreeListBlock* pcFreeList, int iStride, int iAlignment, int iOffset);
};


struct SMemoryIterator
{
	SFreeListIterator	sFreeListIterator;
	CFreeListBlock*		pcFreeList;
};


typedef CArrayTemplate<SFreeListDesc>		CArrayFreeListDesc;
int CompareFreeListDesc(const void* arg1, const void* arg2);


#endif // __MEMORY_HEADER_H__

