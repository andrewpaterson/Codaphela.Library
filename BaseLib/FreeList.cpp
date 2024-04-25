#include <stdlib.h>
#include <memory.h>
#include "FreeList.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(uint16 iElementSize)
{
	Init(iElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(uint16 iElementSize, char iAlignment)
{
	Init(iElementSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(uint16 iElementSize, char iAlignment, char iOffset)
{
	muiMagic = FREE_LIST_MAGIC;

	miAlignment = iAlignment;
	miSuppliedOffset = iOffset;
	miElementSize = iElementSize;

	mcList.Init();
	mpsUnused = NULL;
	mpsNotFull = NULL;

	miOffset = CalculateOffset(iOffset);
	miStride = CalculateStride();
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
	miElementSize = 0;
	miAlignment = 0;
	miStride = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CFreeList::CalculateStride(void)
{
	return ::CalculateStride(miElementSize + miOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char CFreeList::CalculateOffset(int32 iOffset)
{
	return ::CalculateOffset(iOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CFreeList::CalculateOffset(SFNode* psNode)
{
	int32	iBitArraySize;
	int32	iStart;
	int32	iByteDiff;
	char	iOffset;

	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iStart = ((int32)(size_t) psNode) + sizeof(SFNode) + iBitArraySize;

	iByteDiff = iStart % miAlignment;
	if (iByteDiff == 0)
	{
		return miOffset;
	}
	else
	{
		iOffset = (miAlignment - iByteDiff);
		return miOffset + iOffset;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CFreeList::CalculateBitArraySize(uint16 uiChunkSize)
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
	int32		iPosition;
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

		if (iPosition == -1)
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
	memcpy(pvTemp, pvData, miElementSize);
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
void CFreeList::RemoveExisiting(SFNode* psNode, void* pvData)
{
	int32	iBitArraySize;
	int32	iNodeStart;
	int32	iDataPos;
	int32	iPos;

	//Find out the position of the element in the chunk.
	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iNodeStart = (int32)((size_t) psNode) + (iBitArraySize + sizeof(SFNode)) + psNode->iOffset;
	iDataPos = (int32)((size_t) pvData);
	iPos = (iDataPos - iNodeStart)/miStride;

	//Remove the element from the chunk.
	RemoveExisiting(psNode, iPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CalculateNextChunkSize(int32 iNumBlocks)
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
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::AllocateNew(void)
{
	SFNode*		psNew;
	int32		iTotalSize;
	int32		iBitArraySize;
	int32		iChunkSize;

	iChunkSize = CalculateNextChunkSize(mcList.NumElements());

	//Find the size of this free list block.  It is integer aligned.
	//miChunkSize+1 ... -1 to ensure enough space given alignment.
	iBitArraySize = CalculateBitArraySize((uint16)iChunkSize);

	if (miAlignment != 1)
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (miStride * (iChunkSize + 1)) - 1;
	}
	else
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (miElementSize * iChunkSize);
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
	psNew->uiChunkSize = (uint16)iChunkSize;
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
	psParams->iAlignment = miAlignment;
	psParams->iElementSize = miElementSize;
	psParams->iOffset = miSuppliedOffset;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::GetNode(int32 iNode)
{
	SFNode*		psNode;
	int32		iCount;

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
bool CFreeList::IsElementInNodeAllocated(SFNode* psNode, int32 iPosition)
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
void* CFreeList::GetElementInNode(SFNode* psNode, int32 iPosition)
{
	void*	pvData;
	int32	iElementOffset;
	int32	iBitArraySize;

	iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
	iElementOffset = iPosition * miStride;
	pvData = RemapSinglePointer(psNode, sizeof(SFNode) + iBitArraySize + psNode->iOffset + iElementOffset);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetElement(SFNode* psNode, int32 iPosition)
{
	if ((iPosition < 0) || (iPosition >= psNode->uiChunkSize))
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
void* CFreeList::AllocateExisting(SFNode*	psNode, int32 iPosition)
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
bool CFreeList::RemoveExisiting(SFNode* psNode, int32 iPosition)
{
	void*	pvBitArray;

	//Get the position of the start of the bit array.
	pvBitArray = GetBitArray(psNode);

	//Mark this element as no longer in use.
	SetBit(iPosition, pvBitArray, 0);
	psNode->bFull = false;
	mpsNotFull = psNode;

	//Check if any elements are still allocated in this chunk.
	if (FindFirstSetBit(pvBitArray, psNode->uiChunkSize) == -1)
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
void* CFreeList::Get(int32 iElement)
{
	//Unallocated elements in the free list are not included.
	//This is the normal operation.

	int32				i;
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
		if (psIterator->iPosition == -1)
		{
			return StartIteration(psIterator);
		}
		return NULL;
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
void CFreeList::RemoveDuringIteration(SFreeListIterator* psIterator)
{
	SFNode*		psNode;
	int32		iPosition;

	psNode = psIterator->psCurrent;
	if (psNode)
	{
		iPosition = psIterator->iPosition;
		FindPrevAllocatedElement(psIterator);  //Step the iterator back so iteration can continue.
		RemoveExisiting(psNode, iPosition);
	}
	else
	{
		if (psIterator->iPosition == -1)
		{
			StartIteration(psIterator);
			psNode = psIterator->psCurrent;
			if (psNode)
			{
				iPosition = psIterator->iPosition;
				RemoveExisiting(psNode, iPosition);
				FindPrevAllocatedElement(psIterator);  //Step the iterator back so iteration can continue.
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::FindNextAllocatedElement(SFreeListIterator* psIterator)
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
				return;
			}
			pvBitArray = GetBitArray(psIterator->psCurrent);
		}

		if (GetBit(psIterator->iPosition, pvBitArray))
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::FindPrevAllocatedElement(SFreeListIterator* psIterator)
{
	void*	pvBitArray;

	//Assumes that the iterator position must be moved.
	pvBitArray = GetBitArray(psIterator->psCurrent);
	for (;;)
	{
		psIterator->iPosition--;

		if (psIterator->iPosition < 0)
		{
			psIterator->psCurrent = (SFNode*)mcList.GetPrev(psIterator->psCurrent);
			if (psIterator->psCurrent == NULL)
			{
				psIterator->iPosition = -1;
				return;
			}
			psIterator->iPosition = psIterator->psCurrent->uiChunkSize-1;
			pvBitArray = GetBitArray(psIterator->psCurrent);
		}

		if (GetBit(psIterator->iPosition, pvBitArray))
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::FindNode(void* pvData, bool bIsAllocated)
{
	int32		iDataPos;
	int32		iNodeStart;
	int32		iNodeEnd;
	SFNode*		psNode;
	int32		iBitArraySize;
	int32		iPos;
	void*		pvBitArray;

	iDataPos = (int32)((size_t) pvData);
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		iBitArraySize = CalculateBitArraySize(psNode->uiChunkSize);
		iNodeStart = (int32)((size_t) psNode) + (iBitArraySize + sizeof(SFNode)) + psNode->iOffset;
		iNodeEnd = iNodeStart + (miStride * (psNode->uiChunkSize -1));
		if ((iDataPos >= iNodeStart) && (iDataPos <= iNodeEnd))
		{
			//The node is in this block.
			if (bIsAllocated)
			{
				pvBitArray = GetBitArray(psNode);
				iPos = (iDataPos - iNodeStart)/miStride;
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
int32 CFreeList::ByteSize(void)
{
	//This returns just the size of the allocated memory allocated blocks.  It does not include the free list 'header' or the link list 'header. 
	return mcList.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::SetAdditionalSize(int32 iSize)
{
	//Make sure no allocations have been done.
	if ((mcList.GetHead() == NULL) && (mcList.GetTail() == NULL))
	{
		miElementSize += iSize;
		miStride = CalculateStride();
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
int32 CFreeList::NumElements(void)
{
	SFNode*		psNode;
	int32		iCount;

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
	int32		iCount;

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
int32 CFreeList::NumNodeElements(SFNode* psNode)
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
	int32	iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, 0);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindFirstSetBit(pvBitArray, psNode->uiChunkSize);
		if (iPosition == -1)
		{
			return NULL;
		}

		if ((iPosition < 0) || (iPosition >= psNode->uiChunkSize))
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
	int32	iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, psNode->uiChunkSize -1);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindLastSetBit(pvBitArray, psNode->uiChunkSize);
		if (iPosition == -1)
		{
			return NULL;
		}

		if ((iPosition < 0) || (iPosition >= psNode->uiChunkSize))
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
int32 CFreeList::GetElementSize(void) { return miElementSize; }
int32 CFreeList::GetElementStride(void) {return miStride; }
int32 CFreeList::GetNumAllocatedChunks(void) { return mcList.NumElements(); }
char CFreeList::GetOffset(void) { return miOffset; }
int32 CFreeList::GetAlignment(void) { return miAlignment; }

