#ifndef __MEMORY_MANAGER_H__
#define __MEMORY_MANAGER_H__
#include "PrimitiveTypes.h"
#include "LinkedList.h"


#define MM_MAX_SIZE	0x4f'ff'ff
#define MM_NODE_USED 0x80
#define MM_NODE_SIZE_MASK 0x7f


struct SMMNode : public SLLNode
{
	uint16	uiSize;
	uint8	uiFlags;  //bit 7: Used.  bit6..0 Upper 7 bits of size
};


#define MM_NODE_HEADER_SIZE			sizeof(SMMNode)
#define MM_EMPTY_NODE_CACHE_SIZE	32


class CMemoryManager
{
protected:
	CLinkedList		mcLinkedList;
	void*			mpvHeapStart;
	void*			mpvHeapEnd;

	SMMNode*		mapsEmpty[MM_EMPTY_NODE_CACHE_SIZE];
	uint16			muiEmptyCacheSize;
	uint16			muiCacheSize;
	bool			mbCacheFull;

public:
	void		Init(void* pvHeapStart, void* pvHeapEnd);
	void		Kill(void);

	void*		Allocate(uint32 uiSize);
	void		Deallocate(void* pvData);

	void		DisableEmptyCache(void);

	SMMNode*	StartIteration(void);
	SMMNode*	Iterate(SMMNode* psNode);

	uint32		GetNumAllocations(bool bIncludeUnused);
	uint32		GetUnusedAllocationSize(void);
	uint32		GetUsedAllocationSize(void);
	uint32		GetRemaingTailSize(void);
	uint32		GetTotalSize(void);

protected:
	void*		AllocateNodeInUnused(SMMNode* psUnusedNode, uint32 uiUnusedNodeSize, uint32 uiAllocateSize);
	void*		AllocateNodeAfterEnd(uint32 uiSize);
	void		DeallocateLastNode(SMMNode* psNode);

	void		ClearEmptyCache(void);
	void		EvictUnsedEmptyCache(SMMNode* psAltered);
	void		InsertEmptyIntoCache(SMMNode* psEmpty);
};


uint32 GetMMNodeDataSize(SMMNode* psNode);


#endif // __MEMORY_MANAGER_H__

