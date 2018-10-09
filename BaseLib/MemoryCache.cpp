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
#include "DataMacro.h"
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
void CMemoryCache::Init(void)
{
	miDescriptorSize = 0;
	muiCacheSize = 0;
	mpvCache = NULL;
	mpsTail = NULL;
	mpsHead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Init(size_t uiCacheSize, int iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;
	muiCacheSize = uiCacheSize;
	mpvCache = malloc(muiCacheSize);

	Zero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Kill(void)
{
	SafeFree(mpvCache);
	mpsTail = NULL;
	mpsHead = NULL;
	muiCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	size_t						iCachedSize;
	size_t						iRemaining;

	iCachedSize = miDescriptorSize + pcPreAllocationResult->miDataSize;
	if (iCachedSize > muiCacheSize)
	{
		return FALSE;
	}

	iRemaining = RemainingAfterLast();
	if (!IsEmpty())
	{
		if (iCachedSize <= iRemaining)
		{
			psCacheDesc = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, (int)(miDescriptorSize + mpsTail->iDataSize));
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;
		}
		FindOverlapping(psCacheDesc, iCachedSize, &pcPreAllocationResult->mapEvictedCacheDescriptors);
	}
	else
	{
		psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;
	}
	pcPreAllocationResult->miCachedSize = iCachedSize;
	pcPreAllocationResult->mpsCacheDesc = psCacheDesc;  

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Allocate(CMemoryCacheAllocation* pcPreAllocated)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	void*						pvCache;
	SMemoryCacheDescriptor*		psLastOverlap;
	SMemoryCacheDescriptor*		psFirstOverlap;
	SMemoryCacheDescriptor*		psFirstPrev;

	if (pcPreAllocated->miCachedSize > muiCacheSize)
	{
		return NULL;
	}

	psCacheDesc = pcPreAllocated->mpsCacheDesc;

	if (pcPreAllocated->HasOverlaps())
	{
		psLastOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(pcPreAllocated->mapEvictedCacheDescriptors.NumElements() -1));
		psFirstOverlap = (SMemoryCacheDescriptor*)(pcPreAllocated->mapEvictedCacheDescriptors.GetPtr(0));
		
		if (psLastOverlap == mpsTail)
		{
			//@todo - Shouldn't reassign psCacheDesc here.
			psCacheDesc = OneAllocation();  //If the last overlapping cache descriptor points to the last cache descriptor in the cache then everything is being evicted.
		}
		else
		{
			psFirstPrev = GetPrev(psFirstOverlap);
			psFirstPrev->psNext = psCacheDesc;

			mpsTail = psCacheDesc;
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
			//@todo - Shouldn't reassign psCacheDesc here.
			psCacheDesc = OneAllocation();
		}
		else
		{
			mpsTail->psNext = psCacheDesc;
			mpsHead->psPrev = psCacheDesc;

			psCacheDesc->psNext = mpsHead;
			psCacheDesc->psPrev = mpsTail;

			mpsTail = psCacheDesc;
		}
	}

	psCacheDesc->iDataSize = pcPreAllocated->miDataSize;

	//@todo - Should return psCacheDesc here.
	pvCache = RemapSinglePointer(psCacheDesc, miDescriptorSize);
	return pvCache;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::OneAllocation(void)
{
	SMemoryCacheDescriptor*		psCacheDesc;

	psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;

	mpsTail = psCacheDesc;
	mpsHead = psCacheDesc;

	psCacheDesc->psNext = psCacheDesc;
	psCacheDesc->psPrev = psCacheDesc;

	return psCacheDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::GetData(SMemoryCacheDescriptor* psCacheDesc)
{
	return RemapSinglePointer(psCacheDesc, miDescriptorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::QuickAllocate(int iDataSize)
{
	CMemoryCacheAllocation	cPreAllocation;
	BOOL					bResult;
	void*					pvResult;

	//Evicted data is just trashed.

	cPreAllocation.Init(iDataSize, miDescriptorSize);

	bResult = PreAllocate(&cPreAllocation);
	if (!bResult)
	{
		cPreAllocation.Kill();
		return NULL;
	}

	pvResult = Allocate(&cPreAllocation);
	cPreAllocation.Kill();

	return pvResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Deallocate(void* pvData)
{
	SMemoryCacheDescriptor*	psDescriptor;

	if (pvData)
	{
		psDescriptor = DataGetHeader<SMemoryCacheDescriptor, void>(pvData);
		Deallocate(psDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Deallocate(SMemoryCacheDescriptor* psDescriptor)
{
	if ((psDescriptor == mpsHead) && (psDescriptor == mpsTail))
	{
		Zero();
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
size_t CMemoryCache::GetCacheSize(void)
{
	return muiCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CMemoryCache::GetAllocatedSize(void)
{
	SMemoryCacheDescriptor*	psIter;
	size_t					tSize;
		
	tSize = 0;
	psIter = StartIteration();
	while (psIter)
	{
		tSize += psIter->iDataSize + sizeof(SMemoryCacheDescriptor);
		psIter = Iterate(psIter);
	}
	return tSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CMemoryCache::RemainingAfterLast(void)
{
	size_t		iAllocated;

	if (IsEmpty())
	{
		return muiCacheSize;
	}
	else
	{
		iAllocated = ((int)(size_t) mpsTail - (int)(size_t) mpvCache);
		iAllocated += (mpsTail->iDataSize + miDescriptorSize);
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
	uiNextEnd = uiNextStart + psExisting->iDataSize + miDescriptorSize - 1;

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
SMemoryCacheDescriptor* CMemoryCache::FindNewFirst(void* pvNew, size_t uiNewSize)
{
	SMemoryCacheDescriptor*	psNext;

	psNext = mpsHead;
	for (;;)
	{
		if (Overlaps(pvNew, uiNewSize, psNext))
		{
			psNext = psNext->psNext;
			if (psNext == mpsHead)
			{
				return psNext;
			}
		}
		else
		{
			return psNext;
		}
	}
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
void CMemoryCache::Clear(void)
{
	mpsTail = NULL;
	mpsHead = NULL;
	memset_fast(mpvCache, 0, muiCacheSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::GetFirst(void)
{
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::GetNext(SMemoryCacheDescriptor* psCurrent)
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
SMemoryCacheDescriptor* CMemoryCache::GetPrev(SMemoryCacheDescriptor* psCurrent)
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
SMemoryCacheDescriptor* CMemoryCache::GetLast(void)
{
	return mpsTail;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryCache::NumCached(void)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	int							iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psCacheDesc = StartIteration();
	while (psCacheDesc)
	{
		iNum++;
		psCacheDesc = Iterate(psCacheDesc);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryCache::NumCached(int iSize)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	int							iNum;

	if (IsEmpty())
	{
		return 0;
	}

	iNum = 0;
	psCacheDesc = StartIteration();
	while (psCacheDesc)
	{
		if (psCacheDesc->iDataSize == iSize)
		{
			iNum++;
		}
		psCacheDesc = Iterate(psCacheDesc);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::IsEmpty(void)
{
	return mpsTail == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::StartIteration(void)
{
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::Iterate(SMemoryCacheDescriptor* psCurrent)
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
void CMemoryCache::Zero(void)
{
	//Trashes whole cache.

	memset_fast(mpvCache, 0, muiCacheSize);
	mpsTail = NULL;
	mpsHead = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Dump(void)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	CChars						sz;
	char*						pvData;
	int							iLen;

	sz.Init();

	sz.Append("Data Cache (");
	sz.Append(NumCached());
	sz.Append(")\n---------------\n");

	psCacheDesc = StartIteration();
	while (psCacheDesc)
	{
		pvData = (char*)RemapSinglePointer(psCacheDesc, miDescriptorSize);
		iLen = psCacheDesc->iDataSize;

		sz.Append("(Ln:");
		sz.AppendHexHiLo(&iLen, 4);
		sz.Append(" Da:");
		sz.AppendHexHiLo(&psCacheDesc, 4);
		sz.Append(" Nx:");
		sz.AppendHexHiLo(&psCacheDesc->psNext, 4);
		sz.Append(") ");

		sz.AppendData(pvData, iLen, 80);
		sz.AppendNewLine();
		psCacheDesc = Iterate(psCacheDesc);
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


SMemoryCacheDescriptor* CMemoryCache::TestGetLast(void) { return mpsTail; }
SMemoryCacheDescriptor* CMemoryCache::TestGetFirst(void) { return mpsHead; }

