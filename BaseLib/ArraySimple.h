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
#ifndef __ARRAY_SIMPLE_H__
#define __ARRAY_SIMPLE_H__
#include "Define.h"
#include "PointerRemapper.h"
#include "ErrorHandler.h"


template <class M>
class CArraySimple
{
public:
	int		miUsedElements;
	M*		pvArray;

	void	Init(void);
	void	Init(int iIgnored);
	void 	ReInit(void);
	void	Kill(void);
	void	Allocate(int iNum);
	M*		Add(void);
	void	Add(M* pvData);
	void	Set(int iElementPos, M* pvData);
	M*		InsertAt(int iElementPos);
	void	InsertAt(M* pvData, int iElementPos);
	void	BatchInsertElements(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);
	M*		SafeGet(int iElementPos);
	M*		Get(int iElementPos);
	void	RemoveAt(int iElementPos, int bPreserveOrder = 0);
	void	RemoveTail(void);
	void	BatchRemoveElements(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);
	int		ByteSize(void);
	void	InitFromHeader(void);
	BOOL	SetArraySize(int iNum);
	M*		SetArraySize(int iNum, int iClearValue);
	int		NumElements(void);
	void 	Copy(CArraySimple<M>* pArray);
	void 	Zero(void);
	void	Zero(int iStart, int iEnd);
	void	QuickSort(int(* Func)(const void*, const void*));
	void	InsertIntoSorted(int(* Func)(const void*, const void*), M* pvElement);
	int 	Find(M* pData);
	int 	FindWithKey(M* pData, int iKeyOffset, int iKeySize);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	int		FindInSorted(M* pData, int(* Func)(const void*, const void*), int bFindIfNone);
	int		BinarySearch(M* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int bFindIfNone);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);
	int		GetAdjustedIndex(int iIndex);
	int		GetIndex(M* pvElement);
	int		GrowToNumElements(int iNumElements);
	int		GrowByNumElements(int iNumElements);
	M*		InsertNumElementsAt(int iNumElements, int iElementPos);
	M*		Tail(void);
	M*		GetData(void);
	void	FakeSetUsedElements(int iUsedElements);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Init(void)
{
	miUsedElements = 0;
	pvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Init(int iIgnored)
{
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Kill(void)
{
	free(pvArray);
	pvArray = NULL;
	miUsedElements = 0;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Allocate(int iNum)
{
	Init();
	SetArraySize(iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::Add(void)
{
	miUsedElements++;
	pvArray = (M*)realloc(pvArray, miUsedElements * sizeof(M));
	return &pvArray[miUsedElements-1];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Add(M* pvData)
{
	M* pvTemp;

	pvTemp = Add();
	memcpy(pvTemp, pvData, sizeof(M));
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::SafeGet(int iElementPos)
{
	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return NULL;
	}
	else
	{
		return &pvArray[iElementPos];
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::Get(int iElementPos)
{
	return &pvArray[iElementPos];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::RemoveAt(int iElementPos, int bPreserveOrder)
{
	M*	pvEnd;
	M*	pvElement;

	miUsedElements--;
	if (iElementPos < miUsedElements)
	{
		pvElement = Get(iElementPos);
		if (!bPreserveOrder)
		{
			pvEnd = Get(miUsedElements);
			memcpy(pvElement, pvEnd, sizeof(M));
		}
		else
		{
			memmove(pvElement, (M*)((ENGINE_SIZE_T) pvElement + sizeof(M)), sizeof(M) * (miUsedElements - iElementPos));
		}
	}
	pvArray = (M*)realloc(pvArray, miUsedElements * sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::RemoveTail(void)
{
	RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::ByteSize(void)
{
	return miUsedElements * sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::InitFromHeader(void)
{
	//This function assumes that the value of pvArray was invalid.
	pvArray = (M*)malloc(miUsedElements * sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArraySimple<M>::SetArraySize(int iNum)
{
	if (miUsedElements != iNum)
	{
		miUsedElements = iNum;
		pvArray = (M*)realloc(pvArray, miUsedElements * sizeof(M));
		if ((miUsedElements > 0) && (pvArray == NULL))
		{
			gcUserError.Set("CArraySimple, Out of memory.");
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
M* CArraySimple<M>::SetArraySize(int iNum, int iClearValue)
{
	int		iOldUsed;
	void*	pvClearStart;
	int		iClearSize;

	if (miUsedElements != iNum)
	{
		iOldUsed = -1;
		if (iNum > miUsedElements)
		{
			iOldUsed = miUsedElements;
		}
		miUsedElements = iNum;
		pvArray = (M*)realloc(pvArray, miUsedElements * sizeof(M));

		if (iOldUsed != -1)
		{
			pvClearStart = (void*)((ENGINE_SIZE_T) ((int)((ENGINE_SIZE_T) pvArray) + (iOldUsed * sizeof(M))));
			iClearSize = (miUsedElements - iOldUsed) * sizeof(M);
			memset(pvClearStart, iClearValue, iClearSize);
			return (M*)pvClearStart;
		}
	}

	//No new elements were added.
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Copy(CArraySimple* pArray)
{
	Kill();
	SetArraySize(pArray->NumElements());
	memcpy(pvArray, pArray->pvArray, miUsedElements * sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Zero(void)
{
	if (miUsedElements != 0)
	{
		memset(pvArray, 0, miUsedElements * sizeof(M));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Zero(int iStart, int iEnd)
{
	if ((iStart > 0) && (iEnd <= miUsedElements))
	{
		memset((void*)Get(iStart), 0, (iEnd-iStart) * sizeof(M));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::QuickSort(int(* Func)(const void*, const void*))
{
	qsort((void*)pvArray, miUsedElements, sizeof(M), Func);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::InsertIntoSorted(int(* Func)(const void*, const void*), M* pvElement)
{
	int	iPos;

	iPos = FindInSorted(pvElement, Func, 1);
	if (iPos < miUsedElements)
	{
		InsertAt(pvElement, iPos);
	}
	else
	{
		Add(pvElement);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::FindInSorted(M* pData, int(* Func)(const void*, const void*), int bFindIfNone)
{
	if (miUsedElements == 0)
	{
		if (bFindIfNone)
		{
			return 0;
		}
		return -1;
	}
	return BinarySearch(pData, 0, miUsedElements - 1, Func, bFindIfNone);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::BinarySearch(M* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int bFindIfNone)
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
			return iMiddle;
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

	if (bFindIfNone)
	{
		if (iResultMiddle > 0)
		{
			return iMiddle + 1;
		}
		return iMiddle;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::InsertAt(int iElementPos)
{
	M* ptr;

	//Grow the array.
	Add();

	//This assumes that iElementPos is within the array (or the last element).
	ptr = (M*)((ENGINE_SIZE_T) pvArray + iElementPos * sizeof(M));
	memmove((M*)((ENGINE_SIZE_T) ptr + sizeof(M)), ptr, sizeof(M) * (miUsedElements - 1 - iElementPos));
	return ptr;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::InsertAt(M* pvData, int iElementPos)
{
	M* ptr;

	ptr = InsertAt(iElementPos);
	memcpy(ptr, pvData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::Find(M* pData)
{
	int		i;
	M*	pPos;

	pPos = pvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp(pPos, pData, sizeof(M)) == 0)
		{
			return i;
		}
		pPos = (M*)((ENGINE_SIZE_T) pPos + sizeof(M));
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::FindWithKey(M* pData, int iKeyOffset, int iKeySize)
{
	int		i;
	M*	pPos;

	pPos = pvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp((M*)((ENGINE_SIZE_T) pPos + iKeyOffset), (M*)((ENGINE_SIZE_T) pData + iKeyOffset), iKeySize) == 0)
		{
			return i;
		}
		pPos = (M*)((ENGINE_SIZE_T) pPos + sizeof(M));
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int	CArraySimple<M>::FindWithIntKey(int iKey, int iKeyOffset)
{
	int		i;
	void*	pPos;

	pPos = (void*)((ENGINE_SIZE_T) ((int)((ENGINE_SIZE_T) pvArray) + iKeyOffset));
	for (i = 0; i < miUsedElements; i++)
	{
		if (*((int*)pPos) == iKey)
		{
			return i;
		}
		pPos = RemapSinglePointer(pPos, sizeof(M));
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::AddIfUnique(M* pData)
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
int CArraySimple<M>::AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize)
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
int CArraySimple<M>::GetAdjustedIndex(int iIndex)
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
int CArraySimple<M>::GetIndex(M* pvElement)
{
	int iPosition;
	int iBase;
	int iDifference;

	iBase = (int)(ENGINE_SIZE_T) pvArray;
	iPosition = (int)(ENGINE_SIZE_T) pvElement;
	iDifference = iPosition - iBase;

	//Make sure the element is correctly aligned.
	if (iDifference % sizeof(M) != 0)
	{
		return -1;
	}

	//Make sure the element lies within the array.
	iPosition = iDifference / sizeof(M);
	if ((iPosition < 0) || (iPosition >= miUsedElements))
	{
		return -1;
	}

	return iPosition;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::Set(int iElementPos, M* pvData)
{
	M*	pvTemp;

	pvTemp = Get(iElementPos);
	memcpy((M*)((ENGINE_SIZE_T) pvArray + (iElementPos) * sizeof(M)), pvData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::GrowToNumElements(int iNumElements)
{
	int	iOldUsedElments;

	iOldUsedElments = miUsedElements;
	SetArraySize(iNumElements);
	return iOldUsedElments;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArraySimple<M>::GrowByNumElements(int iNumElements)
{
	int	iOldUsedElments;

	iOldUsedElments = miUsedElements;
	SetArraySize(miUsedElements + iNumElements);
	return iOldUsedElments;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::InsertNumElementsAt(int iNumElements, int iElementPos)
{
	M*		pvNew;
	M*		pvFrom;
	M*		pvTo;
	int		iNumToMove;

	if (iNumElements <= 0)
	{
		return NULL;
	}

	pvNew = (M*)malloc((miUsedElements + iNumElements) * sizeof(M));

	if (iElementPos > 0)
	{
		memcpy(pvNew, pvArray, iElementPos * sizeof(M));
	}

	iNumToMove = miUsedElements - iElementPos;
	pvFrom = Get(iElementPos);
	pvTo = &pvNew[iElementPos + iNumElements];
	memcpy(pvTo, pvFrom, iNumToMove * sizeof(M));

	free(pvArray);
	pvArray = pvNew;

	return (M*)pvFrom;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArraySimple<M>::Tail(void)
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
M* CArraySimple<M>::GetData(void)
{
	return pvArray;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::FakeSetUsedElements(int iUsedElements)
{
	this->miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArraySimple<M>::BatchRemoveElements(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	M*		pcFirst;
	int		iTotalStride;
	M*		pcSource;
	M*		pcDest;
	int		iDestPos;
	int		iSourcePos;
	int		iRemaining;

	iTotalStride = iStrideToNextBatch;
	iStrideToNextBatch = iStrideToNextBatch - iNumInBatch;
	pcFirst = Get(iFirstElementPos);

	for (i = 0; i <= iNumBatches-2; i++)
	{
		memcpy(RemapSinglePointer(pcFirst, iStrideToNextBatch * i), RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStrideToNextBatch), iStrideToNextBatch);
	}

	i = iNumBatches-1;

	iDestPos = iTotalStride * (i+1) - iStrideToNextBatch;
	iSourcePos = iStrideToNextBatch * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDestPos);
	pcSource = (M*)RemapSinglePointer(pcFirst, iSourcePos);

	iRemaining = (miUsedElements - iDestPos) - iFirstElementPos;

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
void CArraySimple<M>::BatchInsertElements(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	M*		pcFirst;
	int		iTotalStride;
	M*		pcSource;
	M*		pcDest;
	int		iDestPos;
	int		iSourcePos;
	int		iOldNumElements;
	int		iRemaining;

	iOldNumElements = GrowByNumElements(iNumInBatch * iNumBatches);

	iTotalStride = iNumInBatch + iStrideToNextBatch;
	pcFirst = Get(iFirstElementPos);

	i = iNumBatches-1;

	iDestPos = iTotalStride * (i+1) - iStrideToNextBatch;
	iSourcePos = iStrideToNextBatch * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDestPos);
	pcSource = (M*)RemapSinglePointer(pcFirst, iSourcePos);

	iRemaining = (iOldNumElements - iSourcePos) - iFirstElementPos;

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


#endif //__ARRAY_SIMPLE_H__

