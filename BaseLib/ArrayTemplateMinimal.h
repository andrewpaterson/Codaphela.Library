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
#ifndef __ARRAY_TEMPLATE_MINIMAL_H__
#define __ARRAY_TEMPLATE_MINIMAL_H__
#include "ConstructorCall.h"
#include "ArrayBlockMinimal.h"


template <class M>
class CArrayTemplateMinimal : public CArrayBlockMinimal, protected CPostMalloc<M>
{
protected:
	M*				mpvArray;

public:
	void	Init(void);
	void	Init(CMallocator* pcMallocator);
	void	Init(int iIgnored);
	void	Init(CMallocator* pcMallocator, int iIgnored);
	void 	Init(CArrayTemplateMinimal<M>* pArray);  //Used to be Copy
	void 	Init(CMallocator* pcMallocator, CArrayTemplateMinimal<M>* pArray);  //Used to be Copy
	void 	ReInit(void);
	void	Kill(void);
	void	Allocate(int iNum);
	void	Allocate(CMallocator* pcMallocator, int iNum);

	int		Resize(int iNumElements);  //Test Virtual and make sure only PostMalloc only called once.
	int		NumElements(void);

	M*		Add(void);
	M*		Add(M* pvData);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);
	M* 		AddGetIndex(int* piIndex);
	int		AddNum(int iNumElements);

	M*		Get(int iElementPos);
	M*		SafeGet(int iElementPos);
	M*		Tail(void);
	M*		GetData(void);
	int		GetAdjustedIndex(int iIndex);
	int		GetIndex(M* pvElement);

	M*		InsertAt(int iElementPos);
	M*		InsertAt(M* pvData, int iElementPos);
	M*		InsertNumAt(int iNumElements, int iIndex);
	int		InsertIntoSorted(int(*fCompare)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting);
	void	InsertBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip);  //Test Virtual

	M*		Push(void);
	M*		PushCopy(void);

	void	QuickSort(int(*fCompare)(const void*, const void*));

	int 	Find(M* pData);
	int 	FindWithKey(M* pData, int iKeyOffset, int iKeySize);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	BOOL	FindInSorted(M* pData, int(*)(const void*, const void*), int* piIndex);

	BOOL	RemoveAt(int iElementPos, int bPreserveOrder = 0);
	BOOL	RemoveTail(void);
	void	RemoveBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip);

	void	Set(int iElementPos, M* pvData);

	int		ByteSize(void);
	BOOL	SetUsedElements(int iUsedElements);
	void	FakeSetUsedElements(int iUsedElements);

	void 	Zero(void);
	void	Zero(int iStart, int iEnd);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);

	BOOL	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	BOOL	WriteHeader(CFileWriter* pcFileWriter);
	BOOL	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	BOOL	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);

	BOOL	SetArraySize(int iNum);
	M*		SetArraySize(int iNum, int iClearValue);

