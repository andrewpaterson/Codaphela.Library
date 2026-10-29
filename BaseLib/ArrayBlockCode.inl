/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
//No include guard: ArrayBlockCode.inl is included once per array size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::_Init(void)
{
	miNumElements = 0;
	mpvArray = NULL;
	miElementSize = 0;
	miUsedElements = 0;
	miChunkSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Init(ASIZE iElementSize)
{
	Init(&gcSystemAllocator, iElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Init(CMallocator* pcMalloc, ASIZE iElementSize)
{
	Init(pcMalloc, iElementSize, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Init(CMallocator* pcMalloc, ASIZE iElementSize, ASIZE iChunkSize)
{
	CMalloc::Init(pcMalloc);
	mpvArray = NULL;
	miNumElements = 0;
	miUsedElements = 0;
	miElementSize = iElementSize;
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Init(CMallocator* pcMalloc, ASIZE iElementSize, void* pvData, ASIZE uiNumData)
{
	Init(pcMalloc, iElementSize);
	Resize(uiNumData);
	memcpy(mpvArray, pvData, iElementSize * uiNumData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Fake(ASIZE iElementSize, void* pvData, ASIZE iNum, ASIZE iChunkSize)
{
	CMalloc::Init(&gcNullAllocator);
	mpvArray = pvData;
	miElementSize = iElementSize;
	miNumElements = iChunkSize;
	miUsedElements = iNum;
	miChunkSize = 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::ReInit(void)
{
	CMallocator*	pcMalloc;
	ASIZE			iChunkSize;

	pcMalloc = mpcMalloc;
	iChunkSize = miChunkSize;
	Kill();
	Init(pcMalloc, miElementSize);
	SetAllocateSize(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::SafeGet(ASIZE iIndex)
{
	if (iIndex < miUsedElements)
	{
		return Get(iIndex);
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
bool ARRAY_BLOCK::SafeSet(ASIZE iIndex, void* pvData)
{
	ASIZE		iOldLength;


	if (iIndex < miUsedElements)
	{
		Set(iIndex, pvData);
		return true;
	}
	else
	{
		iOldLength = NumElements();
		SetUsedElements(iIndex+1);
		memset(Get(iOldLength), 0, (iIndex-iOldLength) * miElementSize);
		Set(iIndex, pvData);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::SetUsedElements(ASIZE iUsedElements)
{
	ASIZE	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetArraySize(iUsedElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::RemoveAtNoDeallocate(ASIZE iIndex)
{
	//This is only used by CConvexHullGenerator.RemoveDiscontiguousTriangles/
	//It should be removed.
	return RemoveAtNoDeallocate(iIndex, false, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::SetChunkSize(ASIZE iChunkSize)
{
	if (iChunkSize < miChunkSize)
	{
		if (miChunkSize % iChunkSize == 0)
		{
			miChunkSize = iChunkSize;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (iChunkSize % miChunkSize == 0)
		{
			miChunkSize = iChunkSize;
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::SetArraySize(ASIZE iNeededElements)
{
	void*	pv;
	ASIZE	iNumElements;
	ASIZE	iChunkSize;

	if (iNeededElements == 0)
	{
		if (mpvArray != NULL)
		{
			Free(mpvArray);
			mpvArray = NULL;
		}
		miNumElements = 0;
		miUsedElements = 0;
	}
	else
	{
		iChunkSize = CalculateChunkSize(iNeededElements, miChunkSize);
		iNumElements = CalculateArraySize(iNeededElements, iChunkSize);

		if (miNumElements != iNumElements)
		{
			pv = (void*)Realloc(mpvArray, miElementSize * iNumElements);
			mpvArray = pv;
			if (mpvArray == NULL)
			{
				iNeededElements = 0;
				iNumElements = 0;
			}
		}

		miUsedElements = iNeededElements;
		miChunkSize = iChunkSize;
		miNumElements = iNumElements;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::Add(void)
{
	miUsedElements++;
	if (miUsedElements > miNumElements)
	{
		SetArraySize(miUsedElements);
	}
	return Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::AddGetIndex(ASIZE* piIndex)
{
	void* pv;

	pv = Add();
	(*piIndex) = miUsedElements - 1;
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::AddExistingGetIndex(void* pvData)
{
	void* pv;

	pv = Add();
	memcpy(pv, pvData, miElementSize);
	return miUsedElements - 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::Add(void* pvData)
{
	void* pv;

	pv = Add();
	memcpy(pv, pvData, miElementSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Set(ASIZE iIndex, void* pvData)
{
	void*	pvTemp;

	pvTemp = Get(iIndex);
	memcpy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertAt(ASIZE iIndex)
{
	void*	pSource;
	void*	pDest;

	//The new element will be at iIndex
	if (iIndex > miNumElements)
	{
		return NULL;
	}

	//This assumes that iIndex is within the array (or the last element).
	miUsedElements++;
	if (miUsedElements > miNumElements)
	{
		SetArraySize(miUsedElements);
	}
	pSource = Get(iIndex);
	pDest = RemapSinglePointer(pSource, miElementSize);

	memmove(pDest, pSource, miElementSize * (miUsedElements - 1 - iIndex));
	return (void*)pSource;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertAt(void* pvData, ASIZE iIndex)
{
	void* pv;

	pv = InsertAt(iIndex);
	if (pv)
	{
		memcpy(pv, pvData, miElementSize);
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::PrivateRemoveAt(ASIZE iIndex, bool bPreserveOrder, ASIZE iDataSize)
{
	if (iIndex < miUsedElements)
	{
		miUsedElements = RemoveAtNoDeallocate(iIndex, bPreserveOrder, iDataSize);

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
ASIZE ARRAY_BLOCK::RemoveAtNoDeallocate(ASIZE iIndex, bool bPreserveOrder, ASIZE iDataSize)
{
	void*	pSource;
	void*	pDest;
	void*	pEnd;
	ASIZE	iUsedElements;

	iUsedElements = miUsedElements;
	iUsedElements--;
	if (iIndex == iUsedElements)
	{
		return iUsedElements;
	}

	pSource = RemapSinglePointer(mpvArray, iIndex * iDataSize);

	if (bPreserveOrder)
	{
		//If the order of elements is to be preserved then move all the elements back one.
		pDest = (void*)RemapSinglePointer(pSource, iDataSize);
		memmove(pSource, pDest, iDataSize * (iUsedElements - iIndex));
	}
	else
	{
		//If the order is unimportant then just move the last element to the empty.
		pEnd = (void*)RemapSinglePointer(mpvArray, iUsedElements * iDataSize);
		memcpy(pSource, pEnd, iDataSize);
	}
	return iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::RemoveAt(ASIZE iIndex, bool bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::PrivateRemoveRange(ASIZE iStartIndex, ASIZE iEndIndexExclusive, bool bPreserveOrder, ASIZE iDataSize)
{
	ASIZE	iNumTrailing;
	ASIZE	iNumToRemove;
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
void ARRAY_BLOCK::RemoveRange(ASIZE iStartIndex, ASIZE iEndIndexExclusive, bool bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::RemoveAt(ASIZE* paiElementsToDelete, ASIZE iNumElementsToDelete, bool bPreserveOrder)
{
	ASIZE		i;
	ASIZE		iIndex;
	ASIZE		iFirstElementToMove;
	ASIZE		iEndElementToMove;
	ASIZE		iMoved;
	ASIZE		iDestElement;
	ASIZE		iDestOffset;
	ASIZE		iNumDeleted;

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
					memmove(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);
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
			memmove(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);

			iDestOffset += iNumDeleted;

			iFirstElementToMove = iEndElementToMove;
		}
	}
	else
	{
		i = iNumElementsToDelete;
		if (i != 0)
		{
			do
			{
				i--;
				iIndex = paiElementsToDelete[i];
				RemoveAt(iIndex, false);
			}
			while (iNumElementsToDelete != 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::RemoveTail(void)
{
	if (miUsedElements != 0)
	{
		RemoveAt(miUsedElements - 1);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::RemoveFirst(void)
{
	if (miUsedElements != 0)
	{
		RemoveAt(0);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Finalise(void)
{
	//THIS DOES NOT WORK.
	//Chunk Size is recalculated in SetArraySize so setting the chunk size to 1 here does nothing.

	miChunkSize = 1;
	SetArraySize(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Kill(void)
{
	if (mpvArray != NULL)
	{
		Free(mpvArray);
		miNumElements = 0;
		miUsedElements = 0;
	}
	mpvArray = NULL;
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Zero(void)
{
	if (miUsedElements != 0)
	{
		memset(mpvArray, 0, miUsedElements * miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::One(void)
{
	if (miUsedElements != 0)
	{
		memset(mpvArray, 0xff, miUsedElements * miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Copy(ARRAY_BLOCK* pcArray)
{
	//Assumes the array is initialised.  
	if (pcArray->miNumElements != miNumElements)
	{
		SetArraySize(pcArray->miUsedElements);
	}
	else
	{
		miUsedElements = pcArray->miUsedElements;
	}
	CopyArrayInto(pcArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::PushCopy(void)
{
	void*	pDest;
	void*	pSource;

	if (miUsedElements > 0)
	{
		pDest = Add();
		pSource = Get(miUsedElements-2);
		memcpy(pDest, pSource, miElementSize);
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
void ARRAY_BLOCK::Push(void* pvData)
{
	void* pv;
	pv = Add();
	memcpy(pv, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::Pop(void* pvDest)
{
	void*	pvSource;

	if (miUsedElements != 0)
	{
		pvSource = Get(miUsedElements - 1);
		memcpy(pvDest, pvSource, miElementSize);
		RemoveAt(miUsedElements - 1);
		return true;
	}
	else
	{
		memset(pvDest, 0, miElementSize);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::PopFirst(void* pvDest)
{
	void* pvSource;

	if (miUsedElements != 0)
	{
		pvSource = Get(0);
		memcpy(pvDest, pvSource, miElementSize);
		RemoveAt(0);
		return true;
	}
	else
	{
		memset(pvDest, 0, miElementSize);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::Tail(void)
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
ASIZE ARRAY_BLOCK::AddIfUnique(void* pvData)
{
	ASIZE iElementNum;

	iElementNum = Find(pvData);
	if (iElementNum == ARRAY_NOT_FOUND)
	{
		Add(pvData);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::Find(void* pvData)
{
	ASIZE	i;
	void*	pv;

	for (i = 0; i < miUsedElements; i++)
	{
		pv = Get(i);
		if (memcmp(pv, pvData, miElementSize) == 0)
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::AddIfUniqueKey(void* pvData, ASIZE iKeyOffset, ASIZE iKeySize)
{
	ASIZE	iElementNum;

	iElementNum = FindWithKey(pvData, iKeyOffset, iKeySize);
	if (iElementNum == ARRAY_NOT_FOUND)
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
ASIZE ARRAY_BLOCK::FindWithKey(void* pvData, ASIZE iKeyOffset, ASIZE iKeySize)
{
	ASIZE	i;
	void*	pIndex;
	void*	pTest;

	pIndex = RemapSinglePointer(mpvArray, iKeyOffset);
	pTest = RemapSinglePointer(pvData, iKeyOffset);

	for (i = 0; i < miUsedElements; i++)
	{
		if (memcmp(pIndex, pTest, iKeySize) == 0)
		{
			return i;
		}
		pIndex = RemapSinglePointer(pIndex, miElementSize);
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::FindWithIntKey(int iKey)
{
	ASIZE	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::FindWithIntKey(int iKey, ASIZE iKeyOffset)
{
	ASIZE	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = RemapSinglePointer(mpvArray, i * miElementSize + iKeyOffset);
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::Resize(ASIZE iNumElements)
{
	return SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::GrowToAtLeastNumElements(ASIZE iNumElements, bool bClear, uint8 iClear)
{
	ASIZE	iOldUsedElements;
	void*	pvStart;

	if (iNumElements == 0)
	{
		return NULL;
	}

	if (miUsedElements < iNumElements)
	{
		iOldUsedElements = Resize(iNumElements);
		if (bClear)
		{
			if (iOldUsedElements < iNumElements)
			{
				pvStart = Get(iOldUsedElements);
				memset(pvStart, iClear, (iNumElements - iOldUsedElements) * miElementSize);
			}
		}
		return Get(iNumElements - 1);
	}
	else
	{
		return Get(iNumElements - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::Equals(ARRAY_BLOCK* pcOtherArray)
{
	if (pcOtherArray->miUsedElements == miUsedElements)
	{
		if (memcmp(pcOtherArray->mpvArray, mpvArray, ByteSize()) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Swap(ASIZE iIndex1, ASIZE iIndex2)
{
	void*	pElement1;
	void*	pElement2;

	pElement1 = Get(iIndex1);
	pElement2 = Get(iIndex2);

	MemSwp(pElement1, pElement2, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting)
{
	ASIZE	iIndex;
	bool	bExists;

	bExists = FindInSorted(pvData, fCompare, &iIndex);
	if (iIndex < miUsedElements)
	{
		if (!bExists)
		{
			InsertAt(pvData, iIndex);
		}
		else
		{
			if (bOverwriteExisting)
			{
				Set(iIndex, pvData);
			}
			else
			{
				InsertAt(pvData, iIndex);
			}
		}
	}
	else
	{
		Add(pvData);
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::FindInSorted(void* pvData, DataCompare fCompare, ASIZE* piIndex)
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
bool ARRAY_BLOCK::BinarySearch(void* pvData, ASIZE iLeft, ASIZE iRight, DataCompare fCompare, ASIZE* piIndex)
{
	ASIZE	iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;

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
void ARRAY_BLOCK::Reverse(void)
{
	ASIZE	i;
	ASIZE	iCount;

	iCount = miUsedElements/2;
	for (i = 0; i < iCount; i++)
	{
		Swap(i, miUsedElements - i - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::Shuffle(CRandom* pcRandom)
{
	ASIZE		i;
	ASIZE		iNumElements;
	CRandom		cRandom;
	bool		bKillRandom;
	ASIZE		iIndex;

	bKillRandom = false;
	if (pcRandom == NULL)
	{
		cRandom.Init();
		bKillRandom = true;
		pcRandom = &cRandom;
	}

	iIndex = 0;
	iNumElements = NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		iIndex = pcRandom->Next((int32)i, (int32)(iNumElements - 1));
		Swap(i, iIndex);
	}

	if (bKillRandom)
	{
		pcRandom = NULL;
		cRandom.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::CopyArrayInto(ARRAY_BLOCK* pcOtherArray, ASIZE iIndex)
{
	void*	pv;

	//Check element sizes are the same.
	//The array is always set large enough.

	pv = Get(iIndex);
	memcpy(pv, pcOtherArray->mpvArray, miElementSize * pcOtherArray->miUsedElements);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::CopyBlockInto(void* paElements, ASIZE iLength, ASIZE iIndex)
{
	void*	pv;

	pv = Get(iIndex);
	memcpy(pv, paElements, miElementSize * iLength);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertArrayAfterEnd(ARRAY_BLOCK* pcOtherArray)
{
	ASIZE	iArrayIndex;

	iArrayIndex = AddNum(pcOtherArray->miUsedElements);
	return CopyArrayInto(pcOtherArray, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertArrayBeforeStart(ARRAY_BLOCK* pcOtherArray)
{
	InsertNumAt(pcOtherArray->miUsedElements, 0);
	return CopyArrayInto(pcOtherArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertArrayAt(ARRAY_BLOCK* pcOtherArray, ASIZE iIndex)
{
	InsertNumAt(pcOtherArray->miUsedElements, iIndex);
	return CopyArrayInto(pcOtherArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertNumAt(ASIZE iNumElements, ASIZE iIndex)
{
	void*	pvFrom;
	void*	pvTo;
	ASIZE	iNumToMove;

	if ((iNumElements == 0) || (iIndex > miNumElements))
	{
		return NULL;
	}

	iNumToMove = miUsedElements - iIndex;
	AddNum(iNumElements);

	pvFrom = (void*)Get(iIndex);
	pvTo = (void*)Get(iIndex + iNumElements);
	memmove(pvTo, pvFrom, iNumToMove * miElementSize);
	return (void*)pvFrom;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertBlockAfterEnd(void* paElements, ASIZE iLength)
{
	ASIZE		iArrayIndex;

	iArrayIndex = AddNum(iLength);
	return CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertBlockBeforeStart(void* paElements, ASIZE iLength)
{
	InsertNumAt(iLength, 0);
	return CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::InsertBlockAt(void* paElements, ASIZE iLength, ASIZE iIndex)
{
	InsertNumAt(iLength, iIndex);
	return CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* ARRAY_BLOCK::Get(ASIZE iIndex)
{
	return (void*)RemapSinglePointer(mpvArray, miElementSize * iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::GetAdjustedIndex(ASIZE iIndex)
{
	if (iIndex == ARRAY_NOT_FOUND)
	{
		return NumElements() - 1;
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::GetIndex(void* pvData)
{
	ASIZE tIndex;
	ASIZE tBase;
	ASIZE tDifference;

	tBase = (ASIZE) mpvArray;
	tIndex = (ASIZE) pvData;
	tDifference = tIndex - tBase;

	//Make sure the element is correctly aligned.
	if (tDifference % miElementSize != 0)
	{
		return ARRAY_NOT_FOUND;
	}
	
	//Make sure the element lies within the array.
	tIndex = tDifference / miElementSize;
	if (tIndex >= (ASIZE)miUsedElements)
	{
		return ARRAY_NOT_FOUND;
	}

	return (ASIZE)tIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::RemoveBatch(ASIZE uiFirstElementPos, ASIZE uiNumInBatch, ASIZE uiNumBatches, ASIZE uiSkip)
{
	ASIZE	i;
	void*	pcSource;
	void*	pcDest;
	ASIZE	uiDest;
	ASIZE	uiSource;
	ASIZE	uiCopyWidth;
	ASIZE	uiSourceStride;
	ASIZE	uiNewNumElements;

	if (uiNumBatches >= 2)
	{
		uiCopyWidth = uiSkip * miElementSize;
		uiSourceStride = uiSkip + uiNumInBatch;
		for (i = 0; i < uiNumBatches - 1; i++)
		{
			uiDest = uiFirstElementPos + uiSkip * i;
			uiSource = uiFirstElementPos + uiSourceStride * i + uiNumInBatch;

			pcDest = Get(uiDest);
			pcSource = Get(uiSource);

			memmove(pcDest, pcSource, uiCopyWidth);
		}
	}

	uiSource = uiFirstElementPos + (uiSkip + uiNumInBatch) * uiNumBatches - uiSkip;
	if (miUsedElements > uiSource)
	{
		uiDest = uiFirstElementPos + uiSkip * (uiNumBatches - 1);
		pcDest = Get(uiDest);
		pcSource = Get(uiSource);

		uiCopyWidth = (miUsedElements - uiSource) * miElementSize;
		memmove(pcDest, pcSource, uiCopyWidth);

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
void ARRAY_BLOCK::RemoveAll(void)
{
	SetArraySize(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::Remove(void* pvData, bool bPreserveOrder)
{
	ASIZE	iIndex;

	iIndex = GetIndex(pvData);
	if (iIndex != ARRAY_NOT_FOUND)
	{
		RemoveAt(iIndex, bPreserveOrder);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::RemoveArray(ARRAY_BLOCK* pcOtherArray, bool bPreserveOrder)
{
	ASIZE	i;
	ASIZE	iOtherNumElements;
	void*	pvOther;
	ASIZE	iThisIndex;
	ASIZE	iRemoved;

	iRemoved = 0;
	iOtherNumElements = pcOtherArray->NumElements();
	for (i = 0; i < iOtherNumElements; i++)
	{
		pvOther = pcOtherArray->Get(i);
		iThisIndex = Find(pvOther);
		if (iThisIndex != ARRAY_NOT_FOUND)
		{
			RemoveAt(iThisIndex, bPreserveOrder);
			iRemoved++;
		}
	}
	return iRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ASIZE ARRAY_BLOCK::RemoveArrayFromSorted(ARRAY_BLOCK* pcOtherArray, DataCompare fCompare)
{
	ASIZE	i;
	ASIZE	iOtherNumElements;
	void*	pvOther;
	ASIZE	iThisIndex;
	bool	bFound;
	ASIZE	iRemoved;

	iRemoved = 0;
	iOtherNumElements = pcOtherArray->NumElements();
	for (i = 0; i < iOtherNumElements; i++)
	{
		pvOther = pcOtherArray->Get(i);
		bFound = FindInSorted(pvOther, fCompare, &iThisIndex);
		if (bFound)
		{
			RemoveAt(iThisIndex, true);
			iRemoved++;
		}
	}
	return iRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::InsertBatch(ASIZE iFirstElementPos, ASIZE iNumInBatch, int32 iNumBatches, ASIZE iSkip)
{
	ASIZE	i;
	void*	pcFirst;
	ASIZE	iTotalStride;
	void*	pcSource;
	void*	pcDest;
	ASIZE	iDest;
	ASIZE	iSource;
	ASIZE	iOldNumElements;
	ASIZE	iRemaining;
	ASIZE	iStride;

	iOldNumElements = AddNum(iNumInBatch * iNumBatches);

	pcFirst = Get(iFirstElementPos);

	i = iNumBatches - 1;

	iDest = (iNumInBatch + iSkip) * (i + 1) - iSkip;
	iSource = iSkip * i;
	pcDest = RemapSinglePointer(pcFirst, iDest * miElementSize);
	pcSource = RemapSinglePointer(pcFirst, iSource * miElementSize);

	if (iOldNumElements > iSource)
	{
		if ((iOldNumElements - iSource) > iFirstElementPos)
		{
			iRemaining = (iOldNumElements - iSource) - iFirstElementPos;
			memmove(pcDest, pcSource, iRemaining * miElementSize);
		}
	}

	iStride = iSkip * miElementSize;
	iTotalStride = (iNumInBatch + iSkip) * miElementSize;
	if (iStride == 1)
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), 1);
			}
			while (i != 0);
		}
	}
	else if (iStride == 2)
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), 2);
			}
			while (i != 0);
		}
	}
	else if (iStride == 4)
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), 4);
			}
			while (i != 0);
		}
	}
	else if (iStride == 8)
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memmove(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), 8);
			}
			while (i != 0);
		}
	}
	else if (iStride == 12)
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memmove(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), 12);
			}
			while (i != 0);
		}
	}
	else
	{
		i = iNumBatches - 1;
		if (i != 0)
		{
			do
			{
				i--;
				memmove(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), iStride);
			}
			while (i != 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ARRAY_BLOCK::GetHeader(ARRAY_HEADER* psHeader)
{
	psHeader->miElementSize = miElementSize;
	psHeader->miUsedElements = miUsedElements;
	psHeader->miChunkSize = miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_HEADER::Write(CFileWriter* pcFileWriter)
{
	return pcFileWriter->WriteSize(miElementSize) &&
		   pcFileWriter->WriteSize(miUsedElements) &&
		   pcFileWriter->WriteSize(miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_HEADER::Read(CFileReader* pcFileReader)
{
	return pcFileReader->ReadSize(&miElementSize) &&
		   pcFileReader->ReadSize(&miUsedElements) &&
		   pcFileReader->ReadSize(&miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::WriteHeader(CFileWriter* pcFileWriter)
{
	return WriteHeader(pcFileWriter, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//uiFileElementSize is written in place of the element size.
bool ARRAY_BLOCK::WriteHeader(CFileWriter* pcFileWriter, ASIZE uiFileElementSize)
{
	ARRAY_HEADER	sHeader;

	sHeader.miChunkSize = miChunkSize;
	sHeader.miElementSize = uiFileElementSize;
	sHeader.miUsedElements = miUsedElements;

	if (!sHeader.Write(pcFileWriter)) 
	{ 
		return false; 
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = WriteMallocator(pcFileWriter, mpcMalloc);
	ReturnOnFalse(bResult);

	bResult = WriteHeader(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::Write(CFileWriter* pcFileWriter)
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
bool ARRAY_BLOCK::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	return ReadHeader(pcFileReader, pcMalloc, ARRAY_SIZE_MAX, ARRAY_SIZE_MAX);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//The element size in the file must be uiFileElementSize and the elements are uiElementSize in memory.  SIZE_MAX for
//both uses the element size in the file.
bool ARRAY_BLOCK::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, ASIZE uiFileElementSize, ASIZE uiElementSize)
{
	ARRAY_HEADER	sHeader;

	if (!sHeader.Read(pcFileReader)) 
	{ 
		return false; 
	}

	if (uiFileElementSize == ARRAY_SIZE_MAX)
	{
		uiElementSize = sHeader.miElementSize;
	}
	else if (sHeader.miElementSize != uiFileElementSize)
	{
		return false;
	}

	miChunkSize = sHeader.miChunkSize;
	miElementSize = uiElementSize;
	miNumElements = 0;
	miUsedElements = 0;
	mpvArray = NULL;
	CMalloc::Init(pcMalloc);

	if (sHeader.miUsedElements != 0)
	{
		SetUsedElements(sHeader.miUsedElements);
		return mpvArray != NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::ReadAllocatorAndHeader(CFileReader* pcFileReader)
{
	bool			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc);
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
bool ARRAY_BLOCK::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	bool			bResult;

	bResult = ReadAllocatorAndHeader(pcFileReader);
	if (!bResult)
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool ARRAY_BLOCK::IsSorted(DataCompare fCompare)
{
	ASIZE	ui;
	ASIZE	uiNumElements;
	void*	pvLeft;
	void*	pvRight;
	int		iResult;

	if (miUsedElements < 2)
	{
		return true;
	}

	uiNumElements = miUsedElements - 1;
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pvLeft = Get(ui);
		pvRight = Get(ui+1);
		iResult = fCompare(pvLeft, pvRight);
		if (iResult > 0)
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
void* ARRAY_BLOCK::Push(void) { return Add(); }
bool ARRAY_BLOCK::PopFirst(void) { return RemoveFirst(); }
ASIZE ARRAY_BLOCK::ByteSize() { return miUsedElements * miElementSize; }
bool ARRAY_BLOCK::Pop(void) { return RemoveTail(); }
bool ARRAY_BLOCK::Contains(void* pvData) { return Find(pvData) != ARRAY_NOT_FOUND; }
void ARRAY_BLOCK::Sort(DataCompare fCompare) { TimSort(fCompare); }
void ARRAY_BLOCK::BubbleSort(DataCompare fCompare) { ::BubbleSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void ARRAY_BLOCK::QuickSort(DataCompare fCompare) { ::QuickSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void ARRAY_BLOCK::TimSort(DataCompare fCompare) { ::TimSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void* ARRAY_BLOCK::GetData(void) { return mpvArray; }
ASIZE ARRAY_BLOCK::ChunkSize(void) { return miChunkSize; }
ASIZE ARRAY_BLOCK::AddNum(ASIZE iNumElements) { return SetUsedElements(miUsedElements + iNumElements); }
ASIZE ARRAY_BLOCK::RemoveNum(ASIZE iNumElements) { return SetUsedElements(miUsedElements - iNumElements); }
void ARRAY_BLOCK::FakeSetUsedElements(ASIZE iUsedElements) { miUsedElements = iUsedElements; }
void ARRAY_BLOCK::SetAllocateSize(ASIZE iSize) { miChunkSize = iSize; }
ASIZE ARRAY_BLOCK::ElementSize(void) { return miElementSize; }
ASIZE ARRAY_BLOCK::NumElements(void) { return miUsedElements; }
ASIZE ARRAY_BLOCK::AllocatedElements(void) { return miNumElements; }
bool ARRAY_BLOCK::IsEmpty(void) { return miUsedElements == 0; }
bool ARRAY_BLOCK::IsNotEmpty(void) { return miUsedElements != 0; }
