#include "Logger.h"
#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "Chars.h"
#include "CircularMemoryList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Init(void* pvCache, size_t uiCacheSize, int iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;
	muiCacheSize = uiCacheSize;
	mpvCache = (SMemoryCacheDescriptor*)pvCache;

	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Kill(void)
{
	mpvCache = NULL;
	mpsTail = NULL;
	mpsHead = NULL;
	muiCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Remap(void* pvNewCache, size_t uiCacheSize)
{
	if (pvNewCache != mpvCache)
	{
		RemapDifferentMemory(pvNewCache, uiCacheSize);
	}
	else
	{
		RemapSameMemory(pvNewCache, uiCacheSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::RemapDifferentMemory(void* pvNewCache, size_t uiCacheSize)
{
	ptrdiff_t					iAbsDiff;
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size_t						uiSize;
	SMemoryCacheDescriptor*		psNewPrev;
	SMemoryCacheDescriptor*		psNew;

	memset(pvNewCache, 0, uiCacheSize);

	//Fuck knows if the offsets from the new cache vs the old cache work anymore.
	if (!IsEmpty())
	{
		iAbsDiff = (size_t)pvNewCache - (size_t)mpvCache;

		psCacheBasedDescriptor = GetFirst();
		psNewPrev = NULL;
		psNew = (SMemoryCacheDescriptor*)pvNewCache;
		do
		{
			uiSize = miDescriptorSize + psCacheBasedDescriptor->uiSize;
			memcpy(psNew, psCacheBasedDescriptor, uiSize);

			psNew->psNext = NULL;
			psNew->psPrev = MapFromCacheBasedToZeroBased(pvNewCache, psNewPrev);
			if (psNewPrev != NULL)
			{
				psNewPrev->psNext = MapFromCacheBasedToZeroBased(pvNewCache, psNew);
			}

			psNewPrev = psNew;
			psNew = (SMemoryCacheDescriptor*)RemapSinglePointer(psNew, uiSize);
			psCacheBasedDescriptor = GetNext(psCacheBasedDescriptor);
		} while (!IsFirst(psCacheBasedDescriptor));

		mpvCache = (SMemoryCacheDescriptor*)pvNewCache;
		muiCacheSize = uiCacheSize;

		mpsHead = (SMemoryCacheDescriptor*)1;
		GetFirst()->psPrev = MapFromCacheBasedToZeroBased(psNewPrev);
		mpsTail = MapFromCacheBasedToZeroBased(psNewPrev);
		GetLast()->psNext = mpsHead;
	}
	else
	{
		mpvCache = (SMemoryCacheDescriptor*)pvNewCache;
		muiCacheSize = uiCacheSize;

		mpsHead = NULL;
		mpsTail = NULL;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::RemapSameMemory(void* pvNewCache, size_t uiCacheSize)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	SMemoryCacheDescriptor*		psLargest;
	SMemoryCacheDescriptor*		psNew;
	size_t						uiSize;
	size_t						uiRemaining;

	muiCacheSize = uiCacheSize;
	if (pvNewCache == GetFirst() || mpsHead == NULL)
	{
		return;
	}

	psCacheBasedDescriptor = GetFirst();
	psLargest = NULL;
	do
	{
		if (psCacheBasedDescriptor < GetPrev(psCacheBasedDescriptor))
		{
			if (psCacheBasedDescriptor->psPrev != mpsTail)
			{
				psLargest = GetPrev(psCacheBasedDescriptor);
				break;
			}
		}
		psCacheBasedDescriptor = GetNext(psCacheBasedDescriptor);
	} while (!IsFirst(psCacheBasedDescriptor));

	if (psLargest == NULL)
	{
		return;
	}

	psNew = NULL;
	psCacheBasedDescriptor = GetNext(psLargest);
	do
	{
		uiSize = miDescriptorSize + psCacheBasedDescriptor->uiSize;
		uiRemaining = RemainingAfter(psLargest);
		if (uiRemaining >= uiSize)
		{
			psNew = (SMemoryCacheDescriptor*)RemapSinglePointer(psLargest, psLargest->uiSize + miDescriptorSize);
		}
		else
		{
			psNew = (SMemoryCacheDescriptor*)mpvCache;
		}
		memcpy(psNew, psCacheBasedDescriptor, uiSize);

		psLargest->psNext = MapFromCacheBasedToZeroBased(psNew);
		psNew->psPrev = MapFromCacheBasedToZeroBased(psLargest);
		GetPrev(psNew)->psNext = MapFromCacheBasedToZeroBased(psNew);

		psLargest = psNew;
		
		psCacheBasedDescriptor = GetNext(psCacheBasedDescriptor);
	} while (!IsFirst(psCacheBasedDescriptor));

	mpsTail = MapFromCacheBasedToZeroBased(psLargest);
	GetFirst()->psPrev = mpsTail;
	GetLast()->psNext = mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Clear(void)
{
	mpsTail = NULL;
	mpsHead = NULL;
	memset(mpvCache, 0, muiCacheSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Deallocate(void* pvData)
{
	SMemoryCacheDescriptor* psDescriptor;

	if (pvData)
	{
		psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(pvData, -miDescriptorSize);
		Deallocate(psDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Deallocate(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (IsFirst(psCacheBasedDescriptor) && IsLast(psCacheBasedDescriptor))
	{
		mpsHead = NULL;
		mpsTail = NULL;
		return;
	}

	if (!IsFirst(psCacheBasedDescriptor))
	{
		GetPrev(psCacheBasedDescriptor)->psNext = psCacheBasedDescriptor->psNext;
	}
	else
	{
		GetPrev(psCacheBasedDescriptor)->psNext = psCacheBasedDescriptor->psNext;
		mpsHead = psCacheBasedDescriptor->psNext;
	}

	if (!IsLast(psCacheBasedDescriptor))
	{
		GetNext(psCacheBasedDescriptor)->psPrev = psCacheBasedDescriptor->psPrev;
	}
	else
	{
		GetNext(psCacheBasedDescriptor)->psPrev = psCacheBasedDescriptor->psPrev;
		mpsTail = psCacheBasedDescriptor->psPrev;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::GetCacheSize(void)
{
	return muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::GetAllocatedSize(void)
{
	SMemoryCacheDescriptor* psIter;
	size_t					tSize;

	tSize = 0;
	psIter = StartIteration();
	while (psIter)
	{
		tSize += psIter->uiSize + miDescriptorSize;
		psIter = Iterate(psIter);
	}
	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetFirst(void)
{
	return MapFromZeroBasedToCacheBased(mpsHead);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetNext(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (!psCacheBasedDescriptor)
	{
		return NULL;
	}

	return MapFromZeroBasedToCacheBased(psCacheBasedDescriptor->psNext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetPrev(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (!psCacheBasedDescriptor)
	{
		return NULL;
	}

	return MapFromZeroBasedToCacheBased(psCacheBasedDescriptor->psPrev);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetLast(void)
{
	return MapFromZeroBasedToCacheBased(mpsTail);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::GetSize(void* pvCacheBasedData)
{
	SMemoryCacheDescriptor* psDescriptor;

	psDescriptor = GetDescriptorNoRemap(pvCacheBasedData);
	return psDescriptor->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::NumElements(void)
{
	SMemoryCacheDescriptor*		psDescriptor;
	int							iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		iNum++;
		psDescriptor = Iterate(psDescriptor);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::GetDescriptorSize(void)
{
	return miDescriptorSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CCircularMemoryList::GetData(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (psCacheBasedDescriptor)
	{
		return RemapSinglePointer(psCacheBasedDescriptor, miDescriptorSize);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetDescriptorNoRemap(void* pvCacheBasedData)
{
	if (pvCacheBasedData)
	{
		return (SMemoryCacheDescriptor*)RemapSinglePointer(pvCacheBasedData, -miDescriptorSize);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::NumElements(int iSize)
{
	SMemoryCacheDescriptor* psDescriptor;
	int						iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		if (psDescriptor->uiSize == iSize)
		{
			iNum++;
		}
		psDescriptor = Iterate(psDescriptor);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::IsEmpty(void)
{
	return mpsTail == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::StartIteration(void)
{
	return MapFromZeroBasedToCacheBased(mpsHead);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::Iterate(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	SMemoryCacheDescriptor*		mpsCacheBasedHead;

	if (!psCacheBasedDescriptor)
	{
		return NULL;
	}

	mpsCacheBasedHead = GetFirst();
	psCacheBasedDescriptor = MapFromZeroBasedToCacheBased(psCacheBasedDescriptor->psNext);
	if (psCacheBasedDescriptor != mpsCacheBasedHead)
	{
		return psCacheBasedDescriptor;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Dump(void)
{
	SMemoryCacheDescriptor* psDescriptor;
	CChars						sz;
	char* pvData;
	int							iLen;

	sz.Init();

	sz.Append("Data Cache (");
	sz.Append(NumElements());
	sz.Append(")\n---------------\n");

	psDescriptor = StartIteration();
	while (psDescriptor)
	{
		pvData = (char*)RemapSinglePointer(psDescriptor, miDescriptorSize);
		iLen = psDescriptor->uiSize;

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psDescriptor, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psDescriptor->psNext, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
		psDescriptor = Iterate(psDescriptor);
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::OneAllocation(void)
{
	SMemoryCacheDescriptor* psCacheBasedDescriptor;

	psCacheBasedDescriptor = (SMemoryCacheDescriptor*)mpvCache;

	mpsTail = (SMemoryCacheDescriptor*)1;
	mpsHead = (SMemoryCacheDescriptor*)1;

	psCacheBasedDescriptor->psNext = mpsHead;
	psCacheBasedDescriptor->psPrev = mpsHead;

	return psCacheBasedDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (IsEmpty())
	{
		psCacheBasedDescriptor = OneAllocation();
	}
	else
	{
		MapFromZeroBasedToCacheBased(mpsTail)->psNext = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
		MapFromZeroBasedToCacheBased(mpsHead)->psPrev = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);

		psCacheBasedDescriptor->psNext = mpsHead;
		psCacheBasedDescriptor->psPrev = mpsTail;

		mpsTail = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
	}

	return psCacheBasedDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, size_t uiNewSize, SMemoryCacheDescriptor* psCacheBasedExisting)
{
	size_t	uiNewStart;
	size_t	uiNewEnd;  //Inclusive

	size_t	uiNextStart;
	size_t	uiNextEnd; //Inclusive

	uiNewStart = (size_t)pvCacheBasedNew;
	uiNewEnd = uiNewStart + uiNewSize - 1;

	uiNextStart = (size_t)psCacheBasedExisting;
	uiNextEnd = uiNextStart + psCacheBasedExisting->uiSize + miDescriptorSize - 1;

	if ((uiNewStart <= uiNextStart) && (uiNewEnd >= uiNextStart))
	{
		return TRUE;
	}
	if ((uiNewStart <= uiNextEnd) && (uiNewEnd >= uiNextEnd))
	{
		return TRUE;
	}
	if ((uiNewStart >= uiNextStart) && (uiNewStart <= uiNextEnd))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::RemainingAfterTail(void)
{
	if (IsEmpty())
	{
		return muiCacheSize;
	}
	else
	{
		return RemainingAfter(GetLast());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	size_t		iAllocated;

	iAllocated = ((int)(size_t)psCacheBasedDescriptor - (int)(size_t)mpvCache);
	iAllocated += (psCacheBasedDescriptor->uiSize + miDescriptorSize);
	if (iAllocated < muiCacheSize)
	{
		return muiCacheSize - iAllocated;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::ValidateCache(void)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	int							iCount;
	void*						pvLastCacheByte;
	void*						pvEndDesc;

	if (mpvCache == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cache is NULL.", NULL);
	}
	if (muiCacheSize == 0)
	{
		return gcLogger.Error2(__METHOD__, " Cache Size is zero.", NULL);
	}

	pvLastCacheByte = RemapSinglePointer(mpvCache, muiCacheSize);

	if (mpsHead == NULL)
	{
		if (mpsTail != NULL)
		{
			return gcLogger.Error2(__METHOD__, " Head is NULL but Tail is not NULL.", NULL);
		}
	}

	if (mpsTail == NULL)
	{
		if (mpsHead != NULL)
		{
			return gcLogger.Error2(__METHOD__, " Tail is NULL but Head is not NULL.", NULL);
		}
	}

	if (mpsHead == NULL)
	{
		return TRUE;
	}

	iCount = 0;
	psCacheBasedDescriptor = GetFirst();
	do
	{
		if (psCacheBasedDescriptor->psNext == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Next is NULL.", NULL);
		}
		if (psCacheBasedDescriptor->psPrev == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Prev is NULL.", NULL);
		}

		if (GetPrev(psCacheBasedDescriptor)->psNext != MapFromCacheBasedToZeroBased(psCacheBasedDescriptor))
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Prev.Next is not Descriptor.", NULL);
		}
		if (GetNext(psCacheBasedDescriptor)->psPrev != MapFromCacheBasedToZeroBased(psCacheBasedDescriptor))
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Next.Prev is not Descriptor.", NULL);
		}

		pvEndDesc = RemapSinglePointer(psCacheBasedDescriptor, psCacheBasedDescriptor->uiSize + miDescriptorSize);
		if (pvEndDesc > pvLastCacheByte)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "]'s bytes extend passed the end of the cache.", NULL);
		}

		iCount++;
		psCacheBasedDescriptor = GetNext(psCacheBasedDescriptor);
	} 
	while (!IsFirst(psCacheBasedDescriptor));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::MapFromZeroBasedToCacheBased(SMemoryCacheDescriptor* psZeroBasedDescriptor)
{
	if (psZeroBasedDescriptor)
	{
		return (SMemoryCacheDescriptor*)RemapSinglePointer(psZeroBasedDescriptor, ((ptrdiff_t)mpvCache) - 1);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::MapFromCacheBasedToZeroBased(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	if (psCacheBasedDescriptor)
	{
		return (SMemoryCacheDescriptor*)RemapSinglePointer(psCacheBasedDescriptor, (-(ptrdiff_t)mpvCache) + 1);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::MapFromZeroBasedToCacheBased(void* pvCache, SMemoryCacheDescriptor* psZeroBasedDescriptor)
{
	return (SMemoryCacheDescriptor*)RemapSinglePointer(psZeroBasedDescriptor, ((ptrdiff_t)pvCache) - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::MapFromCacheBasedToZeroBased(void* pvCache, SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	return (SMemoryCacheDescriptor*)RemapSinglePointer(psCacheBasedDescriptor, (-(ptrdiff_t)pvCache) + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::IsFirst(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	return MapFromZeroBasedToCacheBased(mpsHead) == psCacheBasedDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::IsLast(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	return MapFromZeroBasedToCacheBased(mpsTail) == psCacheBasedDescriptor;
}

