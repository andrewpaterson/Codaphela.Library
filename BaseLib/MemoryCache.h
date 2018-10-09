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
#ifndef __MEMORY_CACHE_H__
#define __MEMORY_CACHE_H__
#include "MemoryCacheAllocation.h"


#define CACHE_DESCRIPTOR_FLAG_VALID		0x01


//Memory cache should be built on the link part of LinkListBlock.
//CACHE_DESCRIPTOR_FLAG_VALID is never set FALSE
class CMemoryCache
{
public:
	int							miDescriptorSize;

private:
	void*						mpvCache;
	size_t						muiCacheSize;

	SMemoryCacheDescriptor*		mpsTail;
	SMemoryCacheDescriptor*		mpsHead;

public:
	void						Init(void);
	void						Init(size_t iCacheSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	BOOL						PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult);
	void*						Allocate(CMemoryCacheAllocation* pcPreAllocated);
	void*						QuickAllocate(int iDataSize);

	void						Deallocate(void* pvData);

	size_t						GetCacheSize(void);
	size_t						GetAllocatedSize(void);

	size_t						RemainingAfterLast(void);
	void						FindOverlapping(void* pvNew, size_t uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors);
	SMemoryCacheDescriptor*		FindNewFirst(void* pvNew, size_t uiNewSize);
	BOOL						Overlaps(void* pvNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting);
	void						Clear(void);
	void						Invalidate(SMemoryCacheDescriptor* psCacheDesc);

	SMemoryCacheDescriptor*		GetFirst(void);
	SMemoryCacheDescriptor*		GetNext(SMemoryCacheDescriptor* psCurrent);
	SMemoryCacheDescriptor*		GetPrev(SMemoryCacheDescriptor* psCurrent);
	SMemoryCacheDescriptor*		GetLast(void);
	int							NumCached(void);
	int							NumIgnored(void);
	BOOL						IsEmpty(void);
	int							NumCached(int iSize);

	void*						GetData(SMemoryCacheDescriptor* psCacheDesc);

	void						Zero(void);
	void						Dump(void);
	SMemoryCacheDescriptor*		TestGetFirst(void);
	SMemoryCacheDescriptor*		TestGetLast(void);

	SMemoryCacheDescriptor*		StartIteration(void);
	SMemoryCacheDescriptor*		Iterate(SMemoryCacheDescriptor* psCurrent);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
};


#endif // __MEMORY_CACHE_H__

