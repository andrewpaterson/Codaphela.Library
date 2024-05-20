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
	muiMagic = FREE_LIST_MAGIC;

	muiAlignment = (uint8)uiAlignment;
	miSuppliedOffset = (uint8)iOffset;
	muiElementSize = (uint16)uiElementSize;

	mcList.Init();
	mpsUnused = NULL;
	mpsNotFull = NULL;

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
	}

	mcList.Kill();
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
void* CFreeList::Add(SFNode** ppsNode)
{
	SFNode*		psNode;
	void*		pvBitArray;
	size		iPosition;
	void*		pvTemp;

	if (mcList.GetHead() == NULL)
	{
		pvTemp = AllocateNewSetFirst();
		if (ppsNode)
		{
			*ppsNode = mpsNotFull;
		}
		return pvTemp;
	}

	//Find the first node which is empty.
	if (mpsNotFull)
	{
		psNode = mpsNotFull;
		if (mpsNotFull->bFull)
		{
			psNode = (SFNode*)mcList.GetHead();
		}
	}
	else
	{
		psNode = (SFNode*)mcList.GetHead();
	}

	for (;;)
	{
		while (psNode->bFull)
		{
			psNode = (SFNode*)mcList.GetNext(psNode);
			if (psNode == NULL)
			{
				return AllocateNewSetFirst();
			}
		}

		pvBitArray = GetBitArray(psNode);
		iPosition = FindFirstClearBit(pvBitArray, psNode->uiChunkSize);

		if (iPosition == ARRAY_ELEMENT_NOT_FOUND)
		{
			psNode->bFull = true;
			if (mcList.GetNext(psNode) == NULL)
			{
				pvTemp = AllocateNewSetFirst();
				if (ppsNode)
				{
					*ppsNode = mpsNotFull;
				}
				return pvTemp;
			}
		}
		else
		{
			pvTemp = AllocateExisting(psNode, iPosition);
			if (ppsNode)
			{
				*ppsNode = psNode;
			}
			return pvTemp;
		}
	}
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
	size		iTotalSize;
	size		iBitArraySize;
	size		iChunkSize;

	iChunkSize = CalculateNextChunkSize(mcList.NumElements());

	//Find the size of this free list block.  It is integer aligned.
	//miChunkSize+1 ... -1 to ensure enough space given alignment.
	iBitArraySize = CalculateBitArraySize((size)iChunkSize);

	if (muiAlignment != 1)
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (muiStride * (iChunkSize + 1)) - 1;
	}
	else
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (muiElementSize * iChunkSize);
	}

	if (mpsUnused)
	{
		psNew = mpsUnused;
		
		mcList.InsertDetachedAfterTail(mpsUnused);
		mpsUnused = NULL;
		iChunkSize = psNew->uiChunkSize;
	}
	else
	{
		psNew = (SFNode*)mcList.InsertAfterTail(iTotalSize);
	}

	memset(psNew, 0, sizeof(SFNode) + iBitArraySize);
	mpsNotFull = psNew;
	psNew->iOffset = CalculateOffset(psNew);
	psNew->pcList = this;
	psNew->uiChunkSize = (size)iChunkSize;
	return psNew;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Deallocate(SFNode* psNode)
{
	if (mpsUnused)
	{
		mcList.Remove(psNode);
	}
	else
	{
		mpsUnused = psNode;
		mcList.Detach(psNode);
	}
	if (mpsNotFull == psNode)
	{
		mpsNotFull = NULL;
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
void* CFreeList::AllocateNewSetFirst(void)
{
	SFNode*		psNode;
	void*		pvBitArray;
	void*		pvData;

	psNode = AllocateNew();
	pvBitArray = GetBitArray(psNode);
	SetBit(0, pvBitArray, 1);
	pvData = GetElementInNode(psNode, 0);
	return pvData;
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
void* CFreeList::AllocateExisting(SFNode*	psNode, size iPosition)
{
	void*	pvBitArray;
	void*	pvData;

	pvBitArray = GetBitArray(psNode);
	SetBit(iPosition, pvBitArray, 1);
	pvData = GetElementInNode(psNode, iPosition);
	return pvData;
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

	//Mark this element as no longer in use.
	SetBit(iPosition, pvBitArray, 0);
	psNode->bFull = false;
	mpsNotFull = psNode;

	//Check if any elements are still allocated in this chunk.
	if (FindFirstSetBit(pvBitArray, psNode->uiChunkSize) == ARRAY_ELEMENT_NOT_FOUND)
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
	bool		bPrev;

	psNode = psIterator->psCurrent;
	if (psNode && (psIterator->iPosition != ARRAY_ELEMENT_NOT_FOUND))
	{
		iPosition = psIterator->iPosition;
		bPrev = FindPrevAllocatedElement(psIterator);  //Step the iterator back so iteration can continue.
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
	SFNode*		psNode;
	size		iCount;

	iCount = 0;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		iCount += NumNodeElements(psNode);
		psNode = (SFNode*)mcList.GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeList::HasElements(void)
{
	SFNode*		psNode;
	size		iCount;

	iCount = 0;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		if (NumNodeElements(psNode) > 0)
		{
			return true;
		}
		psNode = (SFNode*)mcList.GetNext(psNode);
	}
	return false;
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
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return psNode->uiChunkSize;
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		return CountBits(pvBitArray, psNode->uiChunkSize);
	}
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

