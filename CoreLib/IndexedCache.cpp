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
CIndexedCacheResult CIndexedCache::Allocate(OIndex oi, unsigned uiDataSize, CMemoryCacheAllocation* pcPreAllocated)
{
	void*						pvCache;
	SIndexedCacheDescriptor*	psCacheDesc;
	CIndexedCacheResult			cResult;

	pvCache = mcCache.Allocate(pcPreAllocated);

	if (!pvCache)
	{
		cResult.Fail();
		return cResult;
	}

	
	psCacheDesc = GetHeader(pvCache);
	psCacheDesc->oi = oi;

	if (uiDataSize == psCacheDesc->iDataSize)
	{
		cResult.Succeed(pvCache);
		return cResult;
	}
	else
	{
		cResult.Fail();
		return cResult;
	}
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
void CIndexedCache::Invalidate(void* pcCache)
{
	SIndexedCacheDescriptor*	psDescriptor;

	if (pcCache)
	{
		psDescriptor = GetHeader(pcCache);
		mcCache.Deallocate(psDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::Deallocate(SIndexedCacheDescriptor* psDescriptor)
{
	mcCache.Deallocate(psDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedCache::SetDirty(void* pvCache)
{
	SIndexedCacheDescriptor* psCacheDesc;

	psCacheDesc = GetHeader(pvCache);
	psCacheDesc->iFlags |= CACHE_DESCRIPTOR_FLAG_DIRTY;
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
BOOL CIndexedCache::Update(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	SIndexedCacheDescriptor*	psCacheIndex;
	void*						pvCache;
	size_t						iDataSize;
	int							iResult;

	//THIS METHOD NEEDS TO BE RETHOUGHT.

	//Assumes that the test to make sure this is in the cache has already been done.
	pvCache = pcDescriptor->GetCache();
	iDataSize = pcDescriptor->GetDataSize();
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
	return (SIndexedCacheDescriptor*)RemapSinglePointer(pvData, -mcCache.miDescriptorSize);
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
size_t CIndexedCache::GetCacheSize(void)
{
	return mcCache.GetCacheSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CIndexedCache::GetAllocatedSize(void)
{
	return mcCache.GetAllocatedSize();
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

