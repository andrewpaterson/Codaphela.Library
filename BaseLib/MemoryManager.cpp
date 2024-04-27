#include "PrimitiveTypes.h"
#include "PointerRemapper.h"
#include "MemoryManager.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::Init(void* pvHeapStart, void* pvHeapEnd)
{
	mcLinkedList.Init();
	mpvHeapStart = pvHeapStart;
	mpvHeapEnd = pvHeapEnd;

	muiCacheSize = MM_EMPTY_NODE_CACHE_SIZE;
	ClearEmptyCache();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::Kill(void)
{
	ClearEmptyCache();

	mcLinkedList.Kill();
	mpvHeapStart = NULL;
	mpvHeapEnd = NULL;
	mbCacheFull = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::DisableEmptyCache(void)
{
	muiCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::ClearEmptyCache(void)
{
	uint16	ui;

	muiEmptyCacheSize = 0;
	for (ui = 0; ui < muiCacheSize; ui++)
	{
		mapsEmpty[ui] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetMMNodeDataSize(SMMNode* psNode)
{
	uint32 uiSize;

	uiSize = psNode->uiSize;
	psNode->uiSize |= (psNode->uiFlags & MM_NODE_SIZE_MASK) << 16;
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetUsedMMNodeDataSize(SMMNode* psNode, uint32 uiSize)
{
	psNode->uiSize = uiSize;
	psNode->uiFlags = (uiSize >> 16) | MM_NODE_USED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetUnusedMMNodeDataSize(SMMNode* psNode, uint32 uiSize)
{
	psNode->uiSize = uiSize;
	psNode->uiFlags = (uiSize >> 16) & (~MM_NODE_USED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMMNode* GetNodeForData(void* pvData)
{
	SMMNode*	psNode;

	psNode = (SMMNode*)RemapSinglePointer(pvData, -(ptrdiff_t)(MM_NODE_HEADER_SIZE));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void MarkNodeRemoved(SMMNode* psNode)
{
	psNode->psNext = NULL;
	psNode->psPrev = NULL;
	psNode->uiFlags = 0xff;
	psNode->uiSize = 0xffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryManager::Allocate(uint32 uiSize)
{
	SMMNode*	psNode;
	uint32		uiNodeSize;

	if (uiSize == 0)
	{
		return NULL;
	}

	uiSize += MM_NODE_HEADER_SIZE;
	if (uiSize > MM_MAX_SIZE)
	{
		return NULL;
	}

	psNode = (SMMNode*)mcLinkedList.GetTail();
	while (psNode)
	{
		if (!(psNode->uiFlags & MM_NODE_USED))
		{
			uiNodeSize = GetMMNodeDataSize(psNode);
			if (uiNodeSize >= uiSize)
			{
				return AllocateNodeInUnused(psNode, uiNodeSize, uiSize);
			}
		}
		psNode = (SMMNode*)psNode->psPrev;
	}

	return AllocateNodeAfterEnd(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryManager::AllocateNodeInUnused(SMMNode* psUnusedNode, uint32 uiUnusedNodeSize, uint32 uiAllocateSize)
{
	uint32		uiCompareSize;
	SMMNode*	psNewUnusedNode;
	void*		pvData;

	pvData = RemapSinglePointer(psUnusedNode, MM_NODE_HEADER_SIZE);
	uiCompareSize = uiUnusedNodeSize - MM_NODE_HEADER_SIZE - 4;  //Make sure there is at least space for another node of four bytes after this one being allocated.
	SetUsedMMNodeDataSize(psUnusedNode, uiAllocateSize);
	if (uiAllocateSize >= uiCompareSize)
	{
		return pvData;
	}

	psNewUnusedNode = (SMMNode*)RemapSinglePointer(psUnusedNode, uiAllocateSize);
	mcLinkedList.InsertAfterNode(psUnusedNode, (SLLNode*)psNewUnusedNode);
	uiUnusedNodeSize = (size_t)psNewUnusedNode->psNext - (size_t)psNewUnusedNode;
	SetUnusedMMNodeDataSize(psNewUnusedNode, uiUnusedNodeSize);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryManager::AllocateNodeAfterEnd(uint32 uiSize)
{
	SMMNode*	psNode;
	uint32		uiNodeSize;
	void*		pvData;
	uint32		uiRemaingSize;

	psNode = (SMMNode*)mcLinkedList.GetTail();
	if (psNode == NULL)
	{
		psNode = (SMMNode*)mpvHeapStart;
	}
	else
	{
		uiNodeSize = GetMMNodeDataSize(psNode);
		psNode = (SMMNode*)RemapSinglePointer(psNode, uiNodeSize);
	}

	uiRemaingSize = (size_t)mpvHeapEnd - (size_t)psNode + 1;
	if (uiSize <= uiRemaingSize)
	{
		SetUsedMMNodeDataSize(psNode, uiSize);
		mcLinkedList.InsertAfterTail(psNode);
		pvData = RemapSinglePointer(psNode, MM_NODE_HEADER_SIZE);
		return pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::DeallocateLastNode(SMMNode* psNode)
{
	SMMNode* psPrev;

	while ((psNode->uiFlags & MM_NODE_USED) == 0)
	{
		psPrev = (SMMNode*)psNode->psPrev;
		mcLinkedList.Remove(psNode);
		MarkNodeRemoved(psNode);
		if (psPrev == NULL)
		{
			return;
		}
		psNode = psPrev;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::EvictUnsedEmptyCache(SMMNode* psAltered)
{
	uint16		ui;
	SMMNode*	psNext;
	bool		bLast;

	if (muiEmptyCacheSize == 0)
	{
		return;
	}

	bLast = true;
	psNext = (SMMNode*)psAltered->psNext;
	if (psNext)
	{
		for (ui = muiEmptyCacheSize-1; ; ui--)
		{
			if ((mapsEmpty[ui] >= psAltered) && (mapsEmpty[ui] < psNext))
			{
				mapsEmpty[ui] = NULL;
				if (bLast)
				{
					muiEmptyCacheSize--;
				}
				else
				{
					bLast = false;
				}
			}
			if (ui == 0)
			{
				break;
			}
		}
	}
	else
	{
		for (ui = muiEmptyCacheSize - 1; ; ui--)
		{
			if ((mapsEmpty[ui] >= psAltered))
			{
				mapsEmpty[ui] = NULL;
				if (bLast)
				{
					muiEmptyCacheSize--;
				}
				else
				{
					bLast = false;
				}
			}
			if (ui == 0)
			{
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::InsertEmptyIntoCache(SMMNode* psEmpty)
{
	uint16		ui;

	for (ui = 0; ui < muiEmptyCacheSize; ui++)
	{
		if (mapsEmpty[ui] == NULL)
		{
			mapsEmpty[ui] = psEmpty;
			return;
		}
	}

	for (; ui < muiCacheSize; ui++)
	{
		if (mapsEmpty[ui] == NULL)
		{
			muiEmptyCacheSize++;
			mapsEmpty[ui] = psEmpty;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::Deallocate(void* pvData)
{
	SMMNode*	psNode;
	SMMNode*	psNext;
	SMMNode*	psCurrent;
	uint32		uiUnusedNodeSize;

	if (pvData == NULL)
	{
		return;
	}

	psNode = GetNodeForData(pvData);
	psNode->uiFlags &= (~MM_NODE_USED);

	psNext = (SMMNode*)psNode->psNext;
	if (psNext == NULL)
	{
		DeallocateLastNode(psNode);
		return;
	}

	//Walk back to first unused node
	psCurrent = NULL;
	while (psNode && ((psNode->uiFlags & MM_NODE_USED) == 0))
	{
		psCurrent = psNode;
		psNode = (SMMNode*)psNode->psPrev;
	}
	psNode = psCurrent;
	psNext = (SMMNode*)psNode->psNext;

	if ((psNext->uiFlags & MM_NODE_USED) == 0)
	{
		while ((psNext->uiFlags & MM_NODE_USED) == 0)
		{
			psNode = psNext;
			psNext = (SMMNode*)psNode->psNext;
			mcLinkedList.Remove(psNode);
			MarkNodeRemoved(psNode);
		}
	}

	uiUnusedNodeSize = (size_t)psCurrent->psNext - (size_t)psCurrent;
	SetUnusedMMNodeDataSize(psCurrent, uiUnusedNodeSize);

	EvictUnsedEmptyCache(psCurrent);
	InsertEmptyIntoCache(psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMMNode* CMemoryManager::StartIteration(void)
{
	return (SMMNode*)mcLinkedList.GetHead();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMMNode* CMemoryManager::Iterate(SMMNode* psNode)
{
	return (SMMNode*)psNode->psNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMemoryManager::GetNumAllocations(bool bIncludeUnused)
{
	SMMNode*	psNode;
	uint32		uiCount;

	uiCount = 0;
	psNode = (SMMNode*)mcLinkedList.GetHead();
	while (psNode)
	{
		if (bIncludeUnused || psNode->uiFlags & MM_NODE_USED)
		{
			uiCount++;
		}
		psNode = (SMMNode*)psNode->psNext;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMemoryManager::GetUnusedAllocationSize(void)
{
	SMMNode*	psNode;
	uint32		uiSize;

	uiSize = 0;
	psNode = (SMMNode*)mcLinkedList.GetHead();
	while (psNode)
	{
		if (!(psNode->uiFlags & MM_NODE_USED))
		{
			uiSize += GetMMNodeDataSize(psNode);
		}
		psNode = (SMMNode*)psNode->psNext;
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMemoryManager::GetUsedAllocationSize(void)
{
	SMMNode*	psNode;
	uint32		uiSize;

	uiSize = 0;
	psNode = (SMMNode*)mcLinkedList.GetHead();
	while (psNode)
	{
		if (psNode->uiFlags & MM_NODE_USED)
		{
			uiSize += GetMMNodeDataSize(psNode);
		}
		psNode = (SMMNode*)psNode->psNext;
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMemoryManager::GetRemaingTailSize(void)
{
	SMMNode* psNode;

	psNode = (SMMNode*)mcLinkedList.GetTail();
	if (psNode)
	{
		return (size_t)mpvHeapEnd - (size_t)psNode - GetMMNodeDataSize(psNode) + 1;
	}
	else
	{
		return (size_t)mpvHeapEnd - (size_t)mpvHeapStart + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CMemoryManager::GetTotalSize(void)
{
	return (size_t)mpvHeapEnd - (size_t)mpvHeapStart + 1;
}

