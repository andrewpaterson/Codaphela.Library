#include <stdlib.h>
#include <memory.h>
#include "FreeList.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(int iChunkSize, int iElementSize)
{
	Init(iChunkSize, iElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(int iChunkSize, int iElementSize, char iAlignment)
{
	Init(iChunkSize, iElementSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::Init(int iChunkSize, int iElementSize, char iAlignment, char iOffset)
{
	miChunkSize = iChunkSize;
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
int CFreeList::CalculateStride(void)
{
	return ::CalculateStride(miElementSize + miOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char CFreeList::CalculateOffset(int iOffset)
{
	return ::CalculateOffset(iOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::CalculateOffset(SFNode* psNode)
{
	int					iBitArraySize;
	int					iStart;
	int					iByteDiff;
	char				iOffset;

	iBitArraySize = CalculateBitArraySize();
	iStart = ((int)(ENGINE_SIZE_T) psNode) + sizeof(SFNode) + iBitArraySize;

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
int CFreeList::CalculateBitArraySize(void)
{
	//The bit array size is the number of elements in a chunk divided by 8 (+1).

	//Except this isn't what this does.
	return (((miChunkSize >> 3) + 4) & 0xfffffffc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Add(SFNode** ppsNode)
{
	SFNode*		psNode;
	void*		pvBitArray;
	int			iPosition;
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
		iPosition = FindFirstClearBit(pvBitArray, miChunkSize);

		if (iPosition == -1)
		{
			psNode->bFull = TRUE;
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
BOOL CFreeList::Remove(void* pvData)
{
	SFNode*		psNode;

	if (!pvData)
	{
		return FALSE;
	}

	//Find the chunk in which the element is within this free list.
	psNode = FindNode(pvData);

	//Check the chunk could be found.
	if (psNode)
	{
		RemoveExisiting(psNode, pvData);
		return TRUE;
	}

	//Could not find the chunk so could not remove the element.
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFreeList::Remove(SFNode* psNode, void* pvData)
{
	if (psNode && pvData)
	{
		RemoveExisiting(psNode, pvData);
		return TRUE;
	}

	//Could not find the chunk so could not remove the element.
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::RemoveExisiting(SFNode* psNode, void* pvData)
{
	int			iBitArraySize;
	int			iNodeStart;
	int			iDataPos;
	int			iPos;

	//Find out the position of the element in the chunk.
	iBitArraySize = CalculateBitArraySize();
	iNodeStart = (int)((ENGINE_SIZE_T) psNode) + (iBitArraySize + sizeof(SFNode)) + psNode->iOffset;
	iDataPos = (int)((ENGINE_SIZE_T) pvData);
	iPos = (iDataPos - iNodeStart)/miStride;

	//Remove the element from the chunk.
	RemoveExisiting(psNode, iPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::AllocateNew(void)
{
	SFNode*		psNew;
	int			iTotalSize;
	int			iBitArraySize;

	//Find the size of this free list block.  It is integer aligned.
	//miChunkSize+1 ... -1 to ensure enough space given alignment.
	iBitArraySize = CalculateBitArraySize();

	if (miAlignment != 1)
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (miStride * (miChunkSize+1)) - 1;
	}
	else
	{
		iTotalSize = sizeof(SFNode) + iBitArraySize + (miElementSize * miChunkSize);
	}

	if (mpsUnused)
	{
		psNew = mpsUnused;
		mcList.InsertDetachedAfterTail(mpsUnused);
		mpsUnused = NULL;
	}
	else
	{
		psNew = (SFNode*)mcList.InsertAfterTail(iTotalSize);
	}
	memset(psNew, 0, sizeof(SFNode) + iBitArraySize);
	mpsNotFull = psNew;
	psNew->iOffset = CalculateOffset(psNew);
	psNew->pcList = this;
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
	psParams->iChunkSize = miChunkSize;
	psParams->iElementSize = miElementSize;
	psParams->iOffset = miSuppliedOffset;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SFNode* CFreeList::GetNode(int iNode)
{
	SFNode*		psNode;
	int			iCount;

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
BOOL CFreeList::IsElementInNodeAllocated(SFNode* psNode, int iPosition)
{
	void*	pvBitArray;
	BOOL	bAllocated;

	pvBitArray = GetBitArray(psNode);
	bAllocated = GetBit(iPosition, pvBitArray);
	if (bAllocated)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetElementInNode(SFNode* psNode, int iPosition)
{
	void*		pvData;
	int		iElementOffset;
	int		iBitArraySize;

	iBitArraySize = CalculateBitArraySize();
	iElementOffset = iPosition * miStride;
	pvData = RemapSinglePointer(psNode, sizeof(SFNode) + iBitArraySize + psNode->iOffset + iElementOffset);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetElement(SFNode* psNode, int iPosition)
{
	if ((iPosition < 0) || (iPosition >= miChunkSize))
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
	void*			pvData;

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
void* CFreeList::AllocateExisting(SFNode*	psNode, int iPosition)
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
BOOL CFreeList::RemoveExisiting(SFNode* psNode, int iPosition)
{
	void*		pvBitArray;

	//Get the position of the start of the bit array.
	pvBitArray = GetBitArray(psNode);

	//Mark this element as no longer in use.
	SetBit(iPosition, pvBitArray, 0);
	psNode->bFull = FALSE;
	mpsNotFull = psNode;

	//Check if any elements are still allocated in this chunk.
	if (FindFirstSetBit(pvBitArray, miChunkSize) == -1)
	{
		//If not deallocate the element.
		Deallocate(psNode);

		//Return true specifying that the chunk has been deallocated.
		return TRUE;
	}

	//Return false specifying that the chunk contains elements and has not been deallocated.
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::Get(int iElement)
{
	//Unallocated elements in the free list are not included.
	//This is the normal operation.

	int					i;
	SFreeListIterator	sFreeListIterator;
	void*					p;

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
	int			iPosition;

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

		if (psIterator->iPosition >= miChunkSize)
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
			psIterator->iPosition = miChunkSize-1;
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
SFNode* CFreeList::FindNode(void* pvData, BOOL bIsAllocated)
{
	int				iDataPos;
	int				iNodeStart;
	int				iNodeEnd;
	SFNode*			psNode;
	int				iBitArraySize;
	int				iPos;
	void*			pvBitArray;

	iBitArraySize = CalculateBitArraySize();

	iDataPos = (int)((ENGINE_SIZE_T) pvData);
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		iNodeStart = (int)((ENGINE_SIZE_T) psNode) + (iBitArraySize + sizeof(SFNode)) + psNode->iOffset;
		iNodeEnd = iNodeStart + (miStride * (miChunkSize-1));
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
int CFreeList::ByteSize(void)
{
	//This returns just the size of the allocated memory allocated blocks.  It does not include the free list 'header' or the link list 'header. 
	return mcList.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeList::SetAdditionalSize(int iSize)
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
BOOL CFreeList::Contains(void* pvData, BOOL bIsAllocated)
{
	SFNode*		psNode;

	psNode = FindNode(pvData, bIsAllocated);
	if (psNode)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::NumElements(void)
{
	SFNode*		psNode;
	int			iCount;

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
int CFreeList::NumNodeElements(SFNode* psNode)
{
	void*		pvBitArray;

	if (psNode->bFull)
	{
		return miChunkSize;
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		return CountBits(pvBitArray, miChunkSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CFreeList::GetFirstNodeElement(SFNode* psNode)
{
	int		iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, 0);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindFirstSetBit(pvBitArray, miChunkSize);
		if (iPosition == -1)
		{
			return NULL;
		}

		if ((iPosition < 0) || (iPosition >= miChunkSize))
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
	int		iPosition;
	void*	pvBitArray;

	if (psNode->bFull)
	{
		return GetElementInNode(psNode, miChunkSize-1);
	}
	else
	{
		pvBitArray = GetBitArray(psNode);
		iPosition = FindLastSetBit(pvBitArray, miChunkSize);
		if (iPosition == -1)
		{
			return NULL;
		}

		if ((iPosition < 0) || (iPosition >= miChunkSize))
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
int CFreeList::GetElementSize(void)
{
	return miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::GetElementStride(void)
{
	return miStride;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::GetNumAllocatedChunks(void)
{
	return mcList.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char CFreeList::GetOffset(void)
{
	return miOffset;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::GetAlignment(void)
{
	return miAlignment;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CFreeList::GetChunkSize(void)
{
	return miChunkSize;
}

