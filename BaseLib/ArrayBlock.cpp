#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "ArraySizer.h"
#include "StdRandom.h"
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(int iElementSize)
{
	Init(&gcSystemAllocator, iElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(CMallocator* pcMalloc, int iElementSize)
{
	Init(pcMalloc, iElementSize, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::Init(CMallocator* pcMalloc, int iElementSize, int iChunkSize)
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
void CArrayBlock::Fake(int iElementSize, void* pvData, int iNum, int iChunkSize)
{
	CMalloc::Kill();
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
	int				iChunkSize;

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
	//Fix for virtual in CArrayTemplate
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
int CArrayBlock::SetUsedElements(int iUsedElements)
{
	int	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetArraySize(iUsedElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::RemoveAtNoDeallocate(int iIndex)
{
	//This is only used by CConvexHullGenerator.RemoveDiscontiguousTriangles/
	//It should be removed.
	return RemoveAtNoDeallocate(iIndex, FALSE, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::SetChunkSize(int iChunkSize)
{
	if (iChunkSize < miChunkSize)
	{
		if (miChunkSize % iChunkSize == 0)
		{
			miChunkSize = iChunkSize;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (iChunkSize % miChunkSize == 0)
		{
			miChunkSize = iChunkSize;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::SetArraySize(int iNeededElements)
{
	void*	ptr;
	int		iNumElements;
	int		iChunkSize;

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
			ptr = (void*)Realloc(mpvArray, miElementSize * iNumElements);
			mpvArray = ptr;
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
void* CArrayBlock::AddGetIndex(int* piIndex)
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
int CArrayBlock::AddGetIndex(void* pvData)
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
int CArrayBlock::RemoveAtNoDeallocate(int iIndex, BOOL bPreserveOrder, int iDataSize)
{
	void*	pSource;
	void*	pDest;
	void*	pEnd;
	int		iUsedElements;

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
		memmove_fast(pSource, pDest, iDataSize * (iUsedElements - iIndex));
	}
	else
	{
		//If the order is unimportant then just move the last element to the empty.
		pEnd = (void*)RemapSinglePointer(mpvArray, iUsedElements * iDataSize);
		memcpy_fast(pSource, pEnd, iDataSize);
	}
	return iUsedElements;
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
BOOL CArrayBlock::RemoveTail(void)
{
	if (miUsedElements > 0)
	{
		RemoveAt(miUsedElements - 1);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::RemoveFirst(void)
{
	if (miUsedElements > 0)
	{
		RemoveAt(0);
		return TRUE;
	}
	return FALSE;
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
void* CArrayBlock::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::Pop(void* pvDest)
{
	void*	pvSource;

	if (miUsedElements > 0)
	{
		pvSource = Get(miUsedElements - 1);
		memcpy(pvDest, pvSource, miElementSize);
		RemoveAt(miUsedElements - 1);
		return TRUE;
	}
	else
	{
		memset(pvDest, 0, miElementSize);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::Pop(void)
{
	return RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::PopFirst(void* pvDest)
{
	void* pvSource;

	if (miUsedElements > 0)
	{
		pvSource = Get(0);
		memcpy(pvDest, pvSource, miElementSize);
		RemoveAt(0);
		return TRUE;
	}
	else
	{
		memset(pvDest, 0, miElementSize);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::PopFirst(void)
{
	return RemoveFirst();
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
int CArrayBlock::AddIfUnique(void* pvData)
{
	int iElementNum;

	iElementNum = Find(pvData);
	if (iElementNum == -1)
	{
		Add(pvData);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::Find(void* pvData)
{
	int		i;
	void*	pv;

	for (i = 0; i < miUsedElements; i++)
	{
		pv = Get(i);
		if (memcmp(pv, pvData, miElementSize) == 0)
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
int CArrayBlock::Contains(void* pvData)
{
	return Find(pvData) >= 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayBlock::AddIfUniqueKey(void* pvData, int iKeyOffset, int iKeySize)
{
	int	iElementNum;

	iElementNum = FindWithKey(pvData, iKeyOffset, iKeySize);
	if (iElementNum == -1)
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
int CArrayBlock::FindWithKey(void* pvData, int iKeyOffset, int iKeySize)
{
	int		i;
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
int CArrayBlock::Resize(int iNumElements)
{
	return SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayBlock::GrowToAtLeastNumElements(int iNumElements, BOOL bClear, unsigned char iClear)
{
	int		iOldUsedElements;
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
int CArrayBlock::AddNum(int iNumElements)
{
	return SetUsedElements(iNumElements + miUsedElements);
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
void CArrayBlock::QuickSort(DataCompare fCompare)
{
	qsort((void*)mpvArray, miUsedElements, miElementSize, fCompare);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::BubbleSort(DataCompare fCompare)
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
			iResult = fCompare(Get(iIndex), Get(iIndex+1));
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
int CArrayBlock::InsertIntoSorted(DataCompare fCompare, void* pvData, BOOL bOverwriteExisting)
{
	int		iIndex;
	BOOL	bExists;

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
BOOL CArrayBlock::FindInSorted(void* pvData, DataCompare fCompare, int* piIndex)
{
	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return FALSE;
	}
	return BinarySearch(pvData, 0, miUsedElements - 1, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayBlock::BinarySearch(void* pvData, int iLeft, int iRight, DataCompare fCompare, int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;

	iResultMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = Get(iMiddle);
		iResultMiddle = fCompare(pvData, pvMiddle);
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
void CArrayBlock::Shuffle(CRandom* pcRandom)
{
	int			i;
	int			iNumElements;
	CRandom		cRandom;
	BOOL		bKillRandom;
	int			iIndex;

	bKillRandom = FALSE;
	if (pcRandom == NULL)
	{
		cRandom.Init();
		bKillRandom = TRUE;
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
void* CArrayBlock::CopyArrayInto(CArrayBlock* pcTemplateArray, int iIndex)
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

	if (iNumElements <= 0)
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
void* CArrayBlock::InsertBlockAfterEnd(void* paElements, int iLength)
{
	int		iArrayIndex;

	iArrayIndex = AddNum(iLength);
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
int CArrayBlock::GetIndex(void* pvData)
{
	size_t tIndex;
	size_t tBase;
	size_t tDifference;

	tBase = (size_t) mpvArray;
	tIndex = (size_t) pvData;
	tDifference = tIndex - tBase;

	//Make sure the element is correctly aligned.
	if (tDifference % miElementSize != 0)
	{
		return -1;
	}

	//Make sure the element lies within the array.
	tIndex = tDifference / miElementSize;
	if ((tIndex < 0) || (tIndex >= (size_t)miUsedElements))
	{
		return -1;
	}

	return (int)tIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::RemoveBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip)
{
	int		i;
	void*	pcFirst;
	void*	pcSource;
	void*	pcDest;
	int		iDest;
	int		iSource;
	int		iRemaining;
	int		iSkipStride;

	pcFirst = Get(iFirstElementPos);

	iSkipStride = iSkip * miElementSize;
	for (i = 0; i <= iNumBatches - 2; i++)
	{
		iDest = iFirstElementPos + iSkip * i;
		iSource = iFirstElementPos + (iSkip + iNumInBatch) * i + iNumInBatch;

		pcDest = Get(iDest);
		pcSource = Get(iSource);

		memcpy(pcDest, pcSource, iSkipStride);
	}

	iDest = iFirstElementPos + (iSkip + iNumInBatch) * iNumBatches - iSkip;
	iRemaining = miUsedElements - iDest;

	if (iRemaining > 0)
	{
		iSource = iFirstElementPos + iSkip * (iNumBatches - 1);
		pcDest = Get(iDest);
		pcSource = Get(iSource);

		memcpy(pcSource, pcDest, iRemaining * miElementSize);
		iRemaining = 0;
	}
	else
	{
		iRemaining = -iRemaining;
	}


	SetUsedElements(miUsedElements - iNumInBatch * iNumBatches + iRemaining);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayBlock::InsertBatch(int iFirstElementPos, int iNumInBatch, int iNumBatches, int iSkip)
{
	int		i;
	void*	pcFirst;
	int		iTotalStride;
	void*	pcSource;
	void*	pcDest;
	int		iDest;
	int		iSource;
	int		iOldNumElements;
	int		iRemaining;
	int		iStride;

	iOldNumElements = AddNum(iNumInBatch * iNumBatches);

	pcFirst = Get(iFirstElementPos);

	i = iNumBatches - 1;

	iDest = (iNumInBatch + iSkip) * (i + 1) - iSkip;
	iSource = iSkip * i;
	pcDest = RemapSinglePointer(pcFirst, iDest * miElementSize);
	pcSource = RemapSinglePointer(pcFirst, iSource * miElementSize);

	iRemaining = (iOldNumElements - iSource) - iFirstElementPos;

	if (iRemaining > 0)
	{
		memcpy(pcDest, pcSource, iRemaining * miElementSize);
	}

	iStride = iSkip * miElementSize;
	iTotalStride = (iNumInBatch + iSkip) * miElementSize;
	if (iStride == 1)
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy_fast_1byte(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 2)
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy_fast_2bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 4)
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy_fast_4bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 8)
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy_fast_8bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else if (iStride == 12)
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy_fast_12bytes(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i));
		}
	}
	else
	{
		for (i = iNumBatches - 2; i >= 0; i--)
		{
			memcpy(RemapSinglePointer(pcFirst, iTotalStride * (i + 1) - iStride), RemapSinglePointer(pcFirst, iStride * i), iStride);
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

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
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
	CMalloc::Init(pcMalloc);

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

	pcMalloc = gcMallocators.Read(pcFileReader);
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
	//Do not call .Init() before Read().

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

