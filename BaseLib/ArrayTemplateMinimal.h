/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "QuickSort.h"
#include "BubbleSort.h"
#include "DataCompare.h"
#include "ArrayElementNotFound.h"
#include "ArrayBlockMinimal.h"


template <class M>
class CArrayTemplateMinimal : public CArrayBlockMinimal, protected CPostMalloc<M>
{
protected:
	M*				mpvArray;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void 	Init(CArrayTemplateMinimal<M>* pArray);  //Used to be Copy
	void 	Init(CMallocator* pcMalloc, CArrayTemplateMinimal<M>* pArray);  //Used to be Copy
	void 	ReInit(void);
	void	Kill(void);
	void	Allocate(size iNum);
	void	Allocate(CMallocator* pcMalloc, size iNum);

	size	Resize(size iNumElements);  //Test Virtual and make sure only PostMalloc only called once.
	size	NumElements(void);

	M*		Add(void);
	M*		Add(M* pvData);
	size 	AddIfUnique(M* pvData);
	size 	AddIfUniqueKey(M* pvData, size iKeyOffset, size iKeySize);
	M* 		AddGetIndex(size* piIndex);
	size	AddNum(size iNumElements);

	M*		Get(size iElementPos);
	M*		SafeGet(size iElementPos);
	M*		Tail(void);
	M*		GetData(void);
	size	GetAdjustedIndex(size iIndex);
	size	GetIndex(M* pvElement);

	M*		InsertAt(size iElementPos);
	M*		InsertAt(M* pvData, size iElementPos);
	M*		InsertNumAt(size iNumElements, size iIndex);
	size	InsertIntoSorted(DataCompare fCompare, M* pvElement, bool bOverwriteExisting);
	void	InsertBatch(size iFirstElementPos, size uiNumInBatch, size uiNumBatches, size uiSkip);  //Test Virtual

	M*		Push(void);
	M*		PushCopy(void);

	void	QuickSort(DataCompare fCompare);
	void	BubbleSort(DataCompare fCompare);
	void	Swap(size iIndex1, size iIndex2);

	size 	Find(M* pvData);
	size 	FindWithKey(M* pvData, size iKeyOffset, size iKeySize);
	size	FindWithIntKey(size iKey, size iKeyOffset);
	bool	FindInSorted(M* pvData, DataCompare fCompare, size* piIndex);

	bool	RemoveAt(size iElementPos, bool bPreserveOrder = 0);
	bool	RemoveTail(void);
	void	RemoveBatch(size iFirstElementPos, size uiNumInBatch, size uiNumBatches, size uiSkip);

	void	Set(size iElementPos, M* pvData);

	size	ByteSize(void);
	bool	SetUsedElements(size iUsedElements);
	void	FakeSetUsedElements(size iUsedElements);

	void 	Zero(void);
	void	Zero(size iStart, size iEnd);

	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);

	bool	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	bool	WriteHeader(CFileWriter* pcFileWriter);
	bool	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	bool	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);

	bool	SetArraySize(size iNum);
	M*		SetArraySize(size iNum, size iClearValue);

