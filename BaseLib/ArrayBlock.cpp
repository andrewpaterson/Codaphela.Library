#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(int iElementSize, int iChunkSize)
{
	Init(&gcSystemAllocator, iElementSize, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(CMallocator* pcMalloc, int iElementSize, int iChunkSize)
{
	mpcMalloc = pcMalloc;
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
void CArrayBlock::Allocate(CMallocator* pcMalloc, int iElementSize, int iNumElements)
{
	Init(pcMalloc, iElementSize, iNumElements);
	GrowByChunk();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Allocate(CMallocator* pcMalloc, int iElementSize, int iChunkSize, int iNumElements)
{
	mpcMalloc = pcMalloc;
	miElementSize = iElementSize;
	miChunkSize = iChunkSize;
	mpvArray = NULL;
	SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Fake(int iElementSize, void* pvData, int iNum, int iChunkSize)
{
	mpcMalloc = NULL;
	mpvArray = pvData;
	miElementSize = iElementSize;
	miNumElements = iChunkSize;
	miUsedElements = iNum;
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::ReInit(int iChunkSize)
{
	CMallocator* pcMalloc;

	pcMalloc = mpcMalloc;
	Kill();
	if (iChunkSize == 0)
	{
		Init(pcMalloc, miElementSize, miChunkSize);
	}
	else
	{
		Init(pcMalloc, miElementSize, iChunkSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::Realloc(void* pv, size_t tSize)
{
	return mpcMalloc->Realloc(pv, tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::SafeGet(int iIndex)
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
BOOL CArrayBlock::SafeSet(int iIndex, void* pvData)
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
void CArrayBlock::SetArraySize(int iNumElements)
{
	void* ptr;

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
			ptr = (void*)Realloc(mpvArray, miElementSize * iNumElements);
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
void* CArrayBlock::Add(void)
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
void* CArrayBlock::AddGetIndex(int* piIndex)
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
void* CArrayBlock::Add(void* pvData)
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
void CArrayBlock::Set(int iIndex, void* pvData)
{
	void*	pvTemp;

	pvTemp = Get(iIndex);
	memcpy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertAt(int iIndex)  //The new element will be at iIndex
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
	return (void*)pSource;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertAt(void* pvData, int iIndex)
{
	void* ptr;

	ptr = InsertAt(iIndex);
	memcpy(ptr, pvData, miElementSize);
	return ptr;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize)
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
void CArrayBlock::RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize)
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
		pDest = (void*)RemapSinglePointer(pSource, iDataSize);
		memmove_fast(pSource, pDest, iDataSize * (miUsedElements - iIndex));
	}
	else
	{
		//If the order is unimportant then just move the last element to the empty.
		pEnd = (void*)RemapSinglePointer(mpvArray, miUsedElements * iDataSize);
		memcpy_fast(pSource, pEnd, iDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::RemoveAtNoDeallocate(int iIndex, int bPreserveOrder)
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
void CArrayBlock::RemoveAt(int iIndex, int bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize)
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
void CArrayBlock::RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::RemoveAt(int* paiElementsToDelete, int iNumElementsToDelete, BOOL bPreserveOrder)
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
void CArrayBlock::RemoveTail(void)
{
	RemoveAt(miUsedElements - 1);
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
	mpcMalloc = NULL;
	mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Unuse(void)
{
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Zero(void)
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
BOOL CArrayBlock::Copy(CArrayBlock* pcTemplateArray)
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
void* CArrayBlock::PushCopy(void)
{
	void*	pDest;
	void*	pSource;

	if (miUsedElements >= 1)
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
void CArrayBlock::Push(void* pvElement)
{
	void* pv;
	pv = Add();
	memcpy(pv, pvElement, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Pop(void* pvElement)
{
	void*	pvSource;

	if (miUsedElements > 0)
	{
		pvSource = Get(miUsedElements - 1);
		memcpy(pvElement, pvSource, miElementSize);
		RemoveTail();
	}
	else
	{
		memset(pvElement, 0, miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Pop(void)
{
	RemoveTail();
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
	return Get(miUsedElements-1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::ByteSize()
{
	return miUsedElements * miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::GrowByChunk(void)
{
	miUsedElements += miChunkSize;
	SetArraySize(miNumElements + miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::AddIfUnique(void* pData)
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
int CArrayBlock::Find(void* pData)
{
	int		i;
	void*		pIndex;

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
int CArrayBlock::Contains(void* pData)
{
	return Find(pData) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize)
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
int CArrayBlock::FindWithKey(void* pData, int iKeyOffset, int iKeySize)
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
int	CArrayBlock::FindWithIntKey(int iKey)
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
int	CArrayBlock::FindWithIntKey(int iKey, int iKeyOffset)
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
int CArrayBlock::GrowToNumElements(int iNumElements)
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
void* CArrayBlock::GrowToAtLeastNumElements(int iNumElements, BOOL bClear, unsigned char iClear)
{
	int	iOldUsedElements;
	void*	pvStart;

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
int CArrayBlock::GrowByNumElements(int iNumElements)
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
void CArrayBlock::SetUsedElements(int iUsedElements)
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
BOOL CArrayBlock::Equals(CArrayBlock* pcTemplateArray)
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
void CArrayBlock::Swap(int iIndex1, int iIndex2)
{
	void*	pvTemp;
	void*	pElement1;
	void*	pElement2;

	//Only allocate memory if we don't already have temporary memory assigned.
	if (miUsedElements < miNumElements)
	{
		pvTemp = Get(miUsedElements);
	}
	else
	{
		pvTemp = (void*)Malloc(miElementSize);
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
void CArrayBlock::QuickSort(int(* Func)(const void*, const void*))
{
	qsort((void*)mpvArray, miUsedElements, miElementSize, Func);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::BubbleSort(int(* Func)(const void*, const void*))
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
int CArrayBlock::InsertIntoSorted(int(* Func)(const void*, const void*), void* pvElement, BOOL bOverwriteExisting)
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
BOOL CArrayBlock::FindInSorted(void* pData, int(* Func)(const void*, const void*), int* piIndex)
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
BOOL CArrayBlock::BinarySearch(void* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;

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
void CArrayBlock::Reverse(void)
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
void* CArrayBlock::CopyArrayInto(CArrayBlock* pcTemplateArray, int iIndex)
{
	void*	pv;

	pv = Get(iIndex);
	memcpy(pv, pcTemplateArray->mpvArray, miElementSize * pcTemplateArray->miUsedElements);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::CopyBlockInto(void* paElements, int iLength, int iIndex)
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
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(pcTemplateArray->miUsedElements);
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
void* CArrayBlock::InsertArrayAt(CArrayBlock* pcTemplateArray, int iIndex)
{
	InsertNumAt(pcTemplateArray->miUsedElements, iIndex);
	return CopyArrayInto(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertNumAt(int iNumElements, int iIndex)
{
	void*	pvFrom;
	void*	pvTo;
	int		iNumToMove;

	iNumToMove = miUsedElements - iIndex;
	GrowByNumElements(iNumElements);

	pvFrom = (void*)Get(iIndex);
	pvTo = (void*)Get(iIndex + iNumElements);
	memmove(pvTo, pvFrom, iNumToMove * miElementSize);
	return (void*)pvFrom;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertBlockAfterEnd(void* paElements, int iLength)
{
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(iLength);
	return CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertBlockBeforeStart(void* paElements, int iLength)
{
	InsertNumAt(iLength, 0);
	return CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::InsertBlockAt(void* paElements, int iLength, int iIndex)
{
	InsertNumAt(iLength, iIndex);
	return CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::Get(int iIndex)
{
	return (void*)RemapSinglePointer(mpvArray, miElementSize * iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::ElementSize(void)
{
	return miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::AllocatedElements(void)
{
	return miNumElements;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::IsEmpty(void)
{
	return miUsedElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::IsNotEmpty(void)
{
	return miUsedElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::GetAdjustedIndex(int iIndex)
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
int CArrayBlock::GetIndex(void* pvElement)
{
	size_t iIndex;
	size_t iBase;
	size_t iDifference;

	iBase = (size_t) mpvArray;
	iIndex = (size_t) pvElement;
	iDifference = iIndex - iBase;

	//Make sure the element is correctly aligned.
	if (iDifference % miElementSize != 0)
	{
		return -1;
	}

	//Make sure the element lies within the array.
	iIndex = iDifference / miElementSize;
	if ((iIndex < 0) || (iIndex >= (size_t)miUsedElements))
	{
		return -1;
	}

	return (int)iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::BatchRemoveElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	void*		pcFirst;
	int		iTotalStride;
	void*		pcSource;
	void*		pcDest;
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
	pcDest = (void*)RemapSinglePointer(pcFirst, iDestIndex);
	pcSource = (void*)RemapSinglePointer(pcFirst, iSourceIndex);

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
void CArrayBlock::BatchInsertElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
{
	int		i;
	void*		pcFirst;
	int		iTotalStride;
	void*		pcSource;
	void*		pcDest;
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
	pcDest = (void*)RemapSinglePointer(pcFirst, iDestIndex);
	pcSource = (void*)RemapSinglePointer(pcFirst, iSourceIndex);

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
void* CArrayBlock::GetData(void)
{
	return mpvArray;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::ChunkSize(void)
{
	return miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::WriteHeader(CFileWriter* pcFileWriter)
{
	SArrayTemplateHeader	sHeader;

	sHeader.miChunkSize = miChunkSize;
	sHeader.miElementSize = miElementSize;
	sHeader.miUsedElements = miUsedElements;

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
BOOL CArrayBlock::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
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
BOOL CArrayBlock::Write(CFileWriter* pcFileWriter)
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
BOOL CArrayBlock::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	SArrayTemplateHeader	sHeader;

	if (!pcFileReader->ReadData(&sHeader, sizeof(SArrayTemplateHeader))) 
	{ 
		return FALSE; 
	}

	miChunkSize = sHeader.miChunkSize;
	miElementSize = sHeader.miElementSize;
	miNumElements = 0;
	miUsedElements = 0;
	mpvArray = NULL;
	mpcMalloc = pcMalloc;

	if (sHeader.miUsedElements != 0)
	{
		SetUsedElements(sHeader.miUsedElements);
		return mpvArray != NULL;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::ReadAllocatorAndHeader(CFileReader* pcFileReader)
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
BOOL CArrayBlock::Read(CFileReader* pcFileReader)
{
	BOOL			bResult;

	bResult = ReadAllocatorAndHeader(pcFileReader);
	if (!bResult)
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::FakeSetUsedElements(int iUsedElements)
{
	miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::SetAllocateSize(int iSize)
{
	miChunkSize = iSize;
}

