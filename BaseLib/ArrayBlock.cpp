#include "BubbleSort.h"
#include "QuickSort.h"
#include "TimSort.h"
#include "SystemAllocator.h"
#include "NullAllocator.h"
#include "GlobalMemory.h"
#include "ArraySizer.h"
#include "StdRandom.h"
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::_Init(void)
{
	miNumElements = 0;
	mpvArray = 0;
	miElementSize = 0;
	miUsedElements = 0;
	miChunkSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(size iElementSize)
{
	Init(&gcSystemAllocator, iElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(CMallocator* pcMalloc, size iElementSize)
{
	Init(pcMalloc, iElementSize, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(CMallocator* pcMalloc, size iElementSize, size iChunkSize)
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
void CArrayBlock::Fake(size iElementSize, void* pvData, size iNum, size iChunkSize)
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
void CArrayBlock::ReInit(void)
{
	CMallocator*	pcMalloc;
	size			iChunkSize;

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
void* CArrayBlock::SafeGet(size iIndex)
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
bool CArrayBlock::SafeSet(size iIndex, void* pvData)
{
	size		iOldLength;


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
size CArrayBlock::SetUsedElements(size iUsedElements)
{
	size	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetArraySize(iUsedElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::RemoveAtNoDeallocate(size iIndex)
{
	//This is only used by CConvexHullGenerator.RemoveDiscontiguousTriangles/
	//It should be removed.
	return RemoveAtNoDeallocate(iIndex, false, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlock::SetChunkSize(size iChunkSize)
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
void CArrayBlock::SetArraySize(size iNeededElements)
{
	void*	pv;
	size	iNumElements;
	size	iChunkSize;

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
void* CArrayBlock::Add(void)
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
void* CArrayBlock::AddGetIndex(size* piIndex)
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
size CArrayBlock::AddExistingGetIndex(void* pvData)
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
void* CArrayBlock::Add(void* pvData)
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
void CArrayBlock::Set(size iIndex, void* pvData)
{
	void*	pvTemp;

	pvTemp = Get(iIndex);
	memcpy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertAt(size iIndex)
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
void* CArrayBlock::InsertAt(void* pvData, size iIndex)
{
	void* pv;

	pv = InsertAt(iIndex);
	memcpy(pv, pvData, miElementSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::PrivateRemoveAt(size iIndex, bool bPreserveOrder, size iDataSize)
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
size CArrayBlock::RemoveAtNoDeallocate(size iIndex, bool bPreserveOrder, size iDataSize)
{
	void*	pSource;
	void*	pDest;
	void*	pEnd;
	size		iUsedElements;

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
void CArrayBlock::RemoveAt(size iIndex, bool bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::PrivateRemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder, size iDataSize)
{
	size		iNumTrailing;
	size		iNumToRemove;
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
void CArrayBlock::RemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::RemoveAt(size* paiElementsToDelete, size iNumElementsToDelete, bool bPreserveOrder)
{
	size		i;
	size		iIndex;
	size		iFirstElementToMove;
	size		iEndElementToMove;
	size		iMoved;
	size		iDestElement;
	size		iDestOffset;
	size		iNumDeleted;

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
					memcpy(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);
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
			memcpy(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);

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
bool CArrayBlock::RemoveTail(void)
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
bool CArrayBlock::RemoveFirst(void)
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
void CArrayBlock::Finalise(void)
{
	miChunkSize = 1;
	SetArraySize(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Kill(void)
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
void CArrayBlock::Zero(void)
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
void CArrayBlock::Copy(CArrayBlock* pcArray)
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
void* CArrayBlock::PushCopy(void)
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
void CArrayBlock::Push(void* pvData)
{
	void* pv;
	pv = Add();
	memcpy(pv, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlock::Pop(void* pvDest)
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
bool CArrayBlock::PopFirst(void* pvDest)
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
void* CArrayBlock::Tail(void)
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
size CArrayBlock::AddIfUnique(void* pvData)
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
size CArrayBlock::Find(void* pvData)
{
	size	i;
	void*	pv;

	for (i = 0; i < miUsedElements; i++)
	{
		pv = Get(i);
		if (memcmp(pv, pvData, miElementSize) == 0)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::AddIfUniqueKey(void* pvData, size iKeyOffset, size iKeySize)
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
size CArrayBlock::FindWithKey(void* pvData, size iKeyOffset, size iKeySize)
{
	size	i;
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
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::FindWithIntKey(size iKey)
{
	size	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (*((size*)pIndex) == iKey)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::FindWithIntKey(size iKey, size iKeyOffset)
{
	size	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = RemapSinglePointer(mpvArray, i * miElementSize + iKeyOffset);
		if (*((size*)pIndex) == iKey)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::Resize(size iNumElements)
{
	return SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::GrowToAtLeastNumElements(size iNumElements, bool bClear, uint8 iClear)
{
	size	iOldUsedElements;
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
bool CArrayBlock::Equals(CArrayBlock* pcTemplateArray)
{
	if (pcTemplateArray->miUsedElements == miUsedElements)
	{
		if (memcmp(pcTemplateArray->mpvArray, mpvArray, ByteSize()) == 0)
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
void CArrayBlock::Swap(size iIndex1, size iIndex2)
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
size CArrayBlock::InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting)
{
	size	iIndex;
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
bool CArrayBlock::FindInSorted(void* pvData, DataCompare fCompare, size* piIndex)
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
bool CArrayBlock::BinarySearch(void* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex)
{
	size	iMiddle;
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
void CArrayBlock::Reverse(void)
{
	size	i;
	size	iCount;

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
void CArrayBlock::Shuffle(CRandom* pcRandom)
{
	size		i;
	size		iNumElements;
	CRandom		cRandom;
	bool		bKillRandom;
	size		iIndex;

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
		iIndex = cRandom.Next(i, iNumElements - 1);
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
void* CArrayBlock::CopyArrayInto(CArrayBlock* pcTemplateArray, size iIndex)
{
	void*	pv;

	//Check element sizes are the same.
	//The array is always set large enough.

	pv = Get(iIndex);
	memcpy(pv, pcTemplateArray->mpvArray, miElementSize * pcTemplateArray->miUsedElements);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::CopyBlockInto(void* paElements, size iLength, size iIndex)
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
void* CArrayBlock::InsertArrayAfterEnd(CArrayBlock* pcTemplateArray)
{
	size	iArrayIndex;

	iArrayIndex = AddNum(pcTemplateArray->miUsedElements);
	return CopyArrayInto(pcTemplateArray, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertArrayBeforeStart(CArrayBlock* pcTemplateArray)
{
	InsertNumAt(pcTemplateArray->miUsedElements, 0);
	return CopyArrayInto(pcTemplateArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertArrayAt(CArrayBlock* pcTemplateArray, size iIndex)
{
	InsertNumAt(pcTemplateArray->miUsedElements, iIndex);
	return CopyArrayInto(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertNumAt(size iNumElements, size iIndex)
{
	void*	pvFrom;
	void*	pvTo;
	size	iNumToMove;

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
void* CArrayBlock::InsertBlockAfterEnd(void* paElements, size iLength)
{
	size		iArrayIndex;

	iArrayIndex = AddNum(iLength);
	return CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertBlockBeforeStart(void* paElements, size iLength)
{
	InsertNumAt(iLength, 0);
	return CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertBlockAt(void* paElements, size iLength, size iIndex)
{
	InsertNumAt(iLength, iIndex);
	return CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::Get(size iIndex)
{
	return (void*)RemapSinglePointer(mpvArray, miElementSize * iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayBlock::GetAdjustedIndex(size iIndex)
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
size CArrayBlock::GetIndex(void* pvData)
{
	size tIndex;
	size tBase;
	size tDifference;

	tBase = (size) mpvArray;
	tIndex = (size) pvData;
	tDifference = tIndex - tBase;

	//Make sure the element is correctly aligned.
	if (tDifference % miElementSize != 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	
	//Make sure the element lies within the array.
	tIndex = tDifference / miElementSize;
	if (tIndex >= (size)miUsedElements)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return (size)tIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::RemoveBatch(size uiFirstElementPos, size uiNumInBatch, size uiNumBatches, size uiSkip)
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
		uiCopyWidth = uiSkip * miElementSize;
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

		uiCopyWidth = (miUsedElements - uiSource) * miElementSize;
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
void CArrayBlock::InsertBatch(size iFirstElementPos, size iNumInBatch, int32 iNumBatches, size iSkip)
{
	size	i;
	void*	pcFirst;
	size	iTotalStride;
	void*	pcSource;
	void*	pcDest;
	size	iDest;
	size	iSource;
	size	iOldNumElements;
	size	iRemaining;
	size	iStride;

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
			memcpy(pcDest, pcSource, iRemaining * miElementSize);
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
				memcpy_fast_1byte(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
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
				memcpy_fast_2bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
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
				memcpy_fast_4bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
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
				memcpy_fast_8bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
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
				memcpy_fast_12bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
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
void CArrayBlock::GetHeader(SArrayTemplateHeader* psHeader)
{
	psHeader->miElementSize = miElementSize;
	psHeader->miUsedElements = miUsedElements;
	psHeader->miChunkSize = miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlock::WriteHeader(CFileWriter* pcFileWriter)
{
	SArrayTemplateHeader	sHeader;

	sHeader.miChunkSize = miChunkSize;
	sHeader.miElementSize = miElementSize;
	sHeader.miUsedElements = miUsedElements;

	if (!pcFileWriter->WriteData(&sHeader, sizeof(SArrayTemplateHeader))) 
	{ 
		return false; 
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayBlock::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return false;
	}

	bResult = WriteHeader(pcFileWriter);
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
bool CArrayBlock::Write(CFileWriter* pcFileWriter)
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
bool CArrayBlock::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	SArrayTemplateHeader	sHeader;

	if (!pcFileReader->ReadData(&sHeader, sizeof(SArrayTemplateHeader))) 
	{ 
		return false; 
	}

	miChunkSize = sHeader.miChunkSize;
	miElementSize = sHeader.miElementSize;
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
bool CArrayBlock::ReadAllocatorAndHeader(CFileReader* pcFileReader)
{
	bool			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.Read(pcFileReader);
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
bool CArrayBlock::Read(CFileReader* pcFileReader)
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
void* CArrayBlock::Push(void) { return Add(); }
bool CArrayBlock::PopFirst(void) { return RemoveFirst(); }
size CArrayBlock::ByteSize() { return miUsedElements * miElementSize; }
bool CArrayBlock::Pop(void) { return RemoveTail(); }
bool CArrayBlock::Contains(void* pvData) { return Find(pvData) != ARRAY_ELEMENT_NOT_FOUND; }
void CArrayBlock::BubbleSort(DataCompare fCompare) { ::BubbleSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void CArrayBlock::QuickSort(DataCompare fCompare) { ::QuickSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void CArrayBlock::TimSort(DataCompare fCompare) { ::TimSort(fCompare, mpvArray, miElementSize, miUsedElements); }
void* CArrayBlock::GetData(void) { return mpvArray; }
size CArrayBlock::ChunkSize(void) { return miChunkSize; }
size CArrayBlock::AddNum(size iNumElements) { return SetUsedElements(miUsedElements + iNumElements); }
size CArrayBlock::RemoveNum(size iNumElements) { return SetUsedElements(miUsedElements - iNumElements); }
void CArrayBlock::FakeSetUsedElements(size iUsedElements) { miUsedElements = iUsedElements; }
void CArrayBlock::SetAllocateSize(size iSize) { miChunkSize = iSize; }
size CArrayBlock::ElementSize(void) { return miElementSize; }
size CArrayBlock::NumElements(void) { return miUsedElements; }
size CArrayBlock::AllocatedElements(void) { return miNumElements; }
bool CArrayBlock::IsEmpty(void) { return miUsedElements == 0; }
bool CArrayBlock::IsNotEmpty(void) { return miUsedElements != 0; }

