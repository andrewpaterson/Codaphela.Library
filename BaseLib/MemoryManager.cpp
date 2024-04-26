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
void* CMemoryManager::Allocate(uint32 uiSize)
{
	SMMNode*	psNode;
	uint32		uiNodeSize;
	void*		pvData;
	uint32		uiRemaingSize;

	if (uiSize == 0)
	{
		return NULL;
	}

	uiSize += MM_NODE_HEADER_SIZE;
	if (uiSize > MM_MAX_SIZE)
	{
		return NULL;
	}

	psNode = (SMMNode*)mcLinkedList.GetHead();
	while (psNode)
	{
		if (!(psNode->uiFlags & MM_NODE_USED))
		{
			uiNodeSize = GetMMNodeDataSize(psNode);
			uiNodeSize = uiNodeSize + MM_NODE_HEADER_SIZE;
			if (uiNodeSize >= uiSize)
			{
				return AllocateNodeInUnused(psNode, uiNodeSize, uiSize);
			}
		}
		psNode = (SMMNode*)psNode->psNext;
	}

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
void* CMemoryManager::AllocateNodeInUnused(SMMNode* psUnusedNode, uint32 uiUnusedNodeSize, uint32 uiAllocateSize)
{
	uint32		uiCompareSize;
	SMMNode*	psNewUnusedNode;
	void*		pvData;

	pvData = RemapSinglePointer(psUnusedNode, MM_NODE_HEADER_SIZE);
	uiCompareSize = uiUnusedNodeSize - MM_NODE_HEADER_SIZE - 4;
	if (uiAllocateSize >= uiCompareSize)
	{
		SetUsedMMNodeDataSize(psUnusedNode, uiAllocateSize);
		return pvData;
	}

	psNewUnusedNode = (SMMNode*)RemapSinglePointer(psUnusedNode, uiAllocateSize);
	mcLinkedList.InsertAfterNode(psUnusedNode, (SLLNode*)psNewUnusedNode);
	uiUnusedNodeSize = (size_t)psNewUnusedNode->psNext - (size_t)psNewUnusedNode;
	SetUnusedMMNodeDataSize(psNewUnusedNode, uiUnusedNodeSize);
	return pvData;
}

