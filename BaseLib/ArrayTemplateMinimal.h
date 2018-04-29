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
#include "Define.h"
#include "PointerRemapper.h"
#include "PointerFunctions.h"
#include "ErrorHandler.h"
#include "FileIO.h"
#include "Mallocator.h"
#include "SystemAllocator.h"
#include "ConstructorCall.h"


template <class M>
class CArrayTemplateMinimal : protected CPostMalloc<M>
{
protected:
	int				miUsedElements;
	M*				mpvArray;
	CMallocator*	mpcMalloc;

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

	int		NumElements(void);

	M*		Add(void);
	M*		Add(M* pvData);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);
	M* 		AddGetIndex(int* piIndex);


	M*		Get(int iElementPos);
	M*		SafeGet(int iElementPos);
	M*		Tail(void);
	M*		GetData(void);
	int		GetAdjustedIndex(int iIndex);
	int		GetIndex(M* pvElement);

	M*		InsertAt(int iElementPos);
	M*		InsertAt(M* pvData, int iElementPos);
	M*		InsertNumAt(int iNumElements, int iElementPos);
	int		InsertIntoSorted(int(*fCompare)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting);
	void	InsertBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);  //Test Virtual

	M*		Push(void);
	M*		PushCopy(void);

	int		GrowToNumElements(int iNumElements);  //Test Virtual and make sure only PostMalloc only called once.
	int		GrowByNumElements(int iNumElements);  //Test Virtual and make sure only PostMalloc only called once.

	void	QuickSort(int(*fCompare)(const void*, const void*));

	int 	Find(M* pData);
	int 	FindWithKey(M* pData, int iKeyOffset, int iKeySize);
	int		FindWithIntKey(int iKey, int iKeyOffset);
	BOOL	FindInSorted(M* pData, int(*)(const void*, const void*), int* piIndex);

	BOOL	RemoveAt(int iElementPos, int bPreserveOrder = 0);
	BOOL	RemoveTail(void);
	void	RemoveBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch);

	void	Set(int iElementPos, M* pvData);

	int		ByteSize(void);
	BOOL	SetUsedElements(int iUsedElements);

	void 	Zero(void);
	void	Zero(int iStart, int iEnd);

	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);

	BOOL	WriteAllocatorAndHeader(CFileWriter* pcFileWriter);
	BOOL	WriteHeader(CFileWriter* pcFileWriter);
	BOOL	ReadAllocatorAndHeader(CFileReader* pcFileReader);
	BOOL	ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc);

protected:
	BOOL	SetArraySize(int iNum);
	M*		SetArraySize(int iNum, int iClearValue);
	BOOL	BinarySearch(M* pData, int iLeft, int iRight, int(*Func)(const void*, const void*), int* piIndex);
	void	FakeSetUsedElements(int iUsedElements);

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
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMallocator)
{
	miUsedElements = 0;
	mpvArray = NULL;
	mpcMalloc = pcMallocator;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(int iIgnored)
{
	Init();
	iIgnored = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMallocator, int iIgnored)
{
	Init(pcMallocator);
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
	mpcMalloc = NULL;
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Allocate(int iNum)
{
	Allocate(&gcSystemAllocator, iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Allocate(CMallocator* pcMallocator, int iNum)
{
	Init(pcMallocator);
	SetArraySize(iNum);
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CArrayTemplateMinimal* pArray)
{
	Init(&gcSystemAllocator, pArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::Init(CMallocator* pcMallocator, CArrayTemplateMinimal* pArray)
{
	CMallocator*	pcMalloc;

	Allocate(pcMalloc, pArray->NumElements());
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
		return PostMalloc(&mpvArray[miUsedElements-1]);
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
	return PostMalloc(pv);
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
int CArrayTemplateMinimal<M>::GrowToNumElements(int iNumElements)
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
int CArrayTemplateMinimal<M>::GrowByNumElements(int iNumElements)
{
	int	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetArraySize(miUsedElements + iNumElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplateMinimal<M>::InsertNumAt(int iNumElements, int iElementPos)
{
	M*		pvNew;
	M*		pvFrom;
	M*		pvTo;
	int		iNumToMove;

	if (iNumElements <= 0)
	{
		return NULL;
	}

	pvNew = (M*)Malloc((miUsedElements + iNumElements) * sizeof(M));

	if (iElementPos > 0)
	{
		memcpy(pvNew, mpvArray, iElementPos * sizeof(M));
	}

	iNumToMove = miUsedElements - iElementPos;
	pvFrom = Get(iElementPos);
	pvTo = &pvNew[iElementPos + iNumElements];
	memcpy(pvTo, pvFrom, iNumToMove * sizeof(M));

	Free(mpvArray);
	mpvArray = pvNew;

	pvFrom = Get(iElementPos);
	return PostMalloc(pvFrom, iNumElements, sizeof(M));
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
void CArrayTemplateMinimal<M>::RemoveBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
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

	SetArraySize(miUsedElements - iNumInBatch * iNumBatches);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplateMinimal<M>::InsertBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
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
	if (!pcFileWriter->WriteInt(miUsedElements))
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
BOOL CArrayTemplateMinimal<M>::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteHeader(pcFileWriter);
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
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc);
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

