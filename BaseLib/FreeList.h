/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FREE_LIST_H__
#define __FREE_LIST_H__
#include "IntegerHelper.h"
#include "LinkListBlock.h"
#include "DataTypes.h"
#include "IntegerHelper.h"


struct SFNode
{
	BOOL	bFull;
	int		iOffset;
	void*	pcList;	  //Rember this is only valid provided the freelist never moves in memory.

	//For testing
	int*	GetBitArray(void);
};


struct SFreeListIterator
{
	SFNode*		psCurrent;  //Current FreeList node.
	int			iPosition;  //Element within this node.
};


template<class M>
class __CFreeList
{
private:
	CLinkListBlock	mcList;
	SFNode*			mpsUnused;
	SFNode*			mpsNotFull;
	int				miAlignment;
	int				miStride;
	int				miOffset;

public:
	void		Kill(void);

	M*			Add(SFNode** ppsNode = NULL);
	M*			Add(M* pvData);
	BOOL		Remove(M* pvData);
	BOOL		Remove(SFNode* psNode, M* pvData);
	M*			Get(int iElement);
	M*			StartIteration(SFreeListIterator* psIterator);
	M*			Iterate(SFreeListIterator* psIterator);
	void		RemoveDuringIteration(SFreeListIterator* psIterator);
	BOOL		Contains(M* pvData, BOOL bIsAllocated = TRUE);
	int			NumElements(void);
	void		SetAdditionalSize(int iSize);
	int			GetElementSize(void);
	int			GetElementStride(void);
	int			GetNumAllocatedChunks(void);
	SFNode*		GetNode(int iNode);
	M*			GetElement(SFNode* psNode, int iPosition);
	int			GetOffset(void);
	int			GetAlignment(void);
	void*		GetBitArray(SFNode* psNode);
	SFNode* 	FindNode(M* pvData, BOOL bIsAllocated = TRUE);
	int			ByteSize(void);

private:
	SFNode* AllocateNew(void);
	void	Deallocate(SFNode* psNode);
	M*		AllocateExisting(SFNode* psNode, int iPosition);
	M*		AllocateNewSetFirst(void);
	void	FindNextAllocatedElement(SFreeListIterator* psIterator);
	void	FindPrevAllocatedElement(SFreeListIterator* psIterator);
	BOOL	RemoveExisiting(SFNode* psNode, int iPosition);
	int		CalculateStride(void);
	int		CalculateOffset(int iOffset);
	int		CalculateBitArraySize(void);
	int		CalculateOffset(SFNode* psNode);
	M*		GetElementInNode(SFNode* psNode, int iPosition);
	BOOL	IsElementInNodeAllocated(SFNode* psNode, int iPosition);
	void	RemoveExisiting(SFNode* psNode, M* pvData);

protected:
	int		miElementSize;
	int		miChunkSize;

	void	Init(int iElementSize);
	void	Init(int iChunkSize, int iElementSize);
	void	Init(int iChunkSize, int iElementSize, int iAlignment);
	void	Init(int iChunkSize, int iElementSize, int iAlignment, int iOffset);
};


