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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryManager::Kill(void)
{
	mcLinkedList.Kill();
	mpvHeapStart = NULL;
	mpvHeapEnd = NULL;
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

