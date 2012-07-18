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
#include "IndexDescriptor.h"
#include "IndexedCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Init(unsigned int uiCacheSize)
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
BOOL CIndexedCache::Allocate(CIndexDescriptor* pcDesc, void* pvData)
{
	void*	pvCache;
	
	pvCache = Allocate(pcDesc);
	if (pvCache)
	{
		memcpy_fast(pvCache, pvData, pcDesc->GetDataSize());
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedCache::Allocate(CIndexDescriptor* pcDesc)
{
	void*						pvCache;
	SIndexedCacheDescriptor*	psCacheDesc;
	CMemoryCacheAllocation		cPreAllocated;

	cPreAllocated.Init(pcDesc->GetDataSize());
	mcCache.PreAllocate(&cPreAllocated);
	pvCache = mcCache.Allocate(&cPreAllocated);
	cPreAllocated.Kill();

	if (!pvCache)
	{
		pcDesc->Cache(NULL);
		return NULL;
	}

	psCacheDesc = (SIndexedCacheDescriptor*)RemapSinglePointer(pvCache, -(int)(sizeof(SIndexedCacheDescriptor)));

	psCacheDesc->oi = pcDesc->GetIndex();
	if (!pcDesc->HasFile())
	{
		psCacheDesc->iFlags |= CACHE_DESCRIPTOR_FLAG_DIRTY;
	}

	//CIndexDescriptor (pcDesc) adjusted here.
	pcDesc->Cache(pvCache);

	return pvCache;
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
void CIndexedCache::Invalidate(CIndexDescriptor* pcDesc)
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
int CIndexedCache::NumIgnored(void)
{
	return mcCache.NumIgnored();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedCache::Update(CIndexDescriptor* pcDesc, void* pvData)
{
	SIndexedCacheDescriptor*	psCacheIndex;
	void*						pvCache;
	unsigned int				iDataSize;

	//Assumes that the test to make sure this is in the cache has already been done.
	pvCache = pcDesc->GetCache();
	iDataSize = pcDesc->GetDataSize();
	psCacheIndex = GetHeader(pvCache);
	if (memcmp(pvCache, pvData, iDataSize) != 0)
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
