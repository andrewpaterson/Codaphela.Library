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
	mpvCache = malloc(uiCacheSize + sizeof(SCircularMemoryList));
	mcMemoryList.Init(mpvCache, uiCacheSize + sizeof(SCircularMemoryList), iDescriptorSize);
	mpcEvictionCallback = pcEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Kill(void)
{
	SafeFree(mpvCache);
	mcMemoryList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Resize(size uiNewCacheSize)
{
	SCircularMemoryList*	pvNewDetail;

	mpvCache = realloc(mpvCache, uiNewCacheSize + sizeof(SCircularMemoryList));
	pvNewDetail = (SCircularMemoryList*)mpvCache;
	if (pvNewDetail)
	{
		mcMemoryList.Remap(pvNewDetail, uiNewCacheSize + sizeof(SCircularMemoryList));
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Could not realloc.");
		mcMemoryList.Remap((SCircularMemoryList*)NULL, 0);
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
	size						iDescriptorSize;
	size						iOverlapping;

	iDescriptorSize = mcMemoryList.GetDescriptorSize();
	iCachedSize = iDescriptorSize + pcPreAllocationResult->muiSize;
	if (iCachedSize > mcMemoryList.GetCacheSize())
	{
		return false;
	}

	iRemainingAfterLast = mcMemoryList.RemainingAfterTail();
	if (!mcMemoryList.IsEmpty())
	{
		iOverlapping = 0;
		if (iCachedSize <= iRemainingAfterLast)
		{
			psTail = mcMemoryList.GetLastDescriptor();
			psCacheBasedDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(psTail, iDescriptorSize + psTail->GetSize());
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psCacheBasedDescriptor = mcMemoryList.GetCache();
		}
		iOverlapping = mcMemoryList.FindOverlapping(psCacheBasedDescriptor, iCachedSize, &pcPreAllocationResult->mapEvictedCacheDescriptors);
	}
	else
	{
		psCacheBasedDescriptor = mcMemoryList.GetCache();
	}

	pcPreAllocationResult->miCachedSize = iCachedSize;
	pcPreAllocationResult->mpsDescriptor = psCacheBasedDescriptor;  //psCacheBasedDescriptor will look like garbage until it is initialised in PostAllocate.

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

	mcMemoryList.ValidateEnds();

	if (pcPreAllocated->miCachedSize > mcMemoryList.GetCacheSize())
	{
		return NULL;
	}

	if (pcPreAllocated->HasOverlaps())
	{
		psLastOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(pcPreAllocated->mapEvictedCacheDescriptors.NumElements() -1));
		psFirstOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(0));
		
		if (mcMemoryList.IsLast(psLastOverlap))
		{
			psCacheBasedDescriptor = mcMemoryList.OneAllocation();  //If the last overlapping cache descriptor points to the last cache descriptor in the cache then everything is being evicted.
		}
		else
		{
			psCacheBasedDescriptor = pcPreAllocated->mpsDescriptor;

			mcMemoryList.SetEndsForPostAllocate(psCacheBasedDescriptor, psFirstOverlap, psLastOverlap);
		}
	}
	else
	{
		psCacheBasedDescriptor = mcMemoryList.InsertNext(pcPreAllocated->mpsDescriptor);
	}

	psCacheBasedDescriptor->SetSize(pcPreAllocated->muiSize);
	return mcMemoryList.GetData(psCacheBasedDescriptor);
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
			pvEvictedData = mcMemoryList.GetData(psDescriptor);
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
size CMemoryCache::GetSize(void* pvData)
{
	return mcMemoryList.GetSize(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::StartIteration(void)
{
	return mcMemoryList.StartIteration();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Iterate(void* psCurrent)
{
	return mcMemoryList.Iterate(psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetFirst(void)
{
	return mcMemoryList.GetFirst();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetLast(void)
{
	return mcMemoryList.GetLast();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetNext(void* psCurrent)
{
	return mcMemoryList.GetNext(psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetPrev(void* psCurrent)
{
	return mcMemoryList.GetPrev(psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Clear(void)
{
	mcMemoryList.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Deallocate(void* pvCache)
{
	mcMemoryList.Deallocate(pvCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryCache::NumElements(void)
{
	return mcMemoryList.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryCache::NumElements(size iSize)
{
	return mcMemoryList.NumElements(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryCache::GetDescriptorSize(void)
{
	return mcMemoryList.GetDescriptorSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryCache::GetCacheSize(void)
{
	return mcMemoryList.GetCacheSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryCache::GetAllocatedSize(void)
{
	return mcMemoryList.GetAllocatedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryCache::CanCache(size uiDataSize)
{
	return mcMemoryList.CanCache(uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemoryCache::ValidateCache(void)
{
	return mcMemoryList.ValidateCache();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Dump(void)
{
	mcMemoryList.Dump();
}

