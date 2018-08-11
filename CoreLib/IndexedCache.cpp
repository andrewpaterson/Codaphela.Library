/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/Chars.h"
#include "IndexedDataDescriptor.h"
#include "IndexedCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Init(size_t uiCacheSize)
{
	mcCache.Init(uiCacheSize, sizeof(SIndexedCacheDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Zero(void)
{
	memset(this, 0, sizeof(CIndexedCache));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Kill(void)
{
	mcCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedCache::PreAllocate(CMemoryCacheAllocation* pcResult)
{
	return mcCache.PreAllocate(pcResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedCache::Allocate(OIndex oi, CIndexedDataDescriptor* pcDesc, CMemoryCacheAllocation* pcPreAllocated)
{
	void*						pvCache;
	SIndexedCacheDescriptor*	psCacheDesc;

	pvCache = mcCache.Allocate(pcPreAllocated);

	if (!pvCache)
	{
		pcDesc->Cache(NULL);
		return FALSE;
	}

	psCacheDesc = (SIndexedCacheDescriptor*)RemapSinglePointer(pvCache, -(int)(sizeof(SIndexedCacheDescriptor)));

	psCacheDesc->oi = oi;
	if (!pcDesc->HasFile())
	{
		psCacheDesc->iFlags |= CACHE_DESCRIPTOR_FLAG_DIRTY;
	}

	//CIndexedDataDescriptor (pcDesc) adjusted here.
	pcDesc->Cache(pvCache);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Clear(void)
{
	mcCache.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Invalidate(CIndexedDataDescriptor* pcDesc)
{
	SIndexedCacheDescriptor*	psCacheDesc;

	if (pcDesc->IsCached())
	{
		psCacheDesc = GetHeader(pcDesc->GetCache());
		Invalidate(psCacheDesc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Invalidate(SIndexedCacheDescriptor* psCacheDesc)
{
	mcCache.Invalidate(psCacheDesc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedCache::NumCached(void)
{
	return mcCache.NumCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedCache::NumCached(int iSize)
{
	return mcCache.NumCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedCache::NumIgnored(void)
{
	return mcCache.NumIgnored();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedCache::Update(CIndexedDataDescriptor* pcDesc, void* pvData)
{
	SIndexedCacheDescriptor*	psCacheIndex;
	void*						pvCache;
	size_t						iDataSize;
	int							iResult;

	//Assumes that the test to make sure this is in the cache has already been done.
	pvCache = pcDesc->GetCache();
	iDataSize = pcDesc->GetDataSize();
	psCacheIndex = GetHeader(pvCache);
	iResult = memcmp(pvCache, pvData, iDataSize);
	if (iResult != 0)
	{
		memcpy_fast(pvCache, pvData, iDataSize);
		psCacheIndex->iFlags |= CACHE_DESCRIPTOR_FLAG_DIRTY;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedCacheDescriptor* CIndexedCache::GetHeader(void* pvData)
{
	return (SIndexedCacheDescriptor*)RemapSinglePointer(pvData, -((int)sizeof(SIndexedCacheDescriptor)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedCacheDescriptor* CIndexedCache::StartIteration(void)
{
	return (SIndexedCacheDescriptor*)mcCache.StartIteration();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedCacheDescriptor* CIndexedCache::Iterate(SIndexedCacheDescriptor* psCurrent)
{
	return (SIndexedCacheDescriptor*)mcCache.Iterate((SIndexedCacheDescriptor*)psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedCache::GetIndexCacheDescritorSize(void)
{
	return mcCache.miDescriptorSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedCacheDescriptor* CIndexedCache::TestGetDescriptor(OIndex oi)
{
	SIndexedCacheDescriptor*	psDesc;

	psDesc = StartIteration();
	while (psDesc)
	{
		if (psDesc->oi == oi)
		{
			return psDesc;
		}
		psDesc = Iterate(psDesc);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Dump(void)
{
	return mcCache.Dump();
}

