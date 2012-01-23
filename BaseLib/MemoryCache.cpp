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
#include "MemoryCache.h"
#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "FastMemcpy.h"
#include "FastMemcmp.h"
#include "FastMemset.h"
#include "Chars.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Init(void)
{
	miDescriptorSize = 0;
	muiCacheSize = 0;
	mpvCache = NULL;
	mpsLast = NULL;
	mpsFirst = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Init(unsigned int uiCacheSize, int iDescriptorSize)
{
	miDescriptorSize = iDescriptorSize;
	muiCacheSize = uiCacheSize;
	mpvCache = malloc(muiCacheSize);
	memset_fast(mpvCache, 0, muiCacheSize);
	mpsLast = NULL;
	mpsFirst = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Kill(void)
{
	SafeFree(mpvCache);
	mpsLast = NULL;
	mpsFirst = NULL;
	muiCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::PreAllocate(int iDataSize, CArrayPointer* papEvictedCacheDescriptors)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	unsigned int				iCachedSize;
	unsigned int				iRemaining;

	papEvictedCacheDescriptors->Init(32768);

	iCachedSize = miDescriptorSize + iDataSize;
	if (iCachedSize > muiCacheSize)
	{
		return FALSE;
	}

	if (mpsLast)
	{
		iRemaining = Remaining();
		if (iCachedSize > iRemaining)
		{
			//Cycle back to the beginning of the cache.
			psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;
		}
		else
		{
			psCacheDesc = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsLast, iCachedSize+1);
		}
		FindOverlapping(psCacheDesc, iCachedSize, papEvictedCacheDescriptors);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryCache::Allocate(int iDataSize)
{
	SMemoryCacheDescriptor*		psCacheDesc;
	unsigned int				iCachedSize;
	unsigned int				iRemaining;
	void*						pvCache;

	iCachedSize = miDescriptorSize + iDataSize;
	if (iCachedSize > muiCacheSize)
	{
		return NULL;
	}

	if (mpsLast)
	{
		iRemaining = Remaining();
		if (iCachedSize > iRemaining)
		{
			//Cycle back to the beginning of the cache.
			psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;
		}
		else
		{
			psCacheDesc = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsLast, mpsLast->iDataSize + miDescriptorSize);
		}
		mpsFirst = FindNewFirst(psCacheDesc, iCachedSize);
		mpsLast->psNext = psCacheDesc;
		psCacheDesc->psNext = mpsFirst;
	}
	else
	{
		psCacheDesc = (SMemoryCacheDescriptor*)mpvCache;
		mpsFirst = psCacheDesc;
		psCacheDesc->psNext = psCacheDesc;
	}

	mpsLast = psCacheDesc;

	psCacheDesc->iDataSize = iDataSize;
	psCacheDesc->iFlags = CACHE_DESCRIPTOR_FLAG_VALID;

	pvCache = RemapSinglePointer(psCacheDesc, miDescriptorSize);
	return pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CMemoryCache::Remaining(void)
{
	unsigned int		iAllocated;

	if (mpsLast)
	{
		iAllocated = ((int)(ENGINE_SIZE_T) mpsLast - (int)(ENGINE_SIZE_T) mpvCache);
		iAllocated += (mpsLast->iDataSize + miDescriptorSize);
		return muiCacheSize - iAllocated;
	}
	else
	{
		return muiCacheSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryCache::Overlaps(void* pvNew, unsigned int uiNewSize, SMemoryCacheDescriptor* psExisting)
{
	unsigned int	uiNewStart;
	unsigned int	uiNewEnd;  //Inclusive

	unsigned int	uiNextStart;
	unsigned int	uiNextEnd; //Inclusive

	uiNewStart = (unsigned int)(ENGINE_SIZE_T) pvNew;
	uiNewEnd = uiNewStart + uiNewSize - 1;

	uiNextStart = (unsigned int)(ENGINE_SIZE_T) psExisting;
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
SMemoryCacheDescriptor* CMemoryCache::FindNewFirst(void* pvNew, unsigned int uiNewSize)
{
	SMemoryCacheDescriptor*	psNext;

	psNext = mpsFirst;
	for (;;)
	{
		if (Overlaps(pvNew, uiNewSize, psNext))
		{
			psNext = psNext->psNext;
			if (psNext == mpsFirst)
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
void CMemoryCache::FindOverlapping(void* pvNew, unsigned int uiNewSize, CArrayPointer* papCacheDescriptors)
{
	SMemoryCacheDescriptor*	psNext;

	psNext = mpsFirst;
	for (;;)
	{
		if (Overlaps(pvNew, uiNewSize, psNext))
		{
			if (psNext->iFlags & CACHE_DESCRIPTOR_FLAG_VALID)
			{
				papCacheDescriptors->Add(psNext, 0);
			}
			psNext = psNext->psNext;
			if (psNext == mpsFirst)
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
	mpsLast = NULL;
	mpsFirst = NULL;
	memset_fast(mpvCache, 0, muiCacheSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Invalidate(SMemoryCacheDescriptor* psCacheDesc)
{
	psCacheDesc->iFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemoryCacheDescriptor* CMemoryCache::GetFirst(void)
{
	if (mpsFirst)
	{
		if (!(mpsFirst->iFlags & CACHE_DESCRIPTOR_FLAG_VALID))
		{
			return GetNext(mpsFirst);
		}
	}
	return mpsFirst;
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
	if (!psCurrent->psNext)
	{
		return NULL;
	}
	for (;;)
	{
		if (psCurrent == mpsFirst)
		{
			return NULL;
		}
		if (!(psCurrent->iFlags & CACHE_DESCRIPTOR_FLAG_VALID))
		{
			psCurrent = psCurrent->psNext;
		}
		else
		{
			return psCurrent;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryCache::NumCached(void)
{
	SMemoryCacheDescriptor*	psCacheDesc;
	int							iNum;

	iNum = 0;
	psCacheDesc = GetFirst();
	while (psCacheDesc)
	{
		iNum++;
		psCacheDesc = GetNext(psCacheDesc);
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemoryCache::NumIgnored(void)
{
	SMemoryCacheDescriptor*	psCacheDesc;
	int							iNum;

	iNum = 0;
	psCacheDesc = mpsFirst;
	while (psCacheDesc)
	{
		if (!(psCacheDesc->iFlags & CACHE_DESCRIPTOR_FLAG_VALID))
		{
			iNum++;
		}
		psCacheDesc = psCacheDesc->psNext;
		if (psCacheDesc == mpsFirst)
		{
			psCacheDesc = NULL;
		}
	}
	return iNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryCache::Dump(void)
{
	SMemoryCacheDescriptor*	psCacheDesc;
	CChars						sz;
	char*						pvData;
	int							iLen;

	sz.Init();
	psCacheDesc = GetFirst();

	sz.Append("Data Cache (");
	sz.Append(NumCached());
	sz.Append(":");
	sz.Append(NumIgnored());
	sz.Append(")\n---------------\n");

	while (psCacheDesc)
	{
		pvData = (char*)RemapSinglePointer(psCacheDesc, miDescriptorSize);
		iLen = psCacheDesc->iDataSize;
		sz.AppendData(pvData, 80);
		sz.AppendNewLine();
		psCacheDesc = GetNext(psCacheDesc);
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

