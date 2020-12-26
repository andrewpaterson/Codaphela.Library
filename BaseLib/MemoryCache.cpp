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
void CMemoryCache::Init(size_t uiCacheSize, int iDescriptorSize)
{
	CCircularMemoryList::Init(malloc(uiCacheSize), uiCacheSize, iDescriptorSize);
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
			psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, (int)(miDescriptorSize + mpsTail->uiSize));
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
void* CMemoryCache::Allocate(CMemoryCacheAllocation* pcPreAllocated)
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

			psFirstPrev = GetPrev(psFirstOverlap);
			psFirstPrev->psNext = psDescriptor;

			mpsTail = psDescriptor;
			mpsHead = GetNext(psLastOverlap);
			mpsHead->psPrev = mpsTail;

			mpsTail->psNext = mpsHead;
			mpsTail->psPrev = psFirstPrev;
		}
	}
	else
	{
		if (IsEmpty())
		{
			psDescriptor = OneAllocation();
		}
		else
		{
			psDescriptor = pcPreAllocated->mpsDescriptor;

			mpsTail->psNext = psDescriptor;
			mpsHead->psPrev = psDescriptor;

			psDescriptor->psNext = mpsHead;
			psDescriptor->psPrev = mpsTail;

			mpsTail = psDescriptor;
		}
	}

	psDescriptor->uiSize = pcPreAllocated->muiSize;
	return GetData(psDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::OneAllocation(void)
{
	SMemoryCacheDescriptor*		psDescriptor;

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
void* CMemoryCache::QuickAllocate(int iDataSize)
{
	CMemoryCacheAllocation	cPreAllocation;
	BOOL					bResult;
	void*					pvCache;

	//Evicted data is just trashed.
	cPreAllocation.Init(iDataSize);

	bResult = PreAllocate(&cPreAllocation);
	if (!bResult)
	{
		cPreAllocation.Kill();
		return NULL;
	}

	pvCache = Allocate(&cPreAllocation);
	cPreAllocation.Kill();

	return pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::CanCache(unsigned int uiDataSize)
{
	return (miDescriptorSize + uiDataSize) <= muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CMemoryCache::RemainingAfterTail(void)
{
	size_t		iAllocated;

	if (IsEmpty())
	{
		return muiCacheSize;
	}
	else
	{
		iAllocated = ((int)(size_t)mpsTail - (int)(size_t)mpvCache);
		iAllocated += (mpsTail->uiSize + miDescriptorSize);
		return muiCacheSize - iAllocated;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::Overlaps(void* pvNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting)
{
	size_t	uiNewStart;
	size_t	uiNewEnd;  //Inclusive

	size_t	uiNextStart;
	size_t	uiNextEnd; //Inclusive

	uiNewStart = (size_t) pvNew;
	uiNewEnd = uiNewStart + uiNewSize - 1;

	uiNextStart = (size_t) psExisting;
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

