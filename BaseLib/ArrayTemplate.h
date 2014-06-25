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
#ifndef __TEMPLATE_ARRAY_H__
#define __TEMPLATE_ARRAY_H__
#include "FastMemset.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"
#include "FileIO.h"


struct SArrayTemplateHeader
{
	int			miElementSize;
	int			miUsedElements;
	int			miChunkSize;
};


template<class M>
class __CArrayTemplate : protected SArrayTemplateHeader
{
protected:
	int		miNumElements;
	M*		mpvArray;

public:
	void 	InitFromHeader(void);
	void 	Init(SArrayTemplateHeader* psHeader);
	void	Init(__CArrayTemplate<M>* pcTemplateArray);
	void 	Kill(void);
	void 	Finalise(void);

	int		NumElements(void);
	BOOL	IsEmpty(void);
	BOOL	IsNotEmpty(void);
	int		AllocatedElements(void);
	int 	ElementSize(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(int* piIndex);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);

	void	BatchInsertElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);
	void	BatchRemoveElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);

	BOOL 	Copy(__CArrayTemplate<M>* pcTemplateArray);

	M*		Get(int iIndex);
	M*		SafeGet(int iIndex);
	int		GetAdjustedIndex(int iIndex);
	M*		GetData(void);
	void	GetHeader(SArrayTemplateHeader* psHeader);
	int		GetIndex(M* pvElement);
	M*		Tail(void);

	void	InsertArrayAfterEnd(__CArrayTemplate<M>* pcTemplateArray);
	void	InsertArrayAt(__CArrayTemplate<M>* pcTemplateArray, int iIndex);
	void	InsertArrayBeforeStart(__CArrayTemplate<M>* pcTemplateArray);
	M* 		InsertAt(int iIndex);
	M*	 	InsertAt(M* pvData, int iIndex);
	void	InsertBlockAfterEnd(M* paElements, int iLength);
	void	InsertBlockAt(M* paElements, int iIndex, int iLength);
	void	InsertBlockBeforeStart(M* paElements, int iLength);
	int		InsertIntoSorted(int(*)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting);
	M*		InsertNumElementsAt(int iNumElements, int iIndex);

	void	Pop(M* pvData);
	void	Pop(void);
	void 	Push(M* pvElement);
	M*		Push(void);
	void 	PushCopy(void);

	void	GrowByChunk(void);
	int		GrowByNumElements(int iNumElements);
	M*		GrowToAtLeastNumElements(int iNumElements, BOOL bClear = FALSE, int iClear = 0);  //ie:  Don't shrink the array.
	int		GrowToNumElements(int iNumElements);  //Can shrink the array.  Should probably call this resize.

	void	BubbleSort(int(*)(const void*, const void*));
	void	QuickSort(int(*)(const void*, const void*));
	void	Reverse(void);

	BOOL	Contains(M* pData);
	BOOL	Equals(__CArrayTemplate<M>* pcTemplateArray);
	int 	Find(M* pData);
	BOOL	FindInSorted(M* pData, int(*)(const void*, const void*), int* piIndex);
	int		FindWithIntKey(int iKey);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	int 	FindWithKey(M* pData, int iKeyOffset, int iKeySize);

	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	void	RemoveAt(int* paiIndex, int iNumElements, BOOL bPreserveOrder = TRUE);
	void	RemoveAtNoDeallocate(int iIndex, int bPreserveOrder);
	void	RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder = TRUE);
	void 	RemoveTail(void);

	void	Set(int iIndex, M* pvData);
	BOOL	SafeSet(int iIndex, M* pvData);
	void	Swap(int iIndex1, int iIndex2);
	void	Unuse(void);
	void 	Zero(void);

	int 	ByteSize(void);
	int		ChunkSize(void);
	void	SetUsedElements(int iNumElements);

	BOOL	WriteArrayTemplateHeader(CFileWriter* pcFileWriter);
	BOOL	WriteArrayTemplate(CFileWriter* pcFileWriter);
	BOOL	ReadArrayTemplateHeader(CFileReader* pcFileReader);
	BOOL	ReadArrayTemplate(CFileReader* pcFileReader);