protected:
	BOOL	BinarySearch(M* pData, int iLeft, int iRight, int(*Func)(const void*, const void*), int* piIndex);

	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(void)
{
	CArrayBlockMinimal::Init();
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMallocator)
{
	CArrayBlockMinimal::Init(pcMallocator);
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(int iIgnored)
{
	CArrayBlockMinimal::Init(iIgnored);
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMallocator, int iIgnored)
{
	CArrayBlockMinimal::Init(pcMallocator, iIgnored);
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::ReInit(void)
{
	CMallocator*	pcMalloc;

	pcMalloc = mpcMalloc;
	Kill();

	Init(pcMalloc);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Kill(void)
{
	Free(mpvArray);
	mpvArray = NULL;
	CArrayBlockMinimal::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Allocate(int iNum)
{
	CArrayBlockMinimal::Init();
	mpvArray = NULL;
	SetArraySize(iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Allocate(CMallocator* pcMallocator, int iNum)
{
	CArrayBlockMinimal::Init(pcMallocator);
	mpvArray = NULL;
	SetArraySize(iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CArrayTemplateMinimal* pvArray)
{
	CArrayBlockMinimal::Init();
	mpvArray = NULL;
	Resize(pvArray->NumElements());
	memcpy(mpvArray, pvArray->mpvArray, miUsedElements * sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMalloc, CArrayTemplateMinimal* pArray)
{
	CArrayBlockMinimal::Init(pcMalloc);
	mpvArray = NULL;
	Resize(pArray->NumElements());
	memcpy(mpvArray, pArray->mpvArray, miUsedElements * sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::SetUsedElements(int iUsedElements)
{
	M*	pvTemp;

	if (iUsedElements == 0)
	{
		Free(mpvArray);
		mpvArray = NULL;

		miUsedElements = 0;
		return TRUE;
	}

	pvTemp = (M*)Realloc(mpvArray, iUsedElements * sizeof(M));

	if (pvTemp != NULL)
	{
		miUsedElements = iUsedElements;
		mpvArray = pvTemp;
		return TRUE;
	}
	else
	{
		gcUserError.Set("CArrayTemplateMinimal, Out of memory.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::Add(void)
{
	if (SetUsedElements(miUsedElements+1))
	{
		return this->PostMalloc(&mpvArray[miUsedElements-1]);
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
M* CArrayTemplateMinimal<M>::Add(M* pvData)
{
	M* pvTemp;

	pvTemp = Add();
	memcpy(pvTemp, pvData, sizeof(M));
	return pvTemp;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::AddGetIndex(int* piIndex)
{
	M* pv;

	pv = Add();
	(*piIndex) = miUsedElements - 1;
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::SafeGet(int iElementPos)
{
	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return NULL;
	}
	else
	{
		return &mpvArray[iElementPos];
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::Get(int iElementPos)
{
	return &mpvArray[iElementPos];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::RemoveAt(int iElementPos, int bPreserveOrder)
{
	M*	pvEnd;
	M*	pvElement;
	int	iUsedElements;

	iUsedElements = miUsedElements - 1;
	if (iElementPos < iUsedElements)
	{
		pvElement = Get(iElementPos);
		if (!bPreserveOrder)
		{
			pvEnd = Get(iUsedElements);
			memcpy(pvElement, pvEnd, sizeof(M));
		}
		else
		{
			memmove(pvElement, (M*)((size_t) pvElement + sizeof(M)), sizeof(M) * (iUsedElements - iElementPos));
		}
	}

	return SetUsedElements(iUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::RemoveTail(void)
{
	return RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplateMinimal<M>::ByteSize(void)
{
	return miUsedElements * sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::SetArraySize(int iNum)
{
	if (miUsedElements != iNum)
	{
		return SetUsedElements(iNum);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::SetArraySize(int iNum, int iClearValue)
{
	int		iOldUsed;
	void*	pvClearStart;
	int		iClearSize;
	BOOL	bResult;

	if (miUsedElements != iNum)
	{
		iOldUsed = -1;
		if (iNum > miUsedElements)
		{
			iOldUsed = miUsedElements;
		}
		bResult = SetUsedElements(iNum);

		if (!bResult)
		{
			return NULL;
		}

		if (iOldUsed != -1)
		{
			pvClearStart = (void*)((size_t) ((int)((size_t) mpvArray) + (iOldUsed * sizeof(M))));
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
int CArrayTemplateMinimal<M>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Zero(void)
{
	if (miUsedElements != 0)
	{
		memset(mpvArray, 0, miUsedElements * sizeof(M));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Zero(int iStart, int iEnd)
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
void CArrayTemplateMinimal<M>::QuickSort(int(*fCompare)(const void*, const void*))
{
	qsort((void*)mpvArray, miUsedElements, sizeof(M), fCompare);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplateMinimal<M>::InsertIntoSorted(int(*fCompare)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting)
{
	int		iIndex;
	BOOL	bExists;

	bExists = FindInSorted(pvElement, fCompare, &iIndex);
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
BOOL CArrayTemplateMinimal<M>::FindInSorted(M* pData, int(* Func)(const void*, const void*), int* piIndex)
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
BOOL CArrayTemplateMinimal<M>::BinarySearch(M* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;
	M*		pvMiddle;

	iResultMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = Get(iMiddle);
		iResultMiddle = Func(pData, pvMiddle);
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
M* CArrayTemplateMinimal<M>::InsertAt(int iElementPos)
{
	M* pv;

	//Grow the array.
	Add();

	//This assumes that iElementPos is within the array (or the last element).
	pv = (M*)((size_t) mpvArray + iElementPos * sizeof(M));
	memmove((M*)((size_t)pv + sizeof(M)), pv, sizeof(M) * (miUsedElements - 1 - iElementPos));
	return this->PostMalloc(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::InsertAt(M* pvData, int iElementPos)
{
	M* pv;

	pv = InsertAt(iElementPos);
	memcpy(pv, pvData, sizeof(M));
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplateMinimal<M>::Find(M* pData)
{
	int		i;
	M*	pPos;

	pPos = mpvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp(pPos, pData, sizeof(M)) == 0)
		{
			return i;
		}
		pPos = (M*)((size_t) pPos + sizeof(M));
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplateMinimal<M>::FindWithKey(M* pData, int iKeyOffset, int iKeySize)
{
	int		i;
	M*	pPos;

	pPos = mpvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp((M*)((size_t) pPos + iKeyOffset), (M*)((size_t) pData + iKeyOffset), iKeySize) == 0)
		{
			return i;
		}
		pPos = (M*)((size_t) pPos + sizeof(M));
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int	CArrayTemplateMinimal<M>::FindWithIntKey(int iKey, int iKeyOffset)
{
	int		i;
	void*	pPos;

	pPos = (void*)((size_t) ((int)((size_t) mpvArray) + iKeyOffset));
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
int CArrayTemplateMinimal<M>::AddIfUnique(M* pData)
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
int CArrayTemplateMinimal<M>::AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize)
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
int CArrayTemplateMinimal<M>::GetAdjustedIndex(int iIndex)
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
int CArrayTemplateMinimal<M>::GetIndex(M* pvElement)
{
	int iPosition;
	int iBase;
	int iDifference;

	iBase = (int)(size_t) mpvArray;
	iPosition = (int)(size_t) pvElement;
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
void CArrayTemplateMinimal<M>::Set(int iElementPos, M* pvData)
{
	memcpy((M*)((size_t) mpvArray + (iElementPos) * sizeof(M)), pvData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplateMinimal<M>::Resize(int iNumElements)
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
int CArrayTemplateMinimal<M>::AddNum(int iNumElements)
{
	int		iOldUsedElements;
	M*		pvFrom;
	BOOL	bResult;

	if (iNumElements < 1)
	{
		return -1;
	}

	iOldUsedElements = miUsedElements;
	bResult = SetArraySize(miUsedElements + iNumElements);
	if (!bResult)
	{
		return -1;
	}

	pvFrom = Get(iOldUsedElements);
	PostMalloc(pvFrom, iNumElements, sizeof(M));
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::InsertNumAt(int iNumElements, int iIndex)
{
	M*		pvFrom;
	M*		pvTo;
	int		iNumToMove;

	if (iNumElements <= 0)
	{
		return NULL;
	}

	iNumToMove = miUsedElements - iIndex;
	AddNum(iNumElements);

	pvFrom = Get(iIndex);
	pvTo = Get(iIndex + iNumElements);
	memmove(pvTo, pvFrom, iNumToMove * sizeof(M));

	return pvFrom;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::Tail(void)
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
M* CArrayTemplateMinimal<M>::GetData(void)
{
	return mpvArray;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::FakeSetUsedElements(int iUsedElements)
{
	this->miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::RemoveBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip)
{
	int		i;
	M*		pcFirst;
	M*		pcSource;
	M*		pcDest;
	int		iDest;
	int		iSource;
	int		iRemaining;
	int		iSkipStride;

	pcFirst = Get(iFirstElementPos);

	iSkipStride = iSkip * sizeof(M);
	for (i = 0; i <= iNumBatches-2; i++)
	{
		iDest = iFirstElementPos + iSkip * i;
		iSource = iFirstElementPos + (iSkip + iNumInBatch) * i + iNumInBatch;

		pcDest = Get(iDest);
		pcSource = Get(iSource);

		memcpy(pcDest, pcSource, iSkipStride);
	}

	iDest = iFirstElementPos + (iSkip + iNumInBatch) * iNumBatches  - iSkip;
	iRemaining = miUsedElements - iDest;

	if (iRemaining > 0)
	{
		iSource = iFirstElementPos + iSkip * (iNumBatches - 1);
		pcDest = Get(iDest);
		pcSource = Get(iSource);

		memcpy(pcSource, pcDest, iRemaining * sizeof(M));
		iRemaining = 0;
	}
	else
	{
		iRemaining = -iRemaining;
	}


	SetArraySize(miUsedElements - iNumInBatch * iNumBatches + iRemaining);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::InsertBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip)
{
	int		i;
	M*		pcFirst;
	int		iTotalStride;
	M*		pcSource;
	M*		pcDest;
	int		iDest;
	int		iSource;
	int		iOldNumElements;
	int		iRemaining;
	int		iStride;

	iOldNumElements = AddNum(iNumInBatch * iNumBatches);

	pcFirst = Get(iFirstElementPos);

	i = iNumBatches-1;

	iDest = (iNumInBatch + iSkip) * (i + 1) - iSkip;
	iSource = iSkip * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDest * sizeof(M));
	pcSource = (M*)RemapSinglePointer(pcFirst, iSource * sizeof(M));

	iRemaining = (iOldNumElements - iSource) - iFirstElementPos;

	if (iRemaining > 0)
	{
		memcpy(pcDest, pcSource, iRemaining * sizeof(M));
	}

	iStride = iSkip * sizeof(M);
	iTotalStride = (iNumInBatch + iSkip) * sizeof(M);
	if (iStride == 1)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_1byte(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 2)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_2bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 4)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_4bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 8)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_8bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 12)
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy_fast_12bytes(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else
	{
		for (i = iNumBatches-2; i >= 0; i--)
		{
			memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i+1) - iStride), RemapSinglePointer(pcFirst, iStride * i), iStride);
		}
	}	
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::PushCopy(void)
{
	M*	pDest;
	M*	pSource;

	if (miUsedElements >= 1)
	{
		pDest = Add();
		pSource = Get(miUsedElements - 2);
		memcpy(pDest, pSource, sizeof(M));
		return pDest;
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
void* CArrayTemplateMinimal<M>::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CArrayTemplateMinimal<M>::Realloc(void* pv, size_t tSize)
{
	return mpcMalloc->Realloc(pv, tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::WriteHeader(CFileWriter* pcFileWriter)
{
	return CArrayBlockMinimal::WriteHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	return CArrayBlockMinimal::WriteAllocatorAndHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplateMinimal<M>::Write(CFileWriter* pcFileWriter)
{
	if (!WriteAllocatorAndHeader(pcFileWriter))
	{
		return FALSE;
	}

	if (miUsedElements != 0)
	{
		if (!pcFileWriter->WriteData(mpvArray, ByteSize())) 
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
BOOL CArrayTemplateMinimal<M>::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	int		iUsedElements;

	if (!pcFileReader->ReadInt(&iUsedElements))
	{
		return FALSE;
	}
	mpvArray = NULL;
	miUsedElements = 0;
	mpcMalloc = pcMalloc;

	if (!SetUsedElements(iUsedElements))
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
BOOL CArrayTemplateMinimal<M>::ReadAllocatorAndHeader(CFileReader* pcFileReader)
{
	BOOL			bResult;

	if (!CArrayBlockMinimal::ReadAllocator(pcFileReader))
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, mpcMalloc);
	if (!bResult)
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
BOOL CArrayTemplateMinimal<M>::Read(CFileReader* pcFileReader)
{
	if (!ReadAllocatorAndHeader(pcFileReader))
	{
		return FALSE;
	}

	if (miUsedElements != 0)
	{
		if (!pcFileReader->ReadData(mpvArray, ByteSize())) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}


#endif // __ARRAY_TEMPLATE_MINIMAL_H__

