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


class CMemoryCache : public CCircularMemoryList
{
public:
	void	Init(size_t uiCacheSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void	Kill(void);

	BOOL	PreAllocate(CMemoryCacheAllocation* pcPreAllocationResult);
	void*	Allocate(CMemoryCacheAllocation* pcPreAllocated);
	void*	QuickAllocate(int iDataSize);

	BOOL	CanCache(unsigned int uiDataSize);

	void	FindOverlapping(void* pvNew, size_t uiNewSize, CArrayVoidPtr* pasOverlappingCacheDescriptors);
	BOOL	Overlaps(void* pvNew, size_t uiNewSize, SMemoryCacheDescriptor* psExisting);

protected:
	SMemoryCacheDescriptor*		OneAllocation(void);
	size_t						RemainingAfterTail(void);
};


#endif // __MEMORY_CACHE_H__

