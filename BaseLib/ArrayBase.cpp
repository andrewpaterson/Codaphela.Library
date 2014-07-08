#include "ArrayBase.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Init(int iElementSize)
{
	this->miElementSize = iElementSize;
	this->miNumElements = 0;
	this->miUsedElements = 0;
	this->miChunkSize = 1;
	this->mpvArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Init(int iElementSize, int iChunkSize)
{
	Init(iElementSize);
	this->miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Allocate(int iElementSize, int iNumElements)
{
	Init(iElementSize, iNumElements);
	this->GrowByChunk();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Allocate(int iElementSize, int iChunkSize, int iNumElements)
{
	this->miElementSize = iElementSize;
	this->miChunkSize = iChunkSize;
	this->mpvArray = NULL;
	this->SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Fake(int iElementSize, void* pvData, int iNum, int iChunkSize)
{
	this->mpvArray = pvData;
	this->miElementSize = iElementSize;
	this->miNumElements = iChunkSize;
	this->miUsedElements = iNum;
	this->miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InitFromHeader(void)
{
	//This function assumes that the value of mpvArray was invalid.
	mpvArray = (void*)Malloc(miElementSize * miNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Init(SArrayTemplateHeader* psHeader)
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
void CArrayBase::Init(CArrayBase* pcTemplateArray)
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
void CArrayBase::ReInit(int iChunkSize)
{
	Kill();
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
void* CArrayBase::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::SafeGet(int iIndex)
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
BOOL CArrayBase::SafeSet(int iIndex, void* pvData)
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
void CArrayBase::SetArraySize(int iNumElements)
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
void* CArrayBase::Add(void)
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
void* CArrayBase::AddGetIndex(int* piIndex)
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
void* CArrayBase::Add(void* pvData)
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
void CArrayBase::Set(int iIndex, void* pvData)
{
	void*	pvTemp;

	pvTemp = Get(iIndex);
	memcpy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::InsertAt(int iIndex)  //The new element will be at iIndex
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
void* CArrayBase::InsertAt(void* pvData, int iIndex)
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
void CArrayBase::PrivateRemoveAt(int iIndex, BOOL bPreserveOrder, int iDataSize)
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
void CArrayBase::RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize)
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
void CArrayBase::RemoveAtNoDeallocate(int iIndex, int bPreserveOrder)
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
void CArrayBase::RemoveAt(int iIndex, int bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::PrivateRemoveRange(int iStartIndex, int iEndIndexExclusive, int bPreserveOrder, int iDataSize)
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
void CArrayBase::RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::RemoveAt(int* paiElementsToDelete, int iNumElementsToDelete, BOOL bPreserveOrder)
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
void CArrayBase::RemoveTail(void)
{
	RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Finalise(void)
{
	miChunkSize = 1;
	SetArraySize(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Kill(void)
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
void CArrayBase::Unuse(void)
{
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Zero(void)
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
BOOL CArrayBase::Copy(CArrayBase* pcTemplateArray)
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
void CArrayBase::PushCopy(void)
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
void CArrayBase::Push(void* pvElement)
{
	void* pv;
	pv = Add();
	memcpy(pv, pvElement, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::Pop(void* pvElement)
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
void CArrayBase::Pop(void)
{
	RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::Tail(void)
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
int CArrayBase::ByteSize()
{
	return miUsedElements * miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::GrowByChunk(void)
{
	miUsedElements += miChunkSize;
	SetArraySize(miNumElements + miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::AddIfUnique(void* pData)
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
int CArrayBase::Find(void* pData)
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
int CArrayBase::Contains(void* pData)
{
	return Find(pData) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::AddIfUniqueKey(void* pData, int iKeyOffset, int iKeySize)
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
int CArrayBase::FindWithKey(void* pData, int iKeyOffset, int iKeySize)
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
int	CArrayBase::FindWithIntKey(int iKey)
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
int	CArrayBase::FindWithIntKey(int iKey, int iKeyOffset)
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
int CArrayBase::GrowToNumElements(int iNumElements)
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
void* CArrayBase::GrowToAtLeastNumElements(int iNumElements, BOOL bClear, int iClear)
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
int CArrayBase::GrowByNumElements(int iNumElements)
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
void CArrayBase::SetUsedElements(int iUsedElements)
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
BOOL CArrayBase::Equals(CArrayBase* pcTemplateArray)
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
void CArrayBase::Swap(int iIndex1, int iIndex2)
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
void CArrayBase::QuickSort(int(* Func)(const void*, const void*))
{
	qsort((void*)mpvArray, miUsedElements, miElementSize, Func);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::BubbleSort(int(* Func)(const void*, const void*))
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
int CArrayBase::InsertIntoSorted(int(* Func)(const void*, const void*), void* pvElement, BOOL bOverwriteExisting)
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
BOOL CArrayBase::FindInSorted(void* pData, int(* Func)(const void*, const void*), int* piIndex)
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
BOOL CArrayBase::BinarySearch(void* pData, int iLeft, int iRight, int(* Func)(const void*, const void*), int* piIndex)
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
void CArrayBase::Reverse(void)
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
void CArrayBase::CopyArrayInto(CArrayBase* pcTemplateArray, int iIndex)
{
	memcpy(Get(iIndex), pcTemplateArray->mpvArray, miElementSize * pcTemplateArray->miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::CopyBlockInto(void* paElements, int iLength, int iIndex)
{
	memcpy(Get(iIndex), paElements, miElementSize * iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InsertArrayAfterEnd(CArrayBase* pcTemplateArray)
{
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(pcTemplateArray->miUsedElements);
	CopyArrayInto(pcTemplateArray, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InsertArrayBeforeStart(CArrayBase* pcTemplateArray)
{
	InsertNumElementsAt(pcTemplateArray->miUsedElements, 0);
	CopyArrayInto(pcTemplateArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InsertArrayAt(CArrayBase* pcTemplateArray, int iIndex)
{
	InsertNumElementsAt(pcTemplateArray->miUsedElements, iIndex);
	CopyArrayInto(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::InsertNumElementsAt(int iNumElements, int iIndex)
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
void CArrayBase::InsertBlockAfterEnd(void* paElements, int iLength)
{
	int		iArrayIndex;

	iArrayIndex = GrowByNumElements(iLength);
	CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InsertBlockBeforeStart(void* paElements, int iLength)
{
	InsertNumElementsAt(iLength, 0);
	CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::InsertBlockAt(void* paElements, int iLength, int iIndex)
{
	InsertNumElementsAt(iLength, iIndex);
	CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::Get(int iIndex)
{
	return (void*)RemapSinglePointer(mpvArray, miElementSize * iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::ElementSize(void)
{
	return miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::AllocatedElements(void)
{
	return miNumElements;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBase::IsEmpty(void)
{
	return miUsedElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBase::IsNotEmpty(void)
{
	return miUsedElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::GetAdjustedIndex(int iIndex)
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
int CArrayBase::GetIndex(void* pvElement)
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
void CArrayBase::BatchRemoveElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
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
void CArrayBase::BatchInsertElements(int iFirstIndex, int iNumInBatch, int iNumBatches, int iStrideToNextBatch)
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
void CArrayBase::GetHeader(SArrayTemplateHeader* psHeader)
{
	psHeader->miElementSize = miElementSize;
	psHeader->miUsedElements = miUsedElements;
	psHeader->miChunkSize = miChunkSize;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBase::GetData(void)
{
	return mpvArray;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBase::ChunkSize(void)
{
	return miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBase::WriteArrayTemplate(CFileWriter* pcFileWriter)
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
BOOL CArrayBase::WriteArrayTemplateHeader(CFileWriter* pcFileWriter)
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
BOOL CArrayBase::ReadArrayTemplate(CFileReader* pcFileReader)
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
BOOL CArrayBase::ReadArrayTemplateHeader(CFileReader* pcFileReader)
{
	SArrayTemplateHeader	sHeader;

	if (!pcFileReader->ReadData(&sHeader, sizeof(SArrayTemplateHeader)))
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
BOOL CArrayBase::WriteArrayUnknown(CFileWriter* pcFileWriter)
{
	if (!pcFileWriter->WriteData(this, sizeof(CArrayBase))) 
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
BOOL CArrayBase::ReadArrayUnknown(CFileReader* pcFileReader)
{
	if (!pcFileReader->ReadData(this, sizeof(CArrayBase))) 
	{ 
		return FALSE; 
	}

	if (NumElements() != 0)
	{
		InitFromHeader();
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
void CArrayBase::FakeSetUsedElements(int iUsedElements)
{
	miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBase::SetAllocateSize(int iSize)
{
	miChunkSize = iSize;
}