protected:
	bool	BinarySearch(M* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex);
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
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMalloc)
{
	CArrayBlockMinimal::Init(pcMalloc);
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
void CArrayTemplateMinimal<M>::Allocate(size iNum)
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
void CArrayTemplateMinimal<M>::Allocate(CMallocator* pcMalloc, size iNum)
{
	CArrayBlockMinimal::Init(pcMalloc);
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
bool CArrayTemplateMinimal<M>::SetUsedElements(size iUsedElements)
{
	M*	pvTemp;

	if (iUsedElements == 0)
	{
		Free(mpvArray);
		mpvArray = NULL;

		miUsedElements = 0;
		return true;
	}

	pvTemp = (M*)Realloc(mpvArray, iUsedElements * sizeof(M));

	if (pvTemp != NULL)
	{
		miUsedElements = iUsedElements;
		mpvArray = pvTemp;
		return true;
	}
	else
	{
		gcUserError.Set("CArrayTemplateMinimal, Out of memory.");
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::Add(void)
{
	if (SetUsedElements(miUsedElements + 1))
	{
		return this->PostMalloc(&mpvArray[miUsedElements - 1]);
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
M* CArrayTemplateMinimal<M>::AddGetIndex(size* piIndex)
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
M* CArrayTemplateMinimal<M>::SafeGet(size iElementPos)
{
	if (iElementPos >= miUsedElements)
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
M* CArrayTemplateMinimal<M>::Get(size iElementPos)
{
	return &mpvArray[iElementPos];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::RemoveAt(size iElementPos, bool bPreserveOrder)
{
	M*	pvEnd;
	M*	pvElement;
	size	iUsedElements;

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
			memmove(pvElement, (M*)((size) pvElement + sizeof(M)), sizeof(M) * (iUsedElements - iElementPos));
		}
	}

	return SetUsedElements(iUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::RemoveTail(void)
{
	return RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::ByteSize(void)
{
	return miUsedElements * sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::SetArraySize(size iNum)
{
	if (miUsedElements != iNum)
	{
		return SetUsedElements(iNum);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::SetArraySize(size iNum, size iClearValue)
{
	size	iOldUsed;
	void*	pvClearStart;
	size	iClearSize;
	bool	bResult;

	if (miUsedElements != iNum)
	{
		iOldUsed = SIZE_MAX;
		if (iNum > miUsedElements)
		{
			iOldUsed = miUsedElements;
		}
		bResult = SetUsedElements(iNum);

		if (!bResult)
		{
			return NULL;
		}

		if (iOldUsed != SIZE_MAX)
		{
			pvClearStart = (void*)((size) ((size)((size) mpvArray) + (iOldUsed * sizeof(M))));
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
size CArrayTemplateMinimal<M>::NumElements(void)
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
void CArrayTemplateMinimal<M>::Zero(size iStart, size iEnd)
{
	if (iEnd <= miUsedElements)
	{
		memset((void*)Get(iStart), 0, (iEnd - iStart) * sizeof(M));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Swap(size iIndex1, size iIndex2)
{
	void* pElement1;
	void* pElement2;

	pElement1 = Get(iIndex1);
	pElement2 = Get(iIndex2);

	MemSwp(pElement1, pElement2, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::QuickSort(DataCompare fCompare)
{
	::QuickSort(fCompare, mpvArray, sizeof(M), miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::BubbleSort(DataCompare fCompare)
{
	::BubbleSort(fCompare, mpvArray, sizeof(M), miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::InsertIntoSorted(DataCompare fCompare, M* pvElement, bool bOverwriteExisting)
{
	size		iIndex;
	bool	bExists;

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
bool CArrayTemplateMinimal<M>::FindInSorted(M* pvData, DataCompare fCompare, size* piIndex)
{
	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return false;
	}
	return BinarySearch(pvData, 0, miUsedElements - 1, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::BinarySearch(M* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex)
{
	size	iMiddle;
	int		iResultMiddle;
	M*		pvMiddle;

	iResultMiddle = 0;
	iMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = Get(iMiddle);
		iResultMiddle = fCompare(pvData, pvMiddle);
		if (iResultMiddle == 0)
		{
			*piIndex = iMiddle;
			return true;
		}
		else if (iResultMiddle < 0)
		{
			if (iMiddle == 0)
			{
				break;
			}
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
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::InsertAt(size iElementPos)
{
	M* pv;

	//Grow the array.
	Add();

	//This assumes that iElementPos is within the array (or the last element).
	pv = (M*)((size) mpvArray + iElementPos * sizeof(M));
	memmove((M*)((size)pv + sizeof(M)), pv, sizeof(M) * (miUsedElements - 1 - iElementPos));
	return this->PostMalloc(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::InsertAt(M* pvData, size iElementPos)
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
size CArrayTemplateMinimal<M>::Find(M* pvData)
{
	size		i;
	M*	pPos;

	pPos = mpvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp(pPos, pvData, sizeof(M)) == 0)
		{
			return i;
		}
		pPos = (M*)((size) pPos + sizeof(M));
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::FindWithKey(M* pvData, size iKeyOffset, size iKeySize)
{
	size		i;
	M*	pPos;

	pPos = mpvArray;

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp((M*)((size) pPos + iKeyOffset), (M*)((size) pvData + iKeyOffset), iKeySize) == 0)
		{
			return i;
		}
		pPos = (M*)((size) pPos + sizeof(M));
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size	CArrayTemplateMinimal<M>::FindWithIntKey(size iKey, size iKeyOffset)
{
	size		i;
	void*	pPos;

	pPos = (void*)((size) ((size)((size) mpvArray) + iKeyOffset));
	for (i = 0; i < miUsedElements; i++)
	{
		if (*((size*)pPos) == iKey)
		{
			return i;
		}
		pPos = RemapSinglePointer(pPos, sizeof(M));
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::AddIfUnique(M* pvData)
{
	size iElementNum;

	iElementNum = Find(pvData);
	if (iElementNum == ARRAY_ELEMENT_NOT_FOUND)
	{
		Add(pvData);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::AddIfUniqueKey(M* pvData, size iKeyOffset, size iKeySize)
{
	size	iElementNum;

	iElementNum = FindWithKey(pvData, iKeyOffset, iKeySize);
	if (iElementNum == ARRAY_ELEMENT_NOT_FOUND)
	{
		Add(pvData);
		iElementNum = miUsedElements - 1;
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::GetAdjustedIndex(size iIndex)
{
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NumElements() - 1;
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::GetIndex(M* pvElement)
{
	size iPosition;
	size iBase;
	size iDifference;

	iBase = (size)(size) mpvArray;
	iPosition = (size)(size) pvElement;
	iDifference = iPosition - iBase;

	//Make sure the element is correctly aligned.
	if (iDifference % sizeof(M) != 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	//Make sure the element lies within the array.
	iPosition = iDifference / sizeof(M);
	if ((iPosition < 0) || (iPosition >= miUsedElements))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return iPosition;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Set(size iElementPos, M* pvData)
{
	memcpy((M*)((size) mpvArray + (iElementPos) * sizeof(M)), pvData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::Resize(size iNumElements)
{
	size	iOldUsedElments;

	iOldUsedElments = miUsedElements;
	SetArraySize(iNumElements);
	return iOldUsedElments;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplateMinimal<M>::AddNum(size iNumElements)
{
	size	iOldUsedElements;
	M*		pvFrom;
	bool	bResult;

	if (iNumElements < 1)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	iOldUsedElements = miUsedElements;
	bResult = SetArraySize(miUsedElements + iNumElements);
	if (!bResult)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
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
M* CArrayTemplateMinimal<M>::InsertNumAt(size iNumElements, size iIndex)
{
	M*		pvFrom;
	M*		pvTo;
	size	uiNumToMove;

	uiNumToMove = miUsedElements - iIndex;
	AddNum(iNumElements);

	pvFrom = Get(iIndex);
	pvTo = Get(iIndex + iNumElements);
	memmove(pvTo, pvFrom, uiNumToMove * sizeof(M));

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
	return Get(miUsedElements - 1);
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
void CArrayTemplateMinimal<M>::FakeSetUsedElements(size iUsedElements)
{
	this->miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::RemoveBatch(size uiFirstElementPos, size uiNumInBatch, size uiNumBatches, size uiSkip)
{
	size	i;
	void*	pcSource;
	void*	pcDest;
	size	uiDest;
	size	uiSource;
	size	uiCopyWidth;
	size	uiSourceStride;
	size	uiNewNumElements;

	if (uiNumBatches >= 2)
	{
		uiCopyWidth = uiSkip * sizeof(M);
		uiSourceStride = uiSkip + uiNumInBatch;
		for (i = 0; i < uiNumBatches - 1; i++)
		{
			uiDest = uiFirstElementPos + uiSkip * i;
			uiSource = uiFirstElementPos + uiSourceStride * i + uiNumInBatch;

			pcDest = Get(uiDest);
			pcSource = Get(uiSource);

			memcpy(pcDest, pcSource, uiCopyWidth);
		}
	}

	uiSource = uiFirstElementPos + (uiSkip + uiNumInBatch) * uiNumBatches - uiSkip;
	if (miUsedElements > uiSource)
	{
		uiDest = uiFirstElementPos + uiSkip * (uiNumBatches - 1);
		pcDest = Get(uiDest);
		pcSource = Get(uiSource);

		uiCopyWidth = (miUsedElements - uiSource) * sizeof(M);
		memcpy(pcDest, pcSource, uiCopyWidth);

		uiNewNumElements = miUsedElements - (uiNumInBatch * uiNumBatches);
		SetArraySize(uiNewNumElements);
	}
	else
	{
		uiNewNumElements = miUsedElements - (uiNumInBatch * uiNumBatches) + (uiSource - miUsedElements);
		SetArraySize(uiNewNumElements);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::InsertBatch(size iFirstElementPos, size uiNumInBatch, size uiNumBatches, size uiSkip)
{
	size	i;
	M* pcFirst;
	size	iTotalStride;
	M* pcSource;
	M* pcDest;
	size	iDest;
	size	iSource;
	size	iOldNumElements;
	size	iRemaining;
	size	iStride;

	iOldNumElements = AddNum(uiNumInBatch * uiNumBatches);

	pcFirst = Get(iFirstElementPos);

	i = uiNumBatches - 1;

	iDest = (uiNumInBatch + uiSkip) * (i + 1) - uiSkip;
	iSource = uiSkip * i;
	pcDest = (M*)RemapSinglePointer(pcFirst, iDest * sizeof(M));
	pcSource = (M*)RemapSinglePointer(pcFirst, iSource * sizeof(M));

	if (iOldNumElements > iSource)
	{
		if ((iOldNumElements - iSource) > iFirstElementPos)
		{
			iRemaining = (iOldNumElements - iSource) - iFirstElementPos;
			memcpy(pcDest, pcSource, iRemaining * sizeof(M));
		}
	}

	iStride = uiSkip * sizeof(M);
	iTotalStride = (uiNumInBatch + uiSkip) * sizeof(M);
	if (iStride == 1)
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy_fast_1byte(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
			}
			while (i != 0);
		}
	}
	else if (iStride == 2)
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy_fast_2bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
			}
			while (i != 0);
		}
	}
	else if (iStride == 4)
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy_fast_4bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
			}
			while (i != 0);
		}
	}
	else if (iStride == 8)
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy_fast_8bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
			}
			while (i != 0);
		}
	}
	else if (iStride == 12)
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy_fast_12bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
			}
			while (i != 0);
		}
	}
	else
	{
		i = uiNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), iStride);
			}
			while (i != 0);
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
bool CArrayTemplateMinimal<M>::WriteHeader(CFileWriter* pcFileWriter)
{
	return CArrayBlockMinimal::WriteHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	return CArrayBlockMinimal::WriteAllocatorAndHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::Write(CFileWriter* pcFileWriter)
{
	if (!WriteAllocatorAndHeader(pcFileWriter))
	{
		return false;
	}

	if (miUsedElements != 0)
	{
		if (!pcFileWriter->WriteData(mpvArray, ByteSize())) 
		{ 
			return false; 
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	size		iUsedElements;

	if (!pcFileReader->ReadSize(&iUsedElements))
	{
		return false;
	}
	mpvArray = NULL;
	miUsedElements = 0;
	CMalloc::Init(pcMalloc);

	if (!SetUsedElements(iUsedElements))
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::ReadAllocatorAndHeader(CFileReader* pcFileReader)
{
	bool			bResult;

	if (!CArrayBlockMinimal::ReadAllocator(pcFileReader))
	{
		return false;
	}

	bResult = ReadHeader(pcFileReader, mpcMalloc);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplateMinimal<M>::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	if (!ReadAllocatorAndHeader(pcFileReader))
	{
		return false;
	}

	if (miUsedElements != 0)
	{
		if (!pcFileReader->ReadData(mpvArray, ByteSize())) 
		{ 
			return false; 
		}
	}
	return true;
}


#endif // __ARRAY_TEMPLATE_MINIMAL_H__

