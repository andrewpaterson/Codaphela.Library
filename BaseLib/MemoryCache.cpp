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
#include "FastMemcpy.h"
#include "FastMemcmp.h"
#include "FastMemset.h"
#include "Chars.h"
#include "MemoryCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Init(size uiCacheSize, CMemoryCacheEvictionCallback* pcEvictionCallback, int iDescriptorSize)
{
	CCircularMemoryList::Init(malloc(uiCacheSize + sizeof(SCircularMemoryList)), uiCacheSize + sizeof(SCircularMemoryList), iDescriptorSize);
	mpcEvictionCallback = pcEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Kill(void)
{
	SafeFree(mpsDetail);
	CCircularMemoryList::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Resize(size uiNewCacheSize)
{
	SCircularMemoryList*	pvNewDetail;

	pvNewDetail = (SCircularMemoryList*)realloc(mpsDetail, uiNewCacheSize + sizeof(SCircularMemoryList));
	if (pvNewDetail)
	{
		Remap(pvNewDetail, uiNewCacheSize + sizeof(SCircularMemoryList));
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not realloc.");
		mpvCache = NULL;
		mpsDetail = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryCache::PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size						iCachedSize;
	size						iRemainingAfterLast;
	SMemoryCacheDescriptor*		psTail;

	iCachedSize = miDescriptorSize + pcPreAllocationResult->muiSize;
	if (iCachedSize > mpsDetail->muiCacheSize)
	{
		return false;
	}

	iRemainingAfterLast = RemainingAfterTail();
	if (!IsEmpty())
	{
		if (iCachedSize <= iRemainingAfterLast)
		{
			psTail = CCircularMemoryList::GetLast();
			psCacheBasedDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(psTail, miDescriptorSize + psTail->uiSize);
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psCacheBasedDescriptor = mpvCache;
		}
		FindOverlapping(psCacheBasedDescriptor, iCachedSize, &pcPreAllocationResult->mapEvictedCacheDescriptors);
	}
	else
	{
		psCacheBasedDescriptor = mpvCache;
	}
	pcPreAllocationResult->miCachedSize = iCachedSize;
	pcPreAllocationResult->mpsDescriptor = psCacheBasedDescriptor;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::PostAllocate(CMemoryCacheAllocation* pcPreAllocated)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	SMemoryCacheDescriptor*		psLastOverlap;
	SMemoryCacheDescriptor*		psFirstOverlap;
	SMemoryCacheDescriptor*		psFirstPrev;

	if (pcPreAllocated->miCachedSize > mpsDetail->muiCacheSize)
	{
		return NULL;
	}

	if (pcPreAllocated->HasOverlaps())
	{
		psLastOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(pcPreAllocated->mapEvictedCacheDescriptors.NumElements() -1));
		psFirstOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(0));
		
		if (IsLast(psLastOverlap))
		{
			psCacheBasedDescriptor = OneAllocation();  //If the last overlapping cache descriptor points to the last cache descriptor in the cache then everything is being evicted.
		}
		else
		{
			psCacheBasedDescriptor = pcPreAllocated->mpsDescriptor;

			psFirstPrev = CCircularMemoryList::GetPrev(psFirstOverlap);
			psFirstPrev->psNext = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);

			mpsDetail->mpsTail = MapFromCacheBasedToZeroBased(psCacheBasedDescriptor);
			mpsDetail->mpsHead = MapFromCacheBasedToZeroBased(CCircularMemoryList::GetNext(psLastOverlap));
			CCircularMemoryList::GetFirst()->psPrev = mpsDetail->mpsTail;

			CCircularMemoryList::GetLast()->psNext = mpsDetail->mpsHead;
			CCircularMemoryList::GetLast()->psPrev = MapFromCacheBasedToZeroBased(psFirstPrev);
		}
	}
	else
	{
		psCacheBasedDescriptor = InsertNext(pcPreAllocated->mpsDescriptor);
	}

	psCacheBasedDescriptor->uiSize = pcPreAllocated->muiSize;
	return GetData(psCacheBasedDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Allocate(size uiDataSize)
{
	CMemoryCacheAllocation	cPreAllocation;
	bool					bResult;
	void*					pvCache;
	int						i;
	int						iNumEvictions;
	SMemoryCacheDescriptor* psDescriptor;
	void*					pvEvictedData;

	cPreAllocation.Init(uiDataSize);

	bResult = PreAllocate(&cPreAllocation);
	if (!bResult)
	{
		cPreAllocation.Kill();
		return NULL;
	}

	if (mpcEvictionCallback)
	{
		iNumEvictions = cPreAllocation.NumElements();
		for (i = 0; i < iNumEvictions; i++)
		{
			psDescriptor = cPreAllocation.Get(i);
			pvEvictedData = GetData(psDescriptor);
			mpcEvictionCallback->CacheDataEvicted(pvEvictedData, psDescriptor);
		}
	}

	pvCache = PostAllocate(&cPreAllocation);
	cPreAllocation.Kill();

	return pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryCache::CanCache(size uiDataSize)
{
	return (miDescriptorSize + uiDataSize) <= mpsDetail->muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::FindOverlapping(SMemoryCacheDescriptor* psCachedBasedNew, size uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors)
{
	SMemoryCacheDescriptor*	psCacheBasedNext;

	psCacheBasedNext = CCircularMemoryList::GetFirst();
	for (;;)
	{
		if (Overlaps(psCachedBasedNew, uiNewSize, psCacheBasedNext))
		{
			pasOverlappingCacheDescriptors->Add(psCacheBasedNext);
			psCacheBasedNext = CCircularMemoryList::GetNext(psCacheBasedNext);
			if (IsFirst(psCacheBasedNext))
			{
				return;
			}
		}
		else
		{
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::StartIteration(void)
{
	SMemoryCacheDescriptor* psDesc;
	void*					pvData;

	psDesc = CCircularMemoryList::StartIteration();
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Iterate(void* psCurrent)
{
	SMemoryCacheDescriptor* psDesc; 
	void*					pvData;

	psDesc = CCircularMemoryList::GetDescriptorNoRemap(psCurrent);
	psDesc = CCircularMemoryList::Iterate(psDesc);
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetFirst(void)
{
	SMemoryCacheDescriptor* psDesc;
	void*					pvData;

	psDesc = CCircularMemoryList::GetFirst();
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetLast(void)
{
	SMemoryCacheDescriptor* psDesc; 
	void*					pvData;

	psDesc = CCircularMemoryList::GetLast();
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetNext(void* psCurrent)
{
	SMemoryCacheDescriptor* psDesc; 
	void*					pvData;

	psDesc = CCircularMemoryList::GetDescriptorNoRemap(psCurrent);
	psDesc = CCircularMemoryList::GetNext(psDesc);
	pvData = GetData(psDesc); 
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetPrev(void* psCurrent)
{
	SMemoryCacheDescriptor* psDesc;
	void* pvData;

	psDesc = CCircularMemoryList::GetDescriptorNoRemap(psCurrent);
	psDesc = CCircularMemoryList::GetPrev(psDesc);
	pvData = GetData(psDesc);
	return pvData;
}


