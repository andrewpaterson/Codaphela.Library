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
#include "CircularMemoryList.h"
#include "MemoryCacheAllocation.h"
#include "MemoryCacheEvictionCallback.h"


class CMemoryCache : public CCircularMemoryList
{
protected:
	CMemoryCacheEvictionCallback*	mpcEvictionCallback;

public:
	void	Init(size_t uiCacheSize, CMemoryCacheEvictionCallback* pcEvictionCallback, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void	Kill(void);
	void	Resize(size_t uiNewCacheSize);

	void*	Allocate(size_t uiDataSize);

	BOOL	CanCache(size_t uiDataSize);

	void*	StartIteration(void);
	void*	Iterate(void* psCurrent);

	void*	GetFirst(void);
	void*	GetLast(void);
	void*	GetNext(void* psCurrent);
	void*	GetPrev(void* psCurrent);

protected:
	void	FindOverlapping(void* pvNew, size_t uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors);
	BOOL	PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult);
	void*	PostAllocate(CMemoryCacheAllocation* pcPreAllocated);
};


#endif // __MEMORY_CACHE_H__

