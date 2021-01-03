/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
void CMemoryCache::Init(size_t uiCacheSize, CMemoryCacheEvictionCallback* pcEvictionCallback, int iDescriptorSize)
{
	CCircularMemoryList::Init(malloc(uiCacheSize), uiCacheSize, iDescriptorSize);
	mpcEvictionCallback = pcEvictionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Kill(void)
{
	SafeFree(mpvCache);
	CCircularMemoryList::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Resize(size_t uiNewCacheSize)
{
	void*	pvNewCache;
	void*	pvOldCache;

	pvOldCache = mpvCache;
	pvNewCache = malloc(uiNewCacheSize);
	Remap(pvNewCache, uiNewCacheSize);

	free(pvOldCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult)
{
	SMemoryCacheDescriptor*		psDescriptor;
	size_t						iCachedSize;
	size_t						iRemainingAfterLast;

	iCachedSize = miDescriptorSize + pcPreAllocationResult->muiSize;
	if (iCachedSize > muiCacheSize)
	{
		return FALSE;
	}

	iRemainingAfterLast = RemainingAfterTail();
	if (!IsEmpty())
	{
		if (iCachedSize <= iRemainingAfterLast)
		{
			psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, miDescriptorSize + mpsTail->uiSize);
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psDescriptor = (SMemoryCacheDescriptor*)mpvCache;
		}
		FindOverlapping(psDescriptor, iCachedSize, &pcPreAllocationResult->mapEvictedCacheDescriptors);
	}
	else
	{
		psDescriptor = (SMemoryCacheDescriptor*)mpvCache;
	}
	pcPreAllocationResult->miCachedSize = iCachedSize;
	pcPreAllocationResult->mpsDescriptor = psDescriptor;  

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::PostAllocate(CMemoryCacheAllocation* pcPreAllocated)
{
	SMemoryCacheDescriptor*		psDescriptor;
	SMemoryCacheDescriptor*		psLastOverlap;
	SMemoryCacheDescriptor*		psFirstOverlap;
	SMemoryCacheDescriptor*		psFirstPrev;

	if (pcPreAllocated->miCachedSize > muiCacheSize)
	{
		return NULL;
	}

	if (pcPreAllocated->HasOverlaps())
	{
		psLastOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(pcPreAllocated->mapEvictedCacheDescriptors.NumElements() -1));
		psFirstOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(0));
		
		if (psLastOverlap == mpsTail)
		{
			psDescriptor = OneAllocation();  //If the last overlapping cache descriptor points to the last cache descriptor in the cache then everything is being evicted.
		}
		else
		{
			psDescriptor = pcPreAllocated->mpsDescriptor;

			psFirstPrev = CCircularMemoryList::GetPrev(psFirstOverlap);
			psFirstPrev->psNext = psDescriptor;

			mpsTail = psDescriptor;
			mpsHead = CCircularMemoryList::GetNext(psLastOverlap);
			mpsHead->psPrev = mpsTail;

			mpsTail->psNext = mpsHead;
			mpsTail->psPrev = psFirstPrev;
		}
	}
	else
	{
		psDescriptor = InsertNext(pcPreAllocated->mpsDescriptor);
	}

	psDescriptor->uiSize = pcPreAllocated->muiSize;
	return GetData(psDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Allocate(size_t uiDataSize)
{
	CMemoryCacheAllocation	cPreAllocation;
	BOOL					bResult;
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
BOOL CMemoryCache::CanCache(size_t uiDataSize)
{
	return (miDescriptorSize + uiDataSize) <= muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::FindOverlapping(void* pvNew, size_t uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors)
{
	SMemoryCacheDescriptor*	psNext;

	psNext = mpsHead;
	for (;;)
	{
		if (Overlaps(pvNew, uiNewSize, psNext))
		{
			pasOverlappingCacheDescriptors->Add(psNext);
			psNext = psNext->psNext;
			if (psNext == mpsHead)
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

	psDesc = CCircularMemoryList::GetDescriptor(psCurrent);
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

	psDesc = CCircularMemoryList::GetDescriptor(psCurrent);
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

	psDesc = CCircularMemoryList::GetDescriptor(psCurrent);
	psDesc = CCircularMemoryList::GetPrev(psDesc);
	pvData = GetData(psDesc);
	return pvData;
}


