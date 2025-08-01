/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Logger.h"
#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "Chars.h"
#include "CircularMemoryList.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Init(void* pvCache, size uiByteSize, size iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;

	if (uiByteSize >= sizeof(SCircularMemoryList))
	{
		mpsDetail = (SCircularMemoryList*)pvCache;
		mpvCache = (SMemoryCacheDescriptor*)RemapSinglePointer(pvCache, sizeof(SCircularMemoryList));
		mpsDetail->muiCacheSize = uiByteSize - sizeof(SCircularMemoryList);
		Clear();
	}
	else
	{
		mpvCache = NULL;
		mpsDetail = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::InitExisting(void* pvCache, size uiByteSize, size iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;
	mpsDetail = (SCircularMemoryList*)pvCache;
	mpvCache = (SMemoryCacheDescriptor*)RemapSinglePointer(pvCache, sizeof(SCircularMemoryList));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Kill(void)
{
	mpvCache = NULL;
	mpsDetail = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Remap(void* pvNewCache, size uiByteSize)
{
	if (pvNewCache != mpsDetail)
	{
		RemapDifferentMemory(RemapSinglePointer(pvNewCache, sizeof(SCircularMemoryList)), uiByteSize);
	}
	else
	{
		RemapSameMemory(uiByteSize - sizeof(SCircularMemoryList));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Remap(SCircularMemoryList* pvNewCache, size uiByteSize)
{
	if (pvNewCache)
	{
		mpsDetail = pvNewCache;
		mpvCache = (SMemoryCacheDescriptor*)RemapSinglePointer(pvNewCache, sizeof(SCircularMemoryList));
		if (mpsDetail->muiCacheSize != uiByteSize - sizeof(SCircularMemoryList))
		{
			RemapSameMemory(uiByteSize - sizeof(SCircularMemoryList));
		}
	}
	else
	{
		mpsDetail = NULL;
		mpvCache = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::RemapDifferentMemory(void* pvNewCache, size uiCacheSize)
{
	ptrdiff_t					iAbsDiff;
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size						uiSize;
	SMemoryCacheDescriptor*		psNewPrev;
	SMemoryCacheDescriptor*		psNew;

	if (!IsEmpty())
	{
		iAbsDiff = (size)pvNewCache - (size)mpvCache;

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
		} 
		while (!IsFirst(psCacheBasedDescriptor));

		mpvCache = (SMemoryCacheDescriptor*)pvNewCache;
		mpsDetail = (SCircularMemoryList*)RemapSinglePointer(pvNewCache, -((ptrdiff_t)sizeof(SCircularMemoryList)));
		mpsDetail->muiCacheSize = uiCacheSize;

		mpsDetail->mpsHead = (SMemoryCacheDescriptor*)1;
		GetFirst()->psPrev = MapFromCacheBasedToZeroBased(psNewPrev);
		mpsDetail->mpsTail = MapFromCacheBasedToZeroBased(psNewPrev);
		GetLast()->psNext = mpsDetail->mpsHead;
	}
	else
	{
		mpvCache = (SMemoryCacheDescriptor*)pvNewCache;
		mpsDetail = (SCircularMemoryList*)RemapSinglePointer(pvNewCache, -((ptrdiff_t)sizeof(SCircularMemoryList)));
		mpsDetail->muiCacheSize = uiCacheSize;

		mpsDetail->mpsHead = NULL;
		mpsDetail->mpsTail = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::RemapSameMemory(size uiCacheSize)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	SMemoryCacheDescriptor*		psLargest;
	SMemoryCacheDescriptor*		psNew;
	size						uiSize;
	size						uiRemaining;
	
	mpsDetail->muiCacheSize = uiCacheSize;
	if (mpvCache == GetFirst() || mpsDetail->mpsHead == NULL)
	{
		return;
	}

	psCacheBasedDescriptor = GetFirst();
	psLargest = NULL;
	do
	{
		if (psCacheBasedDescriptor < GetPrev(psCacheBasedDescriptor))
		{
			if (psCacheBasedDescriptor->psPrev != mpsDetail->mpsTail)
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

	mpsDetail->mpsTail = MapFromCacheBasedToZeroBased(psLargest);
	GetFirst()->psPrev = mpsDetail->mpsTail;
	GetLast()->psNext = mpsDetail->mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircularMemoryList::Clear(void)
{
	mpsDetail->mpsTail = NULL;
	mpsDetail->mpsHead = NULL;
	memset(mpvCache, 0, mpsDetail->muiCacheSize);
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
		psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(pvData, -((ptrdiff)miDescriptorSize));
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
		mpsDetail->mpsHead = NULL;
		mpsDetail->mpsTail = NULL;
		return;
	}

	if (!IsFirst(psCacheBasedDescriptor))
	{
		GetPrev(psCacheBasedDescriptor)->psNext = psCacheBasedDescriptor->psNext;
	}
	else
	{
		GetPrev(psCacheBasedDescriptor)->psNext = psCacheBasedDescriptor->psNext;
		mpsDetail->mpsHead = psCacheBasedDescriptor->psNext;
	}

	if (!IsLast(psCacheBasedDescriptor))
	{
		GetNext(psCacheBasedDescriptor)->psPrev = psCacheBasedDescriptor->psPrev;
	}
	else
	{
		GetNext(psCacheBasedDescriptor)->psPrev = psCacheBasedDescriptor->psPrev;
		mpsDetail->mpsTail = psCacheBasedDescriptor->psPrev;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::GetCacheSize(void)
{
	return mpsDetail->muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::GetAllocatedSize(void)
{
	SMemoryCacheDescriptor* psIter;
	size					uiSize;

	uiSize = 0;
	psIter = StartIteration();
	while (psIter)
	{
		uiSize += psIter->uiSize + miDescriptorSize;
		psIter = Iterate(psIter);
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::GetFirst(void)
{
	return MapFromZeroBasedToCacheBased(mpsDetail->mpsHead);
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
	return MapFromZeroBasedToCacheBased(mpsDetail->mpsTail);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::GetSize(void* pvCacheBasedData)
{
	SMemoryCacheDescriptor* psDescriptor;

	psDescriptor = GetDescriptorNoRemap(pvCacheBasedData);
	return psDescriptor->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::NumElements(void)
{
	SMemoryCacheDescriptor*		psDescriptor;
	size							iNum;

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
size CCircularMemoryList::GetDescriptorSize(void)
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
		return (SMemoryCacheDescriptor*)RemapSinglePointer(pvCacheBasedData, -((ptrdiff)(miDescriptorSize)));
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::NumElements(size iSize)
{
	SMemoryCacheDescriptor*		psDescriptor;
	size						iNum;

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
bool CCircularMemoryList::IsEmpty(void)
{
	return mpsDetail == NULL || mpsDetail->mpsTail == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::StartIteration(void)
{
	return MapFromZeroBasedToCacheBased(mpsDetail->mpsHead);
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
	SMemoryCacheDescriptor*		psDescriptor;
	CChars						sz;
	char*						pvData;
	size						iLen;

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

	mpsDetail->mpsTail = (SMemoryCacheDescriptor*)1;
	mpsDetail->mpsHead = (SMemoryCacheDescriptor*)1;

	psCacheBasedDescriptor->psNext = mpsDetail->mpsHead;
	psCacheBasedDescriptor->psPrev = mpsDetail->mpsHead;

	return psCacheBasedDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CCircularMemoryList::InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	SMemoryCacheDescriptor* psTail;
	SMemoryCacheDescriptor* psHead;

	if (IsEmpty())
	{
		psCacheBasedDescriptor = OneAllocation();
	}
	else
	{
		psTail = MapFromZeroBasedToCacheBased(mpsDetail->mpsTail);
		psHead = MapFromZeroBasedToCacheBased(mpsDetail->mpsHead);
		psTail->psNext = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
		psHead->psPrev = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);

		psCacheBasedDescriptor->psNext = mpsDetail->mpsHead;
		psCacheBasedDescriptor->psPrev = mpsDetail->mpsTail;

		mpsDetail->mpsTail = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
	}

	return psCacheBasedDescriptor;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//SMemoryCacheDescriptor* CCircularMemoryList::InsertNext(SMemoryCacheDescriptor* psCacheBasedDescriptor)
//{
//	if (IsEmpty())
//	{
//		psCacheBasedDescriptor = OneAllocation();
//	}
//	else
//	{
//		MapFromZeroBasedToCacheBased(mpsTail)->psNext = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
//		MapFromZeroBasedToCacheBased(mpsHead)->psPrev = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
//
//		psCacheBasedDescriptor->psNext = mpsHead;
//		psCacheBasedDescriptor->psPrev = mpsTail;
//
//		mpsTail = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
//	}
//
//	return psCacheBasedDescriptor;
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircularMemoryList::Overlaps(SMemoryCacheDescriptor* pvCacheBasedNew, size uiNewSize, SMemoryCacheDescriptor* psCacheBasedExisting)
{
	size	uiNewStart;
	size	uiNewEnd;  //Inclusive

	size	uiNextStart;
	size	uiNextEnd; //Inclusive

	uiNewStart = (size)pvCacheBasedNew;
	uiNewEnd = uiNewStart + uiNewSize - 1;

	uiNextStart = (size)psCacheBasedExisting;
	uiNextEnd = uiNextStart + psCacheBasedExisting->uiSize + miDescriptorSize - 1;

	if ((uiNewStart <= uiNextStart) && (uiNewEnd >= uiNextStart))
	{
		return true;
	}
	if ((uiNewStart <= uiNextEnd) && (uiNewEnd >= uiNextEnd))
	{
		return true;
	}
	if ((uiNewStart >= uiNextStart) && (uiNewStart <= uiNextEnd))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircularMemoryList::RemainingAfterTail(void)
{
	if (IsEmpty())
	{
		return mpsDetail->muiCacheSize;
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
size CCircularMemoryList::RemainingAfter(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	size		iAllocated;

	iAllocated = (size)((ptr)psCacheBasedDescriptor - (ptr)mpvCache);
	iAllocated += (psCacheBasedDescriptor->uiSize + miDescriptorSize);
	if (iAllocated < mpsDetail->muiCacheSize)
	{
		return mpsDetail->muiCacheSize - iAllocated;
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
bool CCircularMemoryList::ValidateCache(void)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size							iCount;
	void*						pvLastCacheByte;
	void*						pvEndDesc;

	if (mpvCache == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cache is NULL.", NULL);
	}
	if (mpsDetail->muiCacheSize == 0)
	{
		return gcLogger.Error2(__METHOD__, " Cache Size is zero.", NULL);
	}

	pvLastCacheByte = RemapSinglePointer(mpvCache, mpsDetail->muiCacheSize);

	if (mpsDetail->mpsHead == NULL)
	{
		if (mpsDetail->mpsTail != NULL)
		{
			return gcLogger.Error2(__METHOD__, " Head is NULL but Tail is not NULL.", NULL);
		}
	}

	if (mpsDetail->mpsTail == NULL)
	{
		if (mpsDetail->mpsHead != NULL)
		{
			return gcLogger.Error2(__METHOD__, " Tail is NULL but Head is not NULL.", NULL);
		}
	}

	if (mpsDetail->mpsHead == NULL)
	{
		return true;
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
	return true;
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
bool CCircularMemoryList::IsFirst(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	return MapFromZeroBasedToCacheBased(mpsDetail->mpsHead) == psCacheBasedDescriptor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircularMemoryList::IsLast(SMemoryCacheDescriptor* psCacheBasedDescriptor)
{
	return MapFromZeroBasedToCacheBased(mpsDetail->mpsTail) == psCacheBasedDescriptor;
}