template<class M>
class CFreeList : public __CFreeList<M>
{
public:
	void	Init(void);
	void	Init(int iChunkSize);
	void	Init(int iChunkSize, int iAlignment);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::Init(int iElementSize)
{
	Init(32, iElementSize);
	miElementSize = iElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::Init(int iChunkSize, int iElementSize)
{
	Init(iChunkSize, iElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::Init(int iChunkSize, int iElementSize, int iAlignment)
{
	Init(iChunkSize, iElementSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::Init(int iChunkSize, int iElementSize, int iAlignment, int iOffset)
{
	miChunkSize = iChunkSize;
	miAlignment = iAlignment;
	mcList.Init();
	mpsUnused = NULL;
	mpsNotFull = NULL;
	miElementSize = iElementSize;

	miOffset = CalculateOffset(iOffset);
	miStride = CalculateStride();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::Kill(void)
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
template<class M>
int __CFreeList<M>::CalculateStride(void)
{
	return ::CalculateStride(miElementSize + miOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::CalculateOffset(int iOffset)
{
	return ::CalculateOffset(iOffset, miAlignment);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::CalculateOffset(SFNode* psNode)
{
	int		iBitArraySize;
	int		iStart;
	int		iByteDiff;
	int		iOffset;

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
template<class M>
int __CFreeList<M>::CalculateBitArraySize(void)
{
	//The bit array size is the number of elements in a chunk divided by 8 (+1).
	
	//Except this isn't what this does.
	return (((miChunkSize >> 3) + 4) & 0xfffffffc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CFreeList<M>::Add(SFNode** ppsNode)
{
	SFNode*		psNode;
	void*		pvBitArray;
	int			iPosition;
	M*			pvTemp;

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

	if (ppsNode)
	{
		*ppsNode = NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CFreeList<M>::Add(M* pvData)
{
	M*	pvTemp;

	pvTemp = Add();
	memcpy(pvTemp, pvData, miElementSize);
	return pvTemp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CFreeList<M>::Remove(M* pvData)
{
	SFNode*		psNode;

	if (!pvData)
	{
		return FALSE;
	}

	//Find the chunk in which the element is within this freelist.
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
template<class M>
BOOL __CFreeList<M>::Remove(SFNode* psNode, M* pvData)
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
template<class M>
void __CFreeList<M>::RemoveExisiting(SFNode* psNode, M* pvData)
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
template<class M>
SFNode* __CFreeList<M>::AllocateNew(void)
{
	SFNode*		psNew;
	int			iTotalSize;
	int			iBitArraySize;

	//Find the size of this free list block.  It is integer alligned.
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
		psNew = (SFNode*)mcList.InsertAfterTail(iTotalSize, -1);
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
template<class M>
void __CFreeList<M>::Deallocate(SFNode* psNode)
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
template<class M>
SFNode* __CFreeList<M>::GetNode(int iNode)
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
template<class M>
BOOL __CFreeList<M>::IsElementInNodeAllocated(SFNode* psNode, int iPosition)
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
template<class M>
M* __CFreeList<M>::GetElementInNode(SFNode* psNode, int iPosition)
{
	M*		pvData;
	int		iElementOffset;
	int		iBitArraySize;

	iBitArraySize = CalculateBitArraySize();
	iElementOffset = iPosition * miStride;
	pvData = (M*)((int)((ENGINE_SIZE_T) psNode) + sizeof(SFNode) + iBitArraySize + psNode->iOffset + iElementOffset);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CFreeList<M>::GetElement(SFNode* psNode, int iPosition)
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
template<class M>
M* __CFreeList<M>::AllocateNewSetFirst(void)
{
	SFNode*		psNode;
	void*		pvBitArray;
	M*			pvData;

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
template<class M>
void* __CFreeList<M>::GetBitArray(SFNode* psNode)
{
	return (void*)((int)((ENGINE_SIZE_T) psNode) + sizeof(SFNode));
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CFreeList<M>::AllocateExisting(SFNode*	psNode, int iPosition)
{
	void*	pvBitArray;
	M*		pvData;

	pvBitArray = GetBitArray(psNode);
	SetBit(iPosition, pvBitArray, 1);
	pvData = GetElementInNode(psNode, iPosition);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CFreeList<M>::RemoveExisiting(SFNode* psNode, int iPosition)
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
template<class M>
M* __CFreeList<M>::Get(int iElement)
{
	//Unallocated elements in the freelist are not included.
	//This is the normal operation.

	int					i;
	SFreeListIterator	sFreeListIterator;
	M*					p;

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
template<class M>
M* __CFreeList<M>::StartIteration(SFreeListIterator* psIterator)
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
template<class M>
M* __CFreeList<M>::Iterate(SFreeListIterator* psIterator)
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
template<class M>
void __CFreeList<M>::RemoveDuringIteration(SFreeListIterator* psIterator)
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
template<class M>
void __CFreeList<M>::FindNextAllocatedElement(SFreeListIterator* psIterator)
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
template<class M>
void __CFreeList<M>::FindPrevAllocatedElement(SFreeListIterator* psIterator)
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
template<class M>
SFNode* __CFreeList<M>::FindNode(M* pvData, BOOL bIsAllocated)
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
template<class M>
int __CFreeList<M>::ByteSize(void)
{
	//This returns just the size of the allocated (malloc'd) blocks.  It does not include the free list 'header' or the link list 'header. 
	return mcList.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CFreeList<M>::SetAdditionalSize(int iSize)
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
template<class M>
BOOL __CFreeList<M>::Contains(M* pvData, BOOL bIsAllocated)
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
template<class M>
int __CFreeList<M>::NumElements(void)
{
	SFNode*		psNode;
	int			iCount;
	void*		pvBitArray;
	int			i;

	iCount = 0;
	psNode = (SFNode*)mcList.GetHead();
	while (psNode)
	{
		if (psNode->bFull)
		{
			iCount += miChunkSize;
		}
		else
		{
			pvBitArray = GetBitArray(psNode);
			for (i = 0; i < miChunkSize; i++)
			{
				if (GetBit(i, pvBitArray))
				{
					iCount++;
				}
			}
		}

		psNode = (SFNode*)mcList.GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::GetElementSize(void)
{
	return miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::GetElementStride(void)
{
	return miStride;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::GetNumAllocatedChunks(void)
{
	SFNode*		psNode;
	int			iCount;

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
template<class M>
int __CFreeList<M>::GetOffset(void)
{
	return miOffset;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CFreeList<M>::GetAlignment(void)
{
	return miAlignment;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CFreeList<M>::Init(void)
{
	__CFreeList<M>::Init(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CFreeList<M>::Init(int iChunkSize)
{
	__CFreeList<M>::Init(iChunkSize, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CFreeList<M>::Init(int iChunkSize, int iAlignment)
{
	__CFreeList<M>::Init(iChunkSize, sizeof(M), iAlignment);
}


#endif //__FREE_LIST_H__

