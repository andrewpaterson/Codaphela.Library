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
	mpvCache = pvCache;

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
	SMemoryCacheDescriptor*		psDescriptor;
	size_t						uiSize;
	SMemoryCacheDescriptor*		psNewPrev;
	SMemoryCacheDescriptor*		psNew;

	memset(pvNewCache, 0, uiCacheSize);

	if (mpsHead)
	{
		iAbsDiff = (size_t)pvNewCache - (size_t)mpvCache;

		psDescriptor = mpsHead;
		psNewPrev = NULL;
		psNew = (SMemoryCacheDescriptor*)pvNewCache;
		do
		{
			uiSize = miDescriptorSize + psDescriptor->uiSize;
			memcpy(psNew, psDescriptor, uiSize);

			psNew->psNext = NULL;
			psNew->psPrev = psNewPrev;
			if (psNewPrev != NULL)
			{
				psNewPrev->psNext = psNew;
			}

			psNewPrev = psNew;
			psNew = (SMemoryCacheDescriptor*)RemapSinglePointer(psNew, uiSize);
			psDescriptor = psDescriptor->psNext;
		} while (psDescriptor != mpsHead);

		mpsHead = (SMemoryCacheDescriptor*)pvNewCache;
		mpsHead->psPrev = psNewPrev;
		mpsTail = psNewPrev;
		mpsTail->psNext = mpsHead;
	}
	else
	{
		mpsHead = NULL;
		mpsTail = NULL;
	}

	mpvCache = pvNewCache;
	muiCacheSize = uiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::RemapSameMemory(void* pvNewCache, size_t uiCacheSize)
{
	SMemoryCacheDescriptor*		psDescriptor;
	SMemoryCacheDescriptor*		psLargest;
	SMemoryCacheDescriptor*		psNew;
	size_t						uiSize;
	size_t						uiRemaining;

	muiCacheSize = uiCacheSize;
	if (pvNewCache == mpsHead)
	{
		return;
	}

	psDescriptor = mpsHead;
	psLargest = NULL;
	do
	{
		if (psDescriptor < psDescriptor->psPrev)
		{
			if (psDescriptor->psPrev != mpsTail)
			{
				psLargest = psDescriptor->psPrev;
				break;
			}
		}
		psDescriptor = psDescriptor->psNext;
	} while (psDescriptor != mpsHead);

	if (psLargest == NULL)
	{
		return;
	}

	psNew = NULL;
	psDescriptor = psLargest->psNext;
	do
	{
		uiSize = miDescriptorSize + psDescriptor->uiSize;
		uiRemaining = RemainingAfter(psLargest);
		if (uiRemaining >= uiSize)
		{
			psNew = (SMemoryCacheDescriptor*)RemapSinglePointer(psLargest, psLargest->uiSize + miDescriptorSize);
		}
		else
		{
			psNew = (SMemoryCacheDescriptor*)mpvCache;
		}
		memcpy(psNew, psDescriptor, uiSize);

		psLargest->psNext = psNew;
		psNew->psPrev = psLargest;
		psNew->psPrev->psNext = psNew;

		psLargest = psNew;
		
		psDescriptor = psDescriptor->psNext;
	} while (psDescriptor != mpsHead);

	mpsTail = psLargest;
	mpsHead->psPrev = mpsTail;
	mpsTail->psNext = mpsHead;
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
void CCircularMemoryList::Deallocate(SMemoryCacheDescriptor* psDescriptor)
{
	if ((psDescriptor == mpsHead) && (psDescriptor == mpsTail))
	{
		mpsHead = NULL;
		mpsTail = NULL;
		return;
	}

	if (psDescriptor != mpsHead)
	{
		psDescriptor->psPrev->psNext = psDescriptor->psNext;
	}
	else
	{
		psDescriptor->psPrev->psNext = psDescriptor->psNext;
		mpsHead = psDescriptor->psNext;
	}

	if (psDescriptor != mpsTail)
	{
		psDescriptor->psNext->psPrev = psDescriptor->psPrev;
	}
	else
	{
		psDescriptor->psNext->psPrev = psDescriptor->psPrev;
		mpsTail = psDescriptor->psPrev;
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
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetNext(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psNext;

	return psCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetPrev(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psPrev;

	return psCurrent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetLast(void)
{
	return mpsTail;
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
void* CCircularMemoryList::GetData(SMemoryCacheDescriptor* psDescriptor)
{
	return RemapSinglePointer(psDescriptor, miDescriptorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetDescriptor(void* pvData)
{
	return (SMemoryCacheDescriptor*)RemapSinglePointer(pvData, -miDescriptorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircularMemoryList::NumElements(int iSize)
{
	SMemoryCacheDescriptor* psDescriptor;
	int							iNum;

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
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::Iterate(SMemoryCacheDescriptor* psCurrent)
{
	if (!psCurrent)
	{
		return NULL;
	}

	psCurrent = psCurrent->psNext;
	if (psCurrent != mpsHead)
	{
		return psCurrent;
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
	SMemoryCacheDescriptor* psDescriptor;

	psDescriptor = (SMemoryCacheDescriptor*)mpvCache;

	mpsTail = psDescriptor;
	mpsHead = psDescriptor;

	psDescriptor->psNext = psDescriptor;
	psDescriptor->psPrev = psDescriptor;

	return psDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::InsertNext(SMemoryCacheDescriptor* psDescriptor)
{
	if (IsEmpty())
	{
		psDescriptor = OneAllocation();
	}
	else
	{
		mpsTail->psNext = psDescriptor;
		mpsHead->psPrev = psDescriptor;

		psDescriptor->psNext = mpsHead;
		psDescriptor->psPrev = mpsTail;

		mpsTail = psDescriptor;
	}

	return psDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCircularMemoryList::Overlaps(void* pvNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting)
{
	size_t	uiNewStart;
	size_t	uiNewEnd;  //Inclusive

	size_t	uiNextStart;
	size_t	uiNextEnd; //Inclusive

	uiNewStart = (size_t)pvNew;
	uiNewEnd = uiNewStart + uiNewSize - 1;

	uiNextStart = (size_t)psExisting;
	uiNextEnd = uiNextStart + psExisting->uiSize + miDescriptorSize - 1;

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
		return RemainingAfter(mpsTail);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CCircularMemoryList::RemainingAfter(SMemoryCacheDescriptor* psDescriptor)
{
	size_t		iAllocated;

	iAllocated = ((int)(size_t)psDescriptor - (int)(size_t)mpvCache);
	iAllocated += (psDescriptor->uiSize + miDescriptorSize);
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
	SMemoryCacheDescriptor*		psDescriptor;
	int							iCount;
	void*						pvLastCacheByte;
	void*						pvEndDesc;
	size_t						uiOffset;

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
	psDescriptor = mpsHead;
	uiOffset = (size_t)psDescriptor - (size_t)mpvCache;
	do
	{
		if (psDescriptor->psNext == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Next is NULL.", NULL);
		}
		if (psDescriptor->psPrev == NULL)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Prev is NULL.", NULL);
		}

		if (psDescriptor->psPrev->psNext != psDescriptor)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Prev.Next is not Descriptor.", NULL);
		}
		if (psDescriptor->psNext->psPrev != psDescriptor)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "] Next.Prev is not Descriptor.", NULL);
		}

		pvEndDesc = RemapSinglePointer(psDescriptor, psDescriptor->uiSize + miDescriptorSize);
		if (pvEndDesc > pvLastCacheByte)
		{
			return gcLogger.Error2(__METHOD__, " Descriptor [", IntToString(iCount), "]'s bytes extend passed the end of the cache.", NULL);
		}

		iCount++;
		psDescriptor = psDescriptor->psNext;
		uiOffset = (size_t)psDescriptor - (size_t)mpvCache;
	} 
	while (psDescriptor != mpsHead);
	return TRUE;
}