protected:
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);

	BOOL	BinarySearch(M* pData, int iLeft, int iRight, int(*)(const void*, const void*), int* piIndex);
	void	CopyArrayInto(__CArrayTemplate<M>* pcTemplateArray, int iIndex);
	void	CopyBlockInto(M* paElements, int iLength, int iIndex);
	void	PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void	PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize);
	void	RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize);
	void 	SetArraySize(int iNumElements);
};


template<class M>
class CArrayTemplate : public __CArrayTemplate<M>
{
public:
	void 	Init(void);
	void 	Init(int iChunkSize);
	void	Init(SArrayTemplateHeader* psHeader);
	void	Init(__CArrayTemplate<M>* pcTemplateArray);

	void 	Allocate(int iNumElements);
	void 	Allocate(int iChunkSize, int iNumElements);
	void	Fake(M* pvData, int iNum);
	void	FakeSetUsedElements(int iUsedElements);
	M*		Get(int iIndex);
	void 	ReInit(int iChunkSize = 0);

	M&		operator[](int iIndex);

	void 	SetAllocateSize(int iSize);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CArrayTemplate<M>::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CArrayTemplate<M>::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InitFromHeader(void)
{
	//This function assumes that the value of mpvArray was invalid.
	mpvArray = (M*)Malloc(miElementSize * miNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Init(SArrayTemplateHeader* psHeader)
{
	miElementSize = psHeader->miElementSize;
	miUsedElements = psHeader->miUsedElements;
	miChunkSize = psHeader->miChunkSize;
	mpvArray = NULL;
	miNumElements = 0;
	SetUsedElements(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Init(__CArrayTemplate<M>* pcTemplateArray)
{
	miElementSize = pcTemplateArray->miElementSize;
	miUsedElements = 0;
	miChunkSize = pcTemplateArray->miChunkSize;
	miNumElements = 0;
	this->mpvArray = NULL;
	Copy(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::SafeGet(int iIndex)
{
	if ((iIndex < 0) || (iIndex >= miUsedElements))
	{
		return NULL;
	}
	else
	{
		return Get(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::SafeSet(int iIndex, M* pvData)
{
	int		iOldLength;

	if (iIndex < 0)
	{
		return FALSE;
	}
	else if (iIndex < miUsedElements)
	{
		Set(iIndex, pvData);
		return TRUE;
	}
	else
	{
		iOldLength = NumElements();
		SetUsedElements(iIndex+1);
		memset_fast(Get(iOldLength), 0, (iIndex-iOldLength) * miElementSize);
		Set(iIndex, pvData);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::SetArraySize(int iNumElements)
{
	M* ptr;

	if (iNumElements == 0)
	{
		if (mpvArray != NULL)
		{
			Free(mpvArray);
			mpvArray = NULL;
			miNumElements = 0;
		}
	}
	else
	{
		if (miNumElements != iNumElements)
		{
			ptr = (M*)Realloc(mpvArray, miElementSize * iNumElements);
			mpvArray = ptr;
			if (mpvArray != NULL)
			{
				miNumElements = iNumElements;
			}
			else
			{
				miUsedElements = miNumElements = 0;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::Add(void)
{
	miUsedElements++;
	if (miUsedElements > miNumElements)
	{
		SetArraySize(miNumElements + miChunkSize);
	}
	return Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::AddGetIndex(int* piIndex)
{
	miUsedElements++;
	if (miUsedElements > miNumElements)
	{
		SetArraySize(miNumElements+miChunkSize);
	}
	(*piIndex) = miUsedElements - 1;
	return Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::Add(M* pvData)
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
void __CArrayTemplate<M>::Set(int iIndex, M* pvData)
{
	M*	pvTemp;

	pvTemp = Get(iIndex);
	memcpy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::InsertAt(int iIndex)  //The new element will be at iIndex
{
	void*	pSource;
	void*	pDest;

	//This assumes that iIndex is within the array (or the last element).
	miUsedElements++;
	if (miUsedElements > miNumElements)
	{
		SetArraySize(miNumElements + miChunkSize);
	}
	pSource = Get(iIndex);
	pDest = RemapSinglePointer(pSource, miElementSize);

	memmove(pDest, pSource, miElementSize * (miUsedElements - 1 - iIndex));
	return (M*)pSource;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::InsertAt(M* pvData, int iIndex)
{
	M* ptr;

	ptr = InsertAt(iIndex);
	memcpy(ptr, pvData, miElementSize);
	return ptr;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize)
{
	if ((iIndex < miUsedElements) && (iIndex >= 0))
	{
		RemoveAtNoDeallocate(iIndex, bPreserveOrder, iDataSize);

		if (miUsedElements == miNumElements - miChunkSize)
		{
			SetArraySize(miNumElements - miChunkSize);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize)
{
	void*	pSource;
	void*	pDest;
	void*	pEnd;

	miUsedElements--;
	if (iIndex == miUsedElements)
	{
		return;
	}

	pSource = RemapSinglePointer(mpvArray, iIndex * iDataSize);

	if (bPreserveOrder)
	{
		//If the order of elements is to be preserved then move all the elements back one.
		pDest = (M*)RemapSinglePointer(pSource, iDataSize);
		memmove_fast(pSource, pDest, iDataSize * (miUsedElements - iIndex));
	}
	else
	{
		//If the order is unimportant then just move the last element to the empty.
		pEnd = (M*)RemapSinglePointer(mpvArray, miUsedElements * iDataSize);
		memcpy_fast(pSource, pEnd, iDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveAtNoDeallocate(int iIndex, int bPreserveOrder)
{
	//Once you've used this function you may not use 'RemoveAt'.
	if ((iIndex < miUsedElements) && (iIndex >= 0))
	{
		RemoveAtNoDeallocate(iIndex, bPreserveOrder, miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveAt(int iIndex, int bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize)
{
	int		iNumTrailing;
	int		iNumToRemove;
	void*	pvEnd;
	void*	pvStart;
	void*	pv;

	if (iEndIndexExclusive <= iStartIndex)
	{
		return;
	}

	if (iEndIndexExclusive >= miUsedElements)
	{
		SetUsedElements(iStartIndex);
	}
	else
	{
		iNumToRemove = iEndIndexExclusive - iStartIndex;
		iNumTrailing = miUsedElements - iEndIndexExclusive;
		pvStart = Get(iStartIndex);
		if ((bPreserveOrder) || (iNumToRemove >= iNumTrailing))
		{
			pvEnd = Get(iEndIndexExclusive);
			memmove(pvStart, pvEnd, iNumTrailing*iDataSize);
		}
		else
		{
			pv = Get(miUsedElements - iNumToRemove);
			memmove(pvStart, pv, iNumToRemove*iDataSize);
		}
		SetUsedElements(miUsedElements-iNumToRemove);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveAt(int* paiElementsToDelete, int iNumElementsToDelete, BOOL bPreserveOrder)
{
	int		i;
	int		iIndex;
	int		iFirstElementToMove;
	int		iEndElementToMove;
	int		iMoved;
	int		iDestElement;
	int		iDestOffset;
	int		iNumDeleted;

	//Little gotcha on paiElementsToDelete.  It must be sorted smallest to largest and each value must be unique.

	if (iNumElementsToDelete == 0)
	{
		return;
	}
	
	if (iNumElementsToDelete == miUsedElements)
	{
		SetUsedElements(0);
		return;
	}

	if (bPreserveOrder)
	{
		iIndex = 0;
		iDestOffset = 0;
		for (iFirstElementToMove = 0; iFirstElementToMove < miUsedElements; iFirstElementToMove++)
		{
			if (paiElementsToDelete[iIndex] == iFirstElementToMove)
			{
				break;
			}
		}

		for (;;)
		{
			iDestElement = iFirstElementToMove;
			for (iNumDeleted = 0;; iFirstElementToMove++, iIndex++, iNumDeleted++)
			{
				if (iIndex == iNumElementsToDelete)
				{
					iMoved = miUsedElements - iFirstElementToMove;
					memcpy_fast(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);
					SetUsedElements(miUsedElements - iNumElementsToDelete);
					return;
				}
				if (paiElementsToDelete[iIndex] != iFirstElementToMove)
				{
					iEndElementToMove = paiElementsToDelete[iIndex];
					break;
				}
			}

			//iFirstElementToMove is the first element to move
			//iEndElementToMove is the last element to move exclusive.
			iMoved = iEndElementToMove - iFirstElementToMove;
			memcpy_fast(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);

			iDestOffset += iNumDeleted;

			iFirstElementToMove = iEndElementToMove;
		}
	}
	else
	{
		for (i = iNumElementsToDelete-1; i >= 0; i--)
		{
			iIndex = paiElementsToDelete[i];
			RemoveAt(iIndex, FALSE);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::RemoveTail(void)
{
	RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Finalise(void)
{
	miChunkSize = 1;
	SetArraySize(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Kill(void)
{
	if (mpvArray != NULL)
	{
		Free(mpvArray);
		miNumElements = 0;
		miUsedElements = 0;
	}
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Unuse(void)
{
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Zero(void)
{
	if (miUsedElements != 0)
	{
		memset_fast(mpvArray, 0, miUsedElements * miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::Copy(__CArrayTemplate* pcTemplateArray)
{
	BOOL	bResult;

	//Assumes the array is initialised.  
	//Returns whether or not it had to be resized.
	bResult = FALSE;
	if ((pcTemplateArray->miNumElements != miNumElements) || (pcTemplateArray->miChunkSize != miChunkSize))
	{
		bResult = TRUE;
		miChunkSize = pcTemplateArray->miChunkSize;
		SetArraySize(pcTemplateArray->miNumElements);
	}
	miUsedElements = pcTemplateArray->miUsedElements;
	CopyArrayInto(pcTemplateArray, 0);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::PushCopy(void)
{
	void*	pDest;
	void*	pSource;

	pDest = Add();
	if (miUsedElements == 1)
	{
		memset(pDest, 0, miElementSize);
	}
	else
	{
		pSource = Get(miUsedElements-1);
		memcpy(pDest, pSource, miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Push(M* pvElement)
{
	M* pv;
	pv = Add();
	memcpy(pv, pvElement, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Pop(M* pvElement)
{
	void*	pvSource;

	pvSource = Get(miUsedElements-1);
	memcpy(pvElement, pvSource, miElementSize);
	RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Pop(void)
{
	RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::Tail(void)
{
	if (miUsedElements == 0)
	{
		return NULL;
	}
	return Get(miUsedElements-1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::ByteSize()
{
	return miUsedElements * miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::GrowByChunk(void)
{
	miUsedElements += miChunkSize;
	SetArraySize(miNumElements + miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::AddIfUnique(M* pData)
{
	int iElementNum;

	iElementNum = Find(pData);
	if (iElementNum == -1)
	{
		Add(pData);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::Find(M* pData)
{
	int		i;
	M*		pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (memcmp(pIndex, pData, miElementSize) == 0)
		{
			return i;
		}
	}
	return -1;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::Contains(M* pData)
{
	return Find(pData) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize)
{
	int	iElementNum;

	iElementNum = FindWithKey(pData, iKeyOffset, iKeySize);
	if (iElementNum == -1)
	{
		Add(pData);
		iElementNum = miUsedElements - 1;
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::FindWithKey(M* pData, int iKeyOffset, int iKeySize)
{
	int		i;
	void*	pIndex;
	void*	pTest;

	pIndex = RemapSinglePointer(mpvArray, iKeyOffset);
	pTest = RemapSinglePointer(pData, iKeyOffset);

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp(pIndex, pTest, iKeySize) == 0)
		{
			return i;
		}
		pIndex = RemapSinglePointer(pIndex, miElementSize);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int	__CArrayTemplate<M>::FindWithIntKey(int iKey)
{
	int		i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int	__CArrayTemplate<M>::FindWithIntKey(int iKey, int iKeyOffset)
{
	int		i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = RemapSinglePointer(mpvArray, i * miElementSize + iKeyOffset);
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::GrowToNumElements(int iNumElements)
{
	int	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetUsedElements(iNumElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::GrowToAtLeastNumElements(int iNumElements, BOOL bClear, int iClear)
{
	int	iOldUsedElements;
	M*	pvStart;

	if (iNumElements == 0)
	{
		return NULL;
	}

	if (miUsedElements < iNumElements)
	{
		iOldUsedElements = GrowToNumElements(iNumElements);
		if (bClear)
		{
			if (iOldUsedElements < iNumElements)
			{
				pvStart = Get(iOldUsedElements);
				memset_fast(pvStart, iClear, (iNumElements - iOldUsedElements) * miElementSize);
			}
		}
		return Get(iNumElements-1);
	}
	else
	{
		return Get(iNumElements-1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::GrowByNumElements(int iNumElements)
{
	int	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetUsedElements(iNumElements + miUsedElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::SetUsedElements(int iUsedElements)
{
	int	iNumAllocations;
	int iNumLeftOvers;

	miUsedElements = iUsedElements;

	//Find how many allocation chunks are needed.
	iNumAllocations = iUsedElements / miChunkSize;
	iNumLeftOvers = iUsedElements % miChunkSize;

	//If there are any additional elements then allocate a new chunk for them.
	if (iNumLeftOvers > 0)
	{
		iNumAllocations++;
	}

	//Num elements is the number of elements which memory has been allocated for, not the number of used elements.
	SetArraySize(iNumAllocations * miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::Equals(__CArrayTemplate<M>* pcTemplateArray)
{
	if (pcTemplateArray->miUsedElements == miUsedElements)
	{
		if (memcmp(pcTemplateArray->mpvArray, mpvArray, ByteSize()) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Swap(int iIndex1, int iIndex2)
{
	M*	pvTemp;
	M*	pElement1;
	M*	pElement2;

	//Only allocate memory if we don't already have temporary memory assigned.
	if (miUsedElements < miNumElements)
	{
		pvTemp = Get(miUsedElements);
	}
	else
	{
		pvTemp = (M*)Malloc(miElementSize);
	}

	pElement1 = Get(iIndex1);
	pElement2 = Get(iIndex2);
	memcpy(pvTemp, pElement1, miElementSize);
	memcpy(pElement1, pElement2, miElementSize);
	memcpy(pElement2, pvTemp, miElementSize);

	//Free any memory we had to allocate.
	if (miUsedElements == miNumElements)
	{
		Free(pvTemp);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::QuickSort(int(* Func)(const void*, const void*))
{
	qsort((void*)mpvArray, miUsedElements, miElementSize, Func);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::BubbleSort(int(* Func)(const void*, const void*))
{
	int		iIndex;
	int		iResult;
	BOOL	bSwapped;
	int		iNumDone;

	bSwapped = TRUE;
	iNumDone = 0;
	while (bSwapped)
	{
		iIndex = 0;
		bSwapped = FALSE;

		while ((iIndex+1) < (miUsedElements - iNumDone))
		{
			iResult = Func(Get(iIndex), Get(iIndex+1));
			if (iResult > 0)
			{
				Swap(iIndex, iIndex+1);
				bSwapped = TRUE;
			}
			iIndex++;
		}
		iNumDone++;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::InsertIntoSorted(int(* Func)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting)
{
	int		iIndex;
	BOOL	bExists;

	bExists = FindInSorted(pvElement, Func, &iIndex);
	if (iIndex < miUsedElements)
	{
		if (!bExists)
		{
			InsertAt(pvElement, iIndex);
		}
		else
		{
			if (bOverwriteExisting)
			{
				Set(iIndex, pvElement);
			}
			else
			{
				InsertAt(pvElement, iIndex);
			}
		}
	}
	else
	{
		Add(pvElement);
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::FindInSorted(M* pData, int(* Func)(const void*, const void*), int* piIndex)
{
	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return FALSE;
	}
	return BinarySearch(pData, 0, miUsedElements - 1, Func, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::BinarySearch(M* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;

	iResultMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		iResultMiddle = Func(pData, Get(iMiddle));
		if (iResultMiddle == 0)
		{
			*piIndex = iMiddle;
			return TRUE;
		}
		else if (iResultMiddle < 0)
		{
			iRight = iMiddle - 1;
		}
		else
		{
			iLeft = iMiddle + 1;
		}
	}

	if (iResultMiddle > 0)
	{
		*piIndex = (iMiddle + 1);
	}
	else
	{
		*piIndex = iMiddle;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::Reverse(void)
{
	int	i;
	int	iCount;

	iCount = miUsedElements/2;
	for (i = 0; i < iCount; i++)
	{
		Swap(i, miUsedElements-i-1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::CopyArrayInto(__CArrayTemplate<M>* pcTemplateArray, int iIndex)
{
	memcpy(Get(iIndex), pcTemplateArray->mpvArray, miElementSize * pcTemplateArray->miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::CopyBlockInto(M* paElements, int iLength, int iIndex)
{
	memcpy(Get(iIndex), paElements, miElementSize * iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertArrayAfterEnd(__CArrayTemplate<M>* pcTemplateArray)
{
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(pcTemplateArray->miUsedElements);
	CopyArrayInto(pcTemplateArray, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertArrayBeforeStart(__CArrayTemplate<M>* pcTemplateArray)
{
	InsertNumElementsAt(pcTemplateArray->miUsedElements, 0);
	CopyArrayInto(pcTemplateArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertArrayAt(__CArrayTemplate<M>* pcTemplateArray, int iIndex)
{
	InsertNumElementsAt(pcTemplateArray->miUsedElements, iIndex);
	CopyArrayInto(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::InsertNumElementsAt(int iNumElements, int iIndex)
{
	void*	pvFrom;
	void*	pvTo;
	int		iNumToMove;

	iNumToMove = miUsedElements - iIndex;
	GrowByNumElements(iNumElements);

	pvFrom = (void*)Get(iIndex);
	pvTo = (void*)Get(iIndex + iNumElements);
	memmove(pvTo, pvFrom, iNumToMove * miElementSize);
	return (M*)pvFrom;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertBlockAfterEnd(M* paElements, int iLength)
{
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(iLength);
	CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertBlockBeforeStart(M* paElements, int iLength)
{
	InsertNumElementsAt(iLength, 0);
	CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::InsertBlockAt(M* paElements, int iLength, int iIndex)
{
	InsertNumElementsAt(iLength, iIndex);
	CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::Get(int iIndex)
{
	return (M*)RemapSinglePointer(mpvArray, miElementSize * iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::ElementSize(void)
{
	return miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::AllocatedElements(void)
{
	return miNumElements;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::IsEmpty(void)
{
	return miUsedElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::IsNotEmpty(void)
{
	return miUsedElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::GetAdjustedIndex(int iIndex)
{
	if (iIndex == -1)
	{
		return NumElements()-1;
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::GetIndex(M* pvElement)
{
	int iIndex;
	int iBase;
	int iDifference;

	iBase = (int)(ENGINE_SIZE_T) mpvArray;
	iIndex = (int)(ENGINE_SIZE_T) pvElement;
	iDifference = iIndex - iBase;

	//Make sure the element is correctly aligned.
	if (iDifference % miElementSize != 0)
	{
		return -1;
	}

	//Make sure the element lies within the array.
	iIndex = iDifference / miElementSize;
	if ((iIndex < 0) || (iIndex >= miUsedElements))
	{
		return -1;
	}

	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::BatchRemoveElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	M*		pcFirst;
	int		iTotalStride;
	M*		pcSource;
	M*		pcDest;
	int		iDestIndex;
	int		iSourceIndex;
	int		iRemaining;

	iTotalStride = iStrideToNextBatch;
	iStrideToNextBatch = iStrideToNextBatch - iNumInBatch;
	pcFirst = Get(iFirstIndex);

	for (i = 0; i <= iNumBatches-2; i++)
	{
		memcpy(RemapSinglePointer(pcFirst, iStrideToNextBatch * i), RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), iStrideToNextBatch);
	}

	i = iNumBatches-1;

	iDestIndex = iTotalStride * (i+1) - iStrideToNextBatch;
	iSourceIndex = iStrideToNextBatch * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDestIndex);
	pcSource = (M*)RemapSinglePointer(pcFirst, iSourceIndex);

	iRemaining = (miUsedElements - iDestIndex) - iFirstIndex;

	if (iRemaining > 0)
	{
		memcpy(pcSource, pcDest, iRemaining);
	}

	GrowToNumElements(miUsedElements - iNumInBatch * iNumBatches);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::BatchInsertElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	M*		pcFirst;
	int		iTotalStride;
	M*		pcSource;
	M*		pcDest;
	int		iDestIndex;
	int		iSourceIndex;
	int		iOldNumElements;
	int		iRemaining;

	iOldNumElements = GrowByNumElements(iNumInBatch * iNumBatches);

	iTotalStride = iNumInBatch + iStrideToNextBatch;
	pcFirst = Get(iFirstIndex);

	i = iNumBatches-1;

	iDestIndex = iTotalStride * (i+1) - iStrideToNextBatch;
	iSourceIndex = iStrideToNextBatch * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDestIndex);
	pcSource = (M*)RemapSinglePointer(pcFirst, iSourceIndex);

	iRemaining = (iOldNumElements - iSourceIndex) - iFirstIndex;

	if (iRemaining > 0)
	{
		memcpy(pcDest, pcSource, iRemaining);
	}

	if (iStrideToNextBatch == 1)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_1byte(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i));
		}
	}
	else if (iStrideToNextBatch == 2)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_2bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i));
		}
	}
	else if (iStrideToNextBatch == 4)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_4bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i));
		}
	}
	else if (iStrideToNextBatch == 8)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_8bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i));
		}
	}
	else if (iStrideToNextBatch == 12)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_12bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i));
		}
	}
	else
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), RemapSinglePointer(pcFirst, iStrideToNextBatch * i), iStrideToNextBatch);
		}
	}	
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayTemplate<M>::GetHeader(SArrayTemplateHeader* psHeader)
{
	psHeader->miElementSize = miElementSize;
	psHeader->miUsedElements = miUsedElements;
	psHeader->miChunkSize = miChunkSize;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayTemplate<M>::GetData(void)
{
	return mpvArray;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayTemplate<M>::ChunkSize(void)
{
	return miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::WriteArrayTemplate(CFileWriter* pcFileWriter)
{
	if (!WriteArrayTemplateHeader(pcFileWriter))
	{
		return FALSE;
	}

	if (NumElements() != 0)
	{
		if (!pcFileWriter->WriteData(GetData(), ByteSize()))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::WriteArrayTemplateHeader(CFileWriter* pcFileWriter)
{
	SArrayTemplateHeader	sHeader;

	GetHeader(&sHeader);
	if (!pcFileWriter->WriteData(&sHeader, sizeof(SArrayTemplateHeader)))
	{ 
		return FALSE; 
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::ReadArrayTemplate(CFileReader* pcFileReader)
{
	if (!ReadArrayTemplateHeader(pcFileReader))
	{
		return FALSE;
	}

	if (NumElements() != 0)
	{
		if (!pcFileReader->ReadData(GetData(), ByteSize()))
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayTemplate<M>::ReadArrayTemplateHeader(CFileReader* pcFileReader)
{
	SArrayTemplateHeader	sHeader;

	if (!pcFileReader->ReadData(&sHeader, sizeof(SArrayTemplateHeader)))
	{ 
		return FALSE; 
	}

	if (sHeader.miElementSize != sizeof(M))
	{
		return FALSE;
	}

	Init(&sHeader);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(void)
{
	this->miElementSize = sizeof(M);
	this->miNumElements = 0;
	this->miUsedElements = 0;
	this->miChunkSize = 1;
	this->mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(SArrayTemplateHeader* psHeader)
{
	__CArrayTemplate<M>::Init(psHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(int iChunkSize)
{
	Init();
	this->miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(__CArrayTemplate<M>* pcTemplateArray)
{
	__CArrayTemplate<M>::Init(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Allocate(int iNumElements)
{
	Init(iNumElements);
	this->GrowByChunk();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Allocate(int iChunkSize, int iNumElements)
{
	this->miElementSize = sizeof(M);
	this->miChunkSize = iChunkSize;
	this->mpvArray = NULL;
	this->SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::ReInit(int iChunkSize)
{
	this->Kill();
	if (iChunkSize == 0)
	{
		Init(miChunkSize);
	}
	else
	{
		Init(iChunkSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Fake(M* pvData, int iNum)
{
	this->mpvArray = pvData;
	this->miElementSize = sizeof(M);
	this->miNumElements = iNum;
	this->miUsedElements = iNum;
	this->miChunkSize = 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::FakeSetUsedElements(int iUsedElements)
{
	this->miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M& CArrayTemplate<M>::operator[](int iIndex)
{
	return this->mpvArray[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Get(int iIndex)
{
	return &this->mpvArray[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::SetAllocateSize(int iSize)
{
	miChunkSize = iSize;
}


#endif //__TEMPLATE_ARRAY_H__

