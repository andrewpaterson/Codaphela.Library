#include "BubbleSort.h"
#include "QuickSort.h"
#include "TimSort.h"
#include "SystemAllocator.h"
#include "NullAllocator.h"
#include "GlobalMemory.h"
#include "ArraySizer.h"
#include "StdRandom.h"
#include "ErrorHandler.h"
#include "ArrayBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::_Init(void)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Init(Count iElementSize)
{
	Init(&gcSystemAllocator, iElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Init(CMallocator* pcMalloc, Count iElementSize)
{
	Init(pcMalloc, iElementSize, 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Init(CMallocator* pcMalloc, Count iElementSize, Count iChunkSize)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Init(CMallocator* pcMalloc, Count iElementSize, void* pvData, Count uiNumData)
{
	Init(pcMalloc, iElementSize);
	Resize(uiNumData);
	SIZE::Copy(mpvArray, pvData, iElementSize * uiNumData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Fake(Count iElementSize, void* pvData, Count iNum, Count iChunkSize)
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
// Initialises this with pcSource's elements, without copying them.		//
// pcSource is left an empty array that can still be used, and must		//
// still be killed.  Like an Init, this frees nothing this already had.	//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::TakeFrom(CArrayBlockBase<SIZE>* pcSource)
{
	Init(pcSource->mpcMalloc, pcSource->miElementSize, pcSource->miChunkSize);
	mpvArray = pcSource->mpvArray;
	miNumElements = pcSource->miNumElements;
	miUsedElements = pcSource->miUsedElements;

	pcSource->mpvArray = NULL;
	pcSource->miNumElements = 0;
	pcSource->miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::ReInit(void)
{
	CMallocator*	pcMalloc;
	Count			iChunkSize;

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
template<class SIZE>
void* CArrayBlockBase<SIZE>::SafeGet(Count iIndex)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::SafeSet(Count iIndex, void* pvData)
{
	Count	iOldLength;


	if (iIndex < miUsedElements)
	{
		Set(iIndex, pvData);
		return true;
	}
	else
	{
		iOldLength = NumElements();
		if (!SetArraySize(iIndex + 1))
		{
			return false;
		}
		SIZE::Set(Get(iOldLength), 0, (Count)((iIndex - iOldLength) * miElementSize));
		Set(iIndex, pvData);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::SetUsedElements(Count iUsedElements)
{
	Count	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	SetArraySize(iUsedElements);
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::SetChunkSize(Count iChunkSize)
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
//
// Room for iNeededElements, which become the used elements.  False if it
// needs more memory and can't have it (or, where size is 16 bits, more
// than MAX_SIZE bytes): then nothing changes, the elements and the counts
// as they were.  A shrink whose realloc fails keeps the memory it has.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::SetArraySize(Count iNeededElements)
{
	void*	pv;
	Count	iNumElements;
	Count	iChunkSize;

	if (iNeededElements == 0)
	{
		if (mpvArray != NULL)
		{
			Free(mpvArray);
			mpvArray = NULL;
		}
		miNumElements = 0;
		miUsedElements = 0;
		return true;
	}

	iChunkSize = (Count)CalculateChunkSize(iNeededElements, miChunkSize);
	iNumElements = (Count)CalculateArraySize(iNeededElements, iChunkSize);
	if (SIZE::CheckSize && ((iNumElements < iNeededElements) || ((miElementSize != 0) && (iNumElements > SIZE::Max / miElementSize))))
	{
		Overflowed();
		return false;
	}

	if (miNumElements != iNumElements)
	{
		pv = (void*)Realloc(mpvArray, (Count)(miElementSize * iNumElements));
		if (pv == NULL)
		{
			if (iNeededElements > miNumElements)
			{
				return false;
			}
			miUsedElements = iNeededElements;
			return true;
		}
		mpvArray = pv;
	}

	miUsedElements = iNeededElements;
	miChunkSize = iChunkSize;
	miNumElements = iNumElements;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The array would need more than MAX_SIZE bytes.  It is emptied, as if out of memory; debug builds say so,
// so it can be made a large array.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Overflowed(void)
{
#ifdef _DEBUG
	gcUserError.Set("An array counted with size needs more than MAX_SIZE bytes.  Make it a large array (counted with lsize).");
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::AddNum(Count iNumElements)
{
	Count	iOldUsedElements;

	if (SIZE::CheckSize && (iNumElements > SIZE::Max - miUsedElements))
	{
		Overflowed();
		return SIZE::NotFound;
	}

	//Within the elements allocated nothing is reallocated (as Add).
	if (miUsedElements + iNumElements <= miNumElements)
	{
		iOldUsedElements = miUsedElements;
		miUsedElements += iNumElements;
		return iOldUsedElements;
	}
	iOldUsedElements = miUsedElements;
	if (!SetArraySize(miUsedElements + iNumElements))
	{
		return SIZE::NotFound;
	}
	return iOldUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::Add(void)
{
	if (SIZE::CheckSize && (miUsedElements == SIZE::Max))
	{
		Overflowed();
		return NULL;
	}
	if (miUsedElements + 1 > miNumElements)
	{
		if (!SetArraySize(miUsedElements + 1))
		{
			return NULL;
		}
	}
	else
	{
		miUsedElements++;
	}
	return Get(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::AddGetIndex(Count* piIndex)
{
	void* pv;

	pv = Add();
	(*piIndex) = pv ? miUsedElements - 1 : SIZE::NotFound;
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::AddExistingGetIndex(void* pvData)
{
	void* pv;

	pv = Add();
	SIZE::Copy(pv, pvData, miElementSize);
	return miUsedElements - 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::Add(void* pvData)
{
	void* pv;

	pv = Add();
	if (pv)
	{
		SIZE::Copy(pv, pvData, miElementSize);
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Set(Count iIndex, void* pvData)
{
	void*	pvTemp;

	pvTemp = Get(iIndex);
	SIZE::Copy(pvTemp, pvData, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertAt(Count iIndex)
{
	void*	pSource;
	void*	pDest;

	//The new element will be at iIndex
	if (iIndex > miNumElements)
	{
		return NULL;
	}

	//This assumes that iIndex is within the array (or the last element).
	if (SIZE::CheckSize && (miUsedElements == SIZE::Max))
	{
		Overflowed();
		return NULL;
	}
	if (miUsedElements + 1 > miNumElements)
	{
		if (!SetArraySize(miUsedElements + 1))
		{
			return NULL;
		}
	}
	else
	{
		miUsedElements++;
	}
	pSource = Get(iIndex);
	pDest = RemapSinglePointer(pSource, miElementSize);

	SIZE::Move(pDest, pSource, (Count)(miElementSize * (miUsedElements - 1 - iIndex)));
	return (void*)pSource;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertAt(void* pvData, Count iIndex)
{
	void* pv;

	pv = InsertAt(iIndex);
	if (pv)
	{
		SIZE::Copy(pv, pvData, miElementSize);
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::PrivateRemoveAt(Count iIndex, bool bPreserveOrder, Count iDataSize)
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
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::RemoveAtNoDeallocate(Count iIndex, bool bPreserveOrder, Count iDataSize)
{
	void*	pSource;
	void*	pDest;
	void*	pEnd;
	Count	iUsedElements;

	iUsedElements = miUsedElements;
	iUsedElements--;
	if (iIndex == iUsedElements)
	{
		return iUsedElements;
	}

	pSource = RemapSinglePointer(mpvArray, (Count)(iIndex * iDataSize));

	if (bPreserveOrder)
	{
		//If the order of elements is to be preserved then move all the elements back one.
		pDest = (void*)RemapSinglePointer(pSource, iDataSize);
		SIZE::Move(pSource, pDest, (Count)(iDataSize * (iUsedElements - iIndex)));
	}
	else
	{
		//If the order is unimportant then just move the last element to the empty.
		pEnd = (void*)RemapSinglePointer(mpvArray, (Count)(iUsedElements * iDataSize));
		SIZE::Copy(pSource, pEnd, iDataSize);
	}
	return iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::RemoveAt(Count iIndex, bool bPreserveOrder)
{
	PrivateRemoveAt(iIndex, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::PrivateRemoveRange(Count iStartIndex, Count iEndIndexExclusive, bool bPreserveOrder, Count iDataSize)
{
	Count	iNumTrailing;
	Count	iNumToRemove;
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
			SIZE::Move(pvStart, pvEnd, iNumTrailing*iDataSize);
		}
		else
		{
			pv = Get(miUsedElements - iNumToRemove);
			SIZE::Move(pvStart, pv, iNumToRemove*iDataSize);
		}
		SetUsedElements(miUsedElements - iNumToRemove);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::RemoveRange(Count iStartIndex, Count iEndIndexExclusive, bool bPreserveOrder)
{
	PrivateRemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::RemoveAt(Count* paiElementsToDelete, Count iNumElementsToDelete, bool bPreserveOrder)
{
	Count	i;
	Count	iIndex;
	Count	iFirstElementToMove;
	Count	iEndElementToMove;
	Count	iMoved;
	Count	iDestElement;
	Count	iDestOffset;
	Count	iNumDeleted;

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
					SIZE::Move(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);
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
			SIZE::Move(Get(iDestElement - iDestOffset), Get(iFirstElementToMove), iMoved * miElementSize);

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
template<class SIZE>
bool CArrayBlockBase<SIZE>::RemoveTail(void)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::RemoveFirst(void)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Finalise(void)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Kill(void)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Zero(void)
{
	if (miUsedElements != 0)
	{
		SIZE::Set(mpvArray, 0, miUsedElements * miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::One(void)
{
	if (miUsedElements != 0)
	{
		SIZE::Set(mpvArray, 0xff, miUsedElements * miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Copy(CArrayBlockBase<SIZE>* pcArray)
{
	//Assumes the array is initialised.
	if (pcArray->miNumElements != miNumElements)
	{
		if (!SetArraySize(pcArray->miUsedElements))
		{
			return;
		}
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
template<class SIZE>
void* CArrayBlockBase<SIZE>::PushCopy(void)
{
	void*	pDest;
	void*	pSource;

	if (miUsedElements > 0)
	{
		pDest = Add();
		if (pDest == NULL)
		{
			return NULL;
		}
		pSource = Get(miUsedElements - 2);
		SIZE::Copy(pDest, pSource, miElementSize);
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Push(void* pvData)
{
	void* pv;

	pv = Add();
	if (pv)
	{
		SIZE::Copy(pv, pvData, miElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::Pop(void* pvDest)
{
	void*	pvSource;

	if (miUsedElements != 0)
	{
		pvSource = Get(miUsedElements - 1);
		SIZE::Copy(pvDest, pvSource, miElementSize);
		RemoveAt(miUsedElements - 1);
		return true;
	}
	else
	{
		SIZE::Set(pvDest, 0, miElementSize);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::PopFirst(void* pvDest)
{
	void* pvSource;

	if (miUsedElements != 0)
	{
		pvSource = Get(0);
		SIZE::Copy(pvDest, pvSource, miElementSize);
		RemoveAt(0);
		return true;
	}
	else
	{
		SIZE::Set(pvDest, 0, miElementSize);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::Tail(void)
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
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::AddIfUnique(void* pvData)
{
	Count iElementNum;

	iElementNum = Find(pvData);
	if (iElementNum == SIZE::NotFound)
	{
		Add(pvData);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::Find(void* pvData)
{
	Count	i;
	void*	pv;

	for (i = 0; i < miUsedElements; i++)
	{
		pv = Get(i);
		if (SIZE::Compare(pv, pvData, miElementSize) == 0)
		{
			return i;
		}
	}
	return SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::AddIfUniqueKey(void* pvData, Count iKeyOffset, Count iKeySize)
{
	Count	iElementNum;

	iElementNum = FindWithKey(pvData, iKeyOffset, iKeySize);
	if (iElementNum == SIZE::NotFound)
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
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::FindWithKey(void* pvData, Count iKeyOffset, Count iKeySize)
{
	Count	i;
	void*	pIndex;
	void*	pTest;

	pIndex = RemapSinglePointer(mpvArray, iKeyOffset);
	pTest = RemapSinglePointer(pvData, iKeyOffset);

	for (i = 0; i < miUsedElements; i++)
	{
		if (SIZE::Compare(pIndex, pTest, iKeySize) == 0)
		{
			return i;
		}
		pIndex = RemapSinglePointer(pIndex, miElementSize);
	}
	return SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::FindWithIntKey(int iKey)
{
	Count	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::FindWithIntKey(int iKey, Count iKeyOffset)
{
	Count	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = RemapSinglePointer(mpvArray, (Count)(i * miElementSize + iKeyOffset));
		if (*((int*)pIndex) == iKey)
		{
			return i;
		}
	}
	return SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// A size key (a channel number): not an int's width everywhere.		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::FindWithSizeKey(size iKey, Count iKeyOffset)
{
	Count	i;
	void*	pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = RemapSinglePointer(mpvArray, (Count)(i * miElementSize) + iKeyOffset);
		if (*((size*)pIndex) == iKey)
		{
			return i;
		}
	}
	return SIZE::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::Resize(Count iNumElements)
{
	return SetUsedElements(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::GrowToAtLeastNumElements(Count iNumElements, bool bClear, uint8 iClear)
{
	Count	iOldUsedElements;
	void*	pvStart;

	if (iNumElements == 0)
	{
		return NULL;
	}

	if (miUsedElements < iNumElements)
	{
		iOldUsedElements = miUsedElements;
		if (!SetArraySize(iNumElements))
		{
			return NULL;
		}
		if (bClear)
		{
			if (iOldUsedElements < iNumElements)
			{
				pvStart = Get(iOldUsedElements);
				SIZE::Set(pvStart, iClear, (Count)((iNumElements - iOldUsedElements) * miElementSize));
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::Equals(CArrayBlockBase<SIZE>* pcOtherArray)
{
	if (pcOtherArray->miUsedElements == miUsedElements)
	{
		if (SIZE::Compare(pcOtherArray->mpvArray, mpvArray, ByteSize()) == 0)
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Swap(Count iIndex1, Count iIndex2)
{
	void*	pElement1;
	void*	pElement2;

	pElement1 = Get(iIndex1);
	pElement2 = Get(iIndex2);

	SIZE::Swap(pElement1, pElement2, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::InsertIntoSorted(DataCompare fCompare, void* pvData, bool bOverwriteExisting)
{
	Count	iIndex;
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::FindInSorted(void* pvData, DataCompare fCompare, Count* piIndex)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::BinarySearch(void* pvData, Count iLeft, Count iRight, DataCompare fCompare, Count* piIndex)
{
	Count	iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;
	char*	pcArray;
	Count	iElementSize;

	//Held in locals because fCompare could change the array as far as the compiler knows.
	pcArray = (char*)mpvArray;
	iElementSize = miElementSize;

	iResultMiddle = 0;
	iMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = &pcArray[(Count)(iElementSize * iMiddle)];
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
template<class SIZE>
void CArrayBlockBase<SIZE>::Reverse(void)
{
	Count	i;
	Count	iCount;

	iCount = miUsedElements / 2;
	for (i = 0; i < iCount; i++)
	{
		Swap(i, miUsedElements - i - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::Shuffle(CRandom* pcRandom)
{
	Count	i;
	Count	iNumElements;
	CRandom	cRandom;
	bool	bKillRandom;
	Count	iIndex;

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
template<class SIZE>
void* CArrayBlockBase<SIZE>::CopyArrayInto(CArrayBlockBase<SIZE>* pcOtherArray, Count iIndex)
{
	void*	pv;

	//Check element sizes are the same.
	//The array is always set large enough.

	pv = Get(iIndex);
	SIZE::Copy(pv, pcOtherArray->mpvArray, miElementSize * pcOtherArray->miUsedElements);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::CopyBlockInto(void* paElements, Count iLength, Count iIndex)
{
	void*	pv;

	pv = Get(iIndex);
	SIZE::Copy(pv, paElements, miElementSize * iLength);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertArrayAfterEnd(CArrayBlockBase<SIZE>* pcOtherArray)
{
	Count	iArrayIndex;

	iArrayIndex = AddNum(pcOtherArray->miUsedElements);
	if (iArrayIndex == SIZE::NotFound)
	{
		return NULL;
	}
	return CopyArrayInto(pcOtherArray, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertArrayBeforeStart(CArrayBlockBase<SIZE>* pcOtherArray)
{
	if ((pcOtherArray->miUsedElements != 0) && (InsertNumAt(pcOtherArray->miUsedElements, 0) == NULL))
	{
		return NULL;
	}
	return CopyArrayInto(pcOtherArray, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertArrayAt(CArrayBlockBase<SIZE>* pcOtherArray, Count iIndex)
{
	if ((pcOtherArray->miUsedElements != 0) && (InsertNumAt(pcOtherArray->miUsedElements, iIndex) == NULL))
	{
		return NULL;
	}
	return CopyArrayInto(pcOtherArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertNumAt(Count iNumElements, Count iIndex)
{
	void*	pvFrom;
	void*	pvTo;
	Count	iNumToMove;

	if ((iNumElements == 0) || (iIndex > miNumElements))
	{
		return NULL;
	}

	iNumToMove = miUsedElements - iIndex;
	if (AddNum(iNumElements) == SIZE::NotFound)
	{
		return NULL;
	}

	pvFrom = (void*)Get(iIndex);
	pvTo = (void*)Get(iIndex + iNumElements);
	SIZE::Move(pvTo, pvFrom, iNumToMove * miElementSize);
	return (void*)pvFrom;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertBlockAfterEnd(void* paElements, Count iLength)
{
	Count	iArrayIndex;

	iArrayIndex = AddNum(iLength);
	if (iArrayIndex == SIZE::NotFound)
	{
		return NULL;
	}
	return CopyBlockInto(paElements, iLength, iArrayIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertBlockBeforeStart(void* paElements, Count iLength)
{
	if ((iLength != 0) && (InsertNumAt(iLength, 0) == NULL))
	{
		return NULL;
	}
	return CopyBlockInto(paElements, iLength, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::InsertBlockAt(void* paElements, Count iLength, Count iIndex)
{
	if ((iLength != 0) && (InsertNumAt(iLength, iIndex) == NULL))
	{
		return NULL;
	}
	return CopyBlockInto(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void* CArrayBlockBase<SIZE>::Get(Count iIndex)
{
	return (void*)RemapSinglePointer(mpvArray, (Count)(miElementSize * iIndex));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::GetAdjustedIndex(Count iIndex)
{
	if (iIndex == SIZE::NotFound)
	{
		return NumElements() - 1;
	}
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::GetIndex(void* pvData)
{
	ptr		tDifference;
	Count	iDifference;

	//Make sure the element lies within the array (whole pointers: a 16 bit size would wrap).
	if ((ptr)pvData < (ptr)mpvArray)
	{
		return SIZE::NotFound;
	}
	tDifference = (ptr)pvData - (ptr)mpvArray;
	if (tDifference >= (ptr)miUsedElements * miElementSize)
	{
		return SIZE::NotFound;
	}

	//Make sure the element is correctly aligned.
	iDifference = (Count)tDifference;
	if (iDifference % miElementSize != 0)
	{
		return SIZE::NotFound;
	}
	return iDifference / miElementSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::RemoveBatch(Count uiFirstElementPos, Count uiNumInBatch, Count uiNumBatches, Count uiSkip)
{
	Count	i;
	void*	pcSource;
	void*	pcDest;
	Count	uiDest;
	Count	uiSource;
	Count	uiCopyWidth;
	Count	uiSourceStride;
	Count	uiNewNumElements;

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

			SIZE::Move(pcDest, pcSource, uiCopyWidth);
		}
	}

	uiSource = uiFirstElementPos + (uiSkip + uiNumInBatch) * uiNumBatches - uiSkip;
	if (miUsedElements > uiSource)
	{
		uiDest = uiFirstElementPos + uiSkip * (uiNumBatches - 1);
		pcDest = Get(uiDest);
		pcSource = Get(uiSource);

		uiCopyWidth = (miUsedElements - uiSource) * miElementSize;
		SIZE::Move(pcDest, pcSource, uiCopyWidth);

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
template<class SIZE>
void CArrayBlockBase<SIZE>::RemoveAll(void)
{
	SetArraySize(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::Remove(void* pvData, bool bPreserveOrder)
{
	Count	iIndex;

	iIndex = GetIndex(pvData);
	if (iIndex != SIZE::NotFound)
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
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::RemoveArray(CArrayBlockBase<SIZE>* pcOtherArray, bool bPreserveOrder)
{
	Count	i;
	Count	iOtherNumElements;
	void*	pvOther;
	Count	iThisIndex;
	Count	iRemoved;

	iRemoved = 0;
	iOtherNumElements = pcOtherArray->NumElements();
	for (i = 0; i < iOtherNumElements; i++)
	{
		pvOther = pcOtherArray->Get(i);
		iThisIndex = Find(pvOther);
		if (iThisIndex != SIZE::NotFound)
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
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::RemoveArrayFromSorted(CArrayBlockBase<SIZE>* pcOtherArray, DataCompare fCompare)
{
	Count	i;
	Count	iOtherNumElements;
	void*	pvOther;
	Count	iThisIndex;
	bool	bFound;
	Count	iRemoved;

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
template<class SIZE>
void CArrayBlockBase<SIZE>::InsertBatch(Count iFirstElementPos, Count iNumInBatch, Count iNumBatches, Count iSkip)
{
	Count	i;
	void*	pcFirst;
	Count	iTotalStride;
	void*	pcSource;
	void*	pcDest;
	Count	iDest;
	Count	iSource;
	Count	iOldNumElements;
	Count	iRemaining;
	Count	iStride;

	if (SIZE::CheckSize && ((iNumBatches != 0) && (iNumInBatch > SIZE::Max / iNumBatches)))
	{
		Overflowed();
		return;
	}
	iOldNumElements = AddNum(iNumInBatch * iNumBatches);
	if (iOldNumElements == SIZE::NotFound)
	{
		return;
	}

	pcFirst = Get(iFirstElementPos);

	i = iNumBatches - 1;

	iDest = (iNumInBatch + iSkip) * (i + 1) - iSkip;
	iSource = iSkip * i;
	pcDest = RemapSinglePointer(pcFirst, (Count)(iDest * miElementSize));
	pcSource = RemapSinglePointer(pcFirst, (Count)(iSource * miElementSize));

	if (iOldNumElements > iSource)
	{
		if ((iOldNumElements - iSource) > iFirstElementPos)
		{
			iRemaining = (iOldNumElements - iSource) - iFirstElementPos;
			SIZE::Move(pcDest, pcSource, iRemaining * miElementSize);
		}
	}

	//Each batch moves up by less than its own length when the inserted part is smaller than it, so the moves overlap.
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), 1);
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), 2);
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), 4);
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), 8);
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), 12);
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
				SIZE::Move(RemapSinglePointer(pcFirst, (Count)(iTotalStride * (i + 1) - iStride)), RemapSinglePointer(pcFirst, (Count)(iStride * i)), iStride);
			}
			while (i != 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CArrayBlockBase<SIZE>::GetHeader(SArrayHeaderBase<SIZE>* psHeader)
{
	psHeader->miElementSize = miElementSize;
	psHeader->miUsedElements = miUsedElements;
	psHeader->miChunkSize = miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool SArrayHeaderBase<SIZE>::Write(CFileWriter* pcFileWriter)
{
	return pcFileWriter->WriteSize(miElementSize) &&
		   pcFileWriter->WriteSize(miUsedElements) &&
		   pcFileWriter->WriteSize(miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool SArrayHeaderBase<SIZE>::Read(CFileReader* pcFileReader)
{
	return pcFileReader->ReadSize(&miElementSize) &&
		   pcFileReader->ReadSize(&miUsedElements) &&
		   pcFileReader->ReadSize(&miChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CArrayBlockBase<SIZE>::WriteHeader(CFileWriter* pcFileWriter)
{
	return WriteHeader(pcFileWriter, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//uiFileElementSize is written in place of the element size.
template<class SIZE>
bool CArrayBlockBase<SIZE>::WriteHeader(CFileWriter* pcFileWriter, Count uiFileElementSize)
{
	SArrayHeaderBase<SIZE>	sHeader;

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
template<class SIZE>
bool CArrayBlockBase<SIZE>::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::Write(CFileWriter* pcFileWriter)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc)
{
	return ReadHeader(pcFileReader, pcMalloc, SIZE::Max, SIZE::Max);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//The element size in the file must be uiFileElementSize and the elements are uiElementSize in memory.  MAX_SIZE for
//both uses the element size in the file.
template<class SIZE>
bool CArrayBlockBase<SIZE>::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, Count uiFileElementSize, Count uiElementSize)
{
	SArrayHeaderBase<SIZE>	sHeader;

	if (!sHeader.Read(pcFileReader))
	{
		return false;
	}

	if (uiFileElementSize == SIZE::Max)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::ReadAllocatorAndHeader(CFileReader* pcFileReader)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::Read(CFileReader* pcFileReader)
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
template<class SIZE>
bool CArrayBlockBase<SIZE>::IsSorted(DataCompare fCompare)
{
	Count	ui;
	Count	uiNumElements;
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
		pvRight = Get(ui + 1);
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
template<class SIZE>
void* CArrayBlockBase<SIZE>::Push(void) { return Add(); }
template<class SIZE>
bool CArrayBlockBase<SIZE>::PopFirst(void) { return RemoveFirst(); }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::ByteSize() { return miUsedElements * miElementSize; }
template<class SIZE>
bool CArrayBlockBase<SIZE>::Pop(void) { return RemoveTail(); }
template<class SIZE>
bool CArrayBlockBase<SIZE>::Contains(void* pvData) { return Find(pvData) != SIZE::NotFound; }
template<class SIZE>
void CArrayBlockBase<SIZE>::Sort(DataCompare fCompare) { TimSort(fCompare); }
template<class SIZE>
void CArrayBlockBase<SIZE>::BubbleSort(DataCompare fCompare) { ::BubbleSort(fCompare, mpvArray, miElementSize, miUsedElements); }
template<class SIZE>
void CArrayBlockBase<SIZE>::QuickSort(DataCompare fCompare) { ::QuickSort(fCompare, mpvArray, miElementSize, miUsedElements); }
template<class SIZE>
void CArrayBlockBase<SIZE>::TimSort(DataCompare fCompare) { ::TimSort(fCompare, mpvArray, miElementSize, miUsedElements); }
template<class SIZE>
void* CArrayBlockBase<SIZE>::GetData(void) const { return mpvArray; }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::ChunkSize(void) { return miChunkSize; }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::RemoveNum(Count iNumElements) { return SetUsedElements(miUsedElements - iNumElements); }
template<class SIZE>
void CArrayBlockBase<SIZE>::FakeSetUsedElements(Count iUsedElements) { miUsedElements = iUsedElements; }
template<class SIZE>
void CArrayBlockBase<SIZE>::SetAllocateSize(Count iSize) { miChunkSize = iSize; }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::ElementSize(void) const { return miElementSize; }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::NumElements(void) const { return miUsedElements; }
template<class SIZE>
typename SIZE::Count CArrayBlockBase<SIZE>::AllocatedElements(void) const { return miNumElements; }
template<class SIZE>
bool CArrayBlockBase<SIZE>::IsEmpty(void) const { return miUsedElements == 0; }
template<class SIZE>
bool CArrayBlockBase<SIZE>::IsNotEmpty(void) const { return miUsedElements != 0; }


//The only instances: ArrayBlock.h declares them extern.
template struct SArrayHeaderBase<SArrayNaturalSize>;
template struct SArrayHeaderBase<SArrayLargeSize>;
template class CArrayBlockBase<SArrayNaturalSize>;
template class CArrayBlockBase<SArrayLargeSize>;

