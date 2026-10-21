#include <stdlib.h>
#include <memory.h>
#include "ArrayElementNotFound.h"
#include "Logger.h"
#include "FreeList.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(size uiElementSize)
{
	Init(uiElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(size uiElementSize, size uiAlignment)
{
	Init(uiElementSize, uiAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(size uiElementSize, size uiAlignment, int16 iOffset)
{
	Init(uiElementSize, uiAlignment, iOffset, false);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(size uiElementSize, size uiAlignment, int16 iOffset, bool bSlabs)
{
	muiMagic = FREE_LIST_MAGIC;

	muiAlignment = (uint8)uiAlignment;
	miSuppliedOffset = (uint8)iOffset;
	muiElementSize = (uint16)uiElementSize;

	mbSlabs = bSlabs;
	if (mbSlabs)
	{
		mcList.Init(&gcSlabAllocator);
	}
	else
	{
		mcList.Init();
	}
	mpsUnused = NULL;
	mpsNotFullHead = NULL;
	muiNumElements = 0;

	muiOffset = (uint8)CalculateOffset(iOffset);
	muiStride = (uint16)CalculateStride();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Kill(void)
{
	if (mpsUnused != NULL)
	{
		mcList.FreeDetached(mpsUnused);
		mpsUnused = NULL;
	}

	mcList.Kill();
	mpsNotFullHead = NULL;
	muiNumElements = 0;
	muiElementSize = 0;
	muiAlignment = 0;
	muiStride = 0;
	muiOffset = 0;
	miSuppliedOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::CalculateStride(void)
{
	return ::CalculateStride(muiElementSize + muiOffset, muiAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8 CFreeList::CalculateOffset(int16 iOffset)
{
	return ::CalculateOffset(iOffset, muiAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8 CFreeList::CalculateOffset(SFNode* psNode)
{
	size	iBitArraySize;
	ptr		iStart;
	uint8	iByteDiff;
	int16	iOffset;

	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iStart = (ptr)(psNode) + sizeof(SFNode) + iBitArraySize;

	iByteDiff = iStart % muiAlignment;
	if (iByteDiff == 0)
	{
		return muiOffset;
	}
	else
	{
		iOffset = (muiAlignment - iByteDiff);
		return muiOffset + iOffset;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::CalculateBitArraySize(size uiChunkSize)
{
	//The bit array size is the number of elements in a chunk divided by 8 (+1).

	//Except this isn't what this does.
	return (((uiChunkSize >> 3) + 4) & 0xfffc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::CalculateNodeSize(size uiChunkSize)
{
	size	iBitArraySize;

	//Find the size of this free list block.  It is integer aligned.
	//miChunkSize+1 ... -1 to ensure enough space given alignment.
	iBitArraySize = CalculateBitArraySize(uiChunkSize);

	if (muiAlignment != 1)
	{
		return sizeof(SFNode) + iBitArraySize + (muiStride * (uiChunkSize + 1)) - 1;
	}
	else
	{
		return sizeof(SFNode) + iBitArraySize + (muiElementSize * uiChunkSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::CalculateSlabChunkSize(void)
{
	size	uiAvailable;
	size	uiChunkSize;

	//The most elements that fit in a slab after the linked list and free list node headers.
	uiAvailable = SLAB_SIZE - sizeof(SLLBlockNode);
	uiChunkSize = (uiAvailable - sizeof(SFNode)) / muiStride;
	while (CalculateNodeSize(uiChunkSize) > uiAvailable)
	{
		uiChunkSize--;
	}
	return uiChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Add(SFNode** ppsNode)
{
	SFNode*		psNode;
	size		iPosition;

	psNode = mpsNotFullHead;
	if (psNode == NULL)
	{
		psNode = AllocateNew();
	}

	iPosition = FindFirstFreePosition(psNode);
	if (ppsNode)
	{
		*ppsNode = psNode;
	}
	return AllocateExisting(psNode, iPosition);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Add(void* pvData)
{
	void*	pvTemp;

	pvTemp = Add();
	memcpy(pvTemp, pvData, muiElementSize);
	return pvTemp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::Remove(void* pvData)
{
	SFNode*		psNode;

	if (!pvData)
	{
		return false;
	}

	//Find the chunk in which the element is within this free list.
	psNode = FindNode(pvData);

	//Check the chunk could be found.
	if (psNode)
	{
		RemoveExisiting(psNode, pvData);
		return true;
	}

	//Could not find the chunk so could not remove the element.
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::Remove(SFNode* psNode, void* pvData)
{
	if (psNode && pvData)
	{
		RemoveExisiting(psNode, pvData);
		return true;
	}

	//Could not find the chunk so could not remove the element.
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::RemoveExisiting(SFNode* psNode, void* pvData)
{
	size	iBitArraySize;
	ptr		iNodeStart;
	ptr		iDataPos;
	size	iPos;

	//Find out the position of the element in the chunk.
	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iNodeStart = (ptr)(psNode) + iBitArraySize + sizeof(SFNode) + psNode->iOffset;
	iDataPos = (ptr)pvData;
	iPos = (iDataPos - iNodeStart) / muiStride;

	//Remove the element from the chunk.
	return RemoveExisiting(psNode, iPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CalculateNextChunkSize(size iNumBlocks)
{
	if (iNumBlocks >= 8)
	{
		return 256;
	}
	else if (iNumBlocks == 0)
	{
		return 8;
	}
	else if (iNumBlocks == 1)
	{
		return 8;
	}
	else if (iNumBlocks == 2)
	{
		return 16;
	}
	else if (iNumBlocks == 3)
	{
		return 32;
	}
	else if (iNumBlocks == 4)
	{
		return 32;
	}
	else if (iNumBlocks == 5)
	{
		return 64;
	}
	else if (iNumBlocks == 6)
	{
		return 96;
	}
	else if (iNumBlocks == 7)
	{
		return 128;
	}
	else
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::AllocateNew(void)
{
	SFNode*		psNew;
	size		iBitArraySize;
	size		iChunkSize;

	if (mpsUnused)
	{
		psNew = mpsUnused;

		mcList.InsertDetachedAfterTail(mpsUnused);
		mpsUnused = NULL;
		iChunkSize = psNew->uiChunkSize;
	}
	else
	{
		if (mbSlabs)
		{
			iChunkSize = CalculateSlabChunkSize();
		}
		else
		{
			iChunkSize = CalculateNextChunkSize(mcList.NumElements());
		}

		psNew = (SFNode*)mcList.InsertAfterTail(CalculateNodeSize(iChunkSize));
	}

	iBitArraySize = CalculateBitArraySize((size)iChunkSize);
	memset(psNew, 0, sizeof(SFNode) + iBitArraySize);
	psNew->uiChunkSize = (size)iChunkSize;
	psNew->iOffset = CalculateOffset(psNew);
	psNew->pcList = this;
	LinkNotFull(psNew);
	return psNew;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Deallocate(SFNode* psNode)
{
	if (!psNode->bFull)
	{
		UnlinkNotFull(psNode);
	}
	muiNumElements -= psNode->uiUsed;

	if (mpsUnused)
	{
		mcList.Remove(psNode);
	}
	else
	{
		mpsUnused = psNode;
		mcList.Detach(psNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::RemoveNode(SFNode* psNode)
{
	Deallocate(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::GetParams(SFreeListParams* psParams)
{
	psParams->uiAlignment = muiAlignment;
	psParams->uiElementSize = muiElementSize;
	psParams->iOffset = miSuppliedOffset;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::GetSlabNode(void* pvData)
{
	//Only valid for elements of a free list initialised with slabs.
	return (SFNode*)RemapSinglePointer(GetSlab(pvData), sizeof(SLLBlockNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::GetNode(size iNode)
{
	SFNode*		psNode;
	size		iCount;

	iCount = 0;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		if (iCount == iNode)
		{
			return psNode;
		}
		iCount++;
		psNode = (SFNode*)mcList.GetNext(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::IsElementInNodeAllocated(SFNode* psNode, size iPosition)
{
	void*	pvBitArray;
	bool	bAllocated;

	pvBitArray = GetBitArray(psNode);
	bAllocated = GetBit(iPosition, pvBitArray);
	if (bAllocated)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetElementInNode(SFNode* psNode, size iPosition)
{
	void*	pvData;
	size	iElementOffset;
	size	iBitArraySize;

	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iElementOffset = iPosition * muiStride;
	pvData = RemapSinglePointer(psNode, sizeof(SFNode) + iBitArraySize + psNode->iOffset + iElementOffset);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetElement(SFNode* psNode, size iPosition)
{
	if ((psNode == NULL) || (iPosition >= psNode->uiChunkSize))
	{
		return NULL;
	}

	if (IsElementInNodeAllocated(psNode, iPosition))
	{
		return GetElementInNode(psNode, iPosition);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetBitArray(SFNode* psNode)
{
	return RemapSinglePointer(psNode, sizeof(SFNode));
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::AllocateExisting(SFNode* psNode, size iPosition)
{
	void*	pvBitArray;
	void*	pvData;

	pvBitArray = GetBitArray(psNode);
	SetBit(iPosition, pvBitArray, 1);
	psNode->uiUsed++;
	muiNumElements++;
	if (psNode->uiUsed == psNode->uiChunkSize)
	{
		psNode->bFull = true;
		UnlinkNotFull(psNode);
	}

	pvData = GetElementInNode(psNode, iPosition);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::LinkNotFull(SFNode* psNode)
{
	psNode->psPrevNotFull = NULL;
	psNode->psNextNotFull = mpsNotFullHead;
	if (mpsNotFullHead)
	{
		mpsNotFullHead->psPrevNotFull = psNode;
	}
	mpsNotFullHead = psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::UnlinkNotFull(SFNode* psNode)
{
	if (psNode->psPrevNotFull)
	{
		psNode->psPrevNotFull->psNextNotFull = psNode->psNextNotFull;
	}
	else
	{
		mpsNotFullHead = psNode->psNextNotFull;
	}
	if (psNode->psNextNotFull)
	{
		psNode->psNextNotFull->psPrevNotFull = psNode->psPrevNotFull;
	}
	psNode->psNextNotFull = NULL;
	psNode->psPrevNotFull = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::FindFirstFreePosition(SFNode* psNode)
{
	uint32*	puiBits;
	size	i;

	//The node is not full so the first clear bit is before the end of the chunk.
	puiBits = (uint32*)GetBitArray(psNode);
	i = 0;
	while (puiBits[i] == 0xffffffff)
	{
		i++;
	}
	return (i * 32) + FindFirstClearBit(puiBits[i]);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::RemoveExisiting(SFNode* psNode, size iPosition)
{
	void*	pvBitArray;

	//Get the position of the start of the bit array.
	pvBitArray = GetBitArray(psNode);
	if (!GetBit(iPosition, pvBitArray))
	{
		//The element is not allocated.
		return false;
	}

	//Mark this element as no longer in use.
	SetBit(iPosition, pvBitArray, 0);
	psNode->uiUsed--;
	muiNumElements--;
	if (psNode->bFull)
	{
		psNode->bFull = false;
		LinkNotFull(psNode);
	}

	//Check if any elements are still allocated in this chunk.
	if (psNode->uiUsed == 0)
	{
		//If not deallocate the element.
		Deallocate(psNode);

		//Return true specifying that the chunk has been deallocated.
		return true;
	}

	//Return false specifying that the chunk contains elements and has not been deallocated.
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Get(size iElement)
{
	//Unallocated elements in the free list are not included.
	//This is the normal operation.

	size				i;
	SFreeListIterator	sFreeListIterator;
	void*				p;

	i = 0;
	p = StartIteration(&sFreeListIterator);
	while (i != iElement)
	{
		i++;
		p = Iterate(&sFreeListIterator);
		if (p == NULL)
		{
			break;
		}
	}
	return p;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::StartIteration(SFreeListIterator* psIterator)
{
	void*	pvBitArray;

	if (mcList.GetHead() == NULL)
	{
		psIterator->psCurrent = NULL;
		return NULL;
	}

	psIterator->psCurrent = (SFNode*)mcList.GetHead();
	psIterator->iPosition = 0;
	pvBitArray = GetBitArray(psIterator->psCurrent);

	if (!GetBit(0, pvBitArray))
	{
		FindNextAllocatedElement(psIterator);
	}
	return GetElementInNode(psIterator->psCurrent, psIterator->iPosition);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Iterate(SFreeListIterator* psIterator)
{
	if (psIterator->psCurrent == NULL)
	{
		return StartIteration(psIterator);
	}

	FindNextAllocatedElement(psIterator);

	if (psIterator->psCurrent != NULL)
	{
		return GetElementInNode(psIterator->psCurrent, psIterator->iPosition);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::RemoveDuringIteration(SFreeListIterator* psIterator)
{
	SFNode*		psNode;
	size		iPosition;

	psNode = psIterator->psCurrent;
	if (psNode && (psIterator->iPosition != ARRAY_ELEMENT_NOT_FOUND))
	{
		iPosition = psIterator->iPosition;
		FindPrevAllocatedElement(psIterator);	//Step the iterator back so iteration can continue.
		return RemoveExisiting(psNode, iPosition);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Iterator.psCurrent is NULL.  Iterate must be called before RemoveDuringIteration.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::FindNextAllocatedElement(SFreeListIterator* psIterator)
{
	void*	pvBitArray;

	//Assumes that the iterator position must be moved.
	pvBitArray = GetBitArray(psIterator->psCurrent);
	for (;;)
	{
		psIterator->iPosition++;

		if (psIterator->iPosition >= psIterator->psCurrent->uiChunkSize)
		{
			psIterator->iPosition = 0;

			psIterator->psCurrent = (SFNode*)mcList.GetNext(psIterator->psCurrent);
			if (psIterator->psCurrent == NULL)
			{
				return false;
			}
			pvBitArray = GetBitArray(psIterator->psCurrent);
		}

		if (GetBit(psIterator->iPosition, pvBitArray))
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::FindPrevAllocatedElement(SFreeListIterator* psIterator)
{
	void*	pvBitArray;

	//Assumes that the iterator position must be moved.
	pvBitArray = GetBitArray(psIterator->psCurrent);
	for (;;)
	{
		if (psIterator->iPosition == ARRAY_ELEMENT_NOT_FOUND)
		{
			psIterator->psCurrent = (SFNode*)mcList.GetPrev(psIterator->psCurrent);
			if (psIterator->psCurrent == NULL)
			{
				psIterator->iPosition = ARRAY_ELEMENT_NOT_FOUND;
				return false;
			}
			psIterator->iPosition = psIterator->psCurrent->uiChunkSize - 1;
			pvBitArray = GetBitArray(psIterator->psCurrent);
		}
		else if (psIterator->iPosition == 0)
		{
			psIterator->iPosition = ARRAY_ELEMENT_NOT_FOUND;
			psIterator->psCurrent = NULL;
			return false;
		}
		else
		{
			psIterator->iPosition--;
		}

		if (GetBit(psIterator->iPosition, pvBitArray))
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::FindNode(void* pvData, bool bIsAllocated)
{
	ptr			iDataPos;
	ptr			iNodeStart;
	ptr			iNodeEnd;
	SFNode*		psNode;
	size		iBitArraySize;
	size		iPos;
	void*		pvBitArray;

	iDataPos = (ptr)pvData;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
		iNodeStart = (ptr)psNode + iBitArraySize + sizeof(SFNode) + psNode->iOffset;
		iNodeEnd = iNodeStart + (muiStride * (psNode->uiChunkSize - 1));

		if ((iDataPos >= iNodeStart) && (iDataPos <= iNodeEnd))
		{
			//The node is in this block.
			if (bIsAllocated)
			{
				pvBitArray = GetBitArray(psNode);
				iPos = (iDataPos - iNodeStart) / muiStride;
				if (GetBit(iPos, pvBitArray))
				{
					return psNode;
				}
				return NULL;
			}
			else
			{
				return psNode;
			}
		}
		else
		{
			psNode = (SFNode*)mcList.GetNext(psNode);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::ByteSize(void)
{
	//This returns just the size of the allocated memory allocated blocks.  It does not include the free list 'header' or the link list 'header. 
	//Slabs are always allocated whole.
	if (mbSlabs)
	{
		return mcList.NumElements() * SLAB_SIZE;
	}
	return mcList.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::SetAdditionalSize(size iSize)
{
	//Make sure no allocations have been done.
	if ((mcList.GetHead() == NULL) && (mcList.GetTail() == NULL))
	{
		muiElementSize += (uint16)iSize;
		muiStride = (uint16)CalculateStride();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::Contains(void* pvData, bool bIsAllocated)
{
	SFNode*		psNode;

	psNode = FindNode(pvData, bIsAllocated);
	if (psNode)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::NumElements(void)
{
	return muiNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::HasElements(void)
{
	return muiNumElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::NumNodes(void)
{
	SFNode*		psNode;
	size		iCount;

	iCount = 0;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		iCount++;
		psNode = (SFNode*)mcList.GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::NumNodeElements(SFNode* psNode)
{
	return psNode->uiUsed;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetFirstNodeElement(SFNode* psNode)
{
	size	iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, 0);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindFirstSetBit(pvBitArray, psNode->uiChunkSize);
		if (iPosition == ARRAY_ELEMENT_NOT_FOUND)
		{
			return NULL;
		}

		if (iPosition >= psNode->uiChunkSize)
		{
			return NULL;
		}

		return GetElementInNode(psNode, iPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetLastNodeElement(SFNode* psNode)
{
	size	iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, psNode->uiChunkSize - 1);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindLastSetBit(pvBitArray, psNode->uiChunkSize);
		if (iPosition == ARRAY_ELEMENT_NOT_FOUND)
		{
			return NULL;
		}

		if (iPosition >= psNode->uiChunkSize)
		{
			return NULL;
		}

		return GetElementInNode(psNode, iPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeList::GetElementSize(void) { return muiElementSize; }
size CFreeList::GetElementStride(void) {return muiStride; }
size CFreeList::GetNumAllocatedChunks(void) { return mcList.NumElements(); }
uint8 CFreeList::GetOffset(void) { return muiOffset; }
uint8 CFreeList::GetAlignment(void) { return muiAlignment; }

