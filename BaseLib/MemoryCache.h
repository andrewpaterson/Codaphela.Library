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
#include "ArrayPointer.h"


#define CACHE_DESCRIPTOR_FLAG_VALID		0x01


struct SMemoryCacheDescriptor
{
	SMemoryCacheDescriptor*		psNext;
	unsigned int				iDataSize;  //Not including the sizeof this.
	unsigned int				iFlags;
};


class CMemoryCache
{
public:
	int							miDescriptorSize;
	void*						mpvCache;
	unsigned int				muiCacheSize;
	SMemoryCacheDescriptor*		mpsLast;
	SMemoryCacheDescriptor*		mpsFirst;

	void						Init(void);
	void						Init(unsigned int iCacheSize, int iDescriptorSize = sizeof(SMemoryCacheDescriptor));
	void						Kill(void);

	BOOL						PreAllocate(int iDataSize, CArrayPointer* papEvictedCacheDescriptors);
	void*						Allocate(int iDataSize);
	unsigned int				Remaining(void);
	void						FindOverlapping(void* pvNew, unsigned int uiNewSize, CArrayPointer* papIndexedCacheDescriptors);
	SMemoryCacheDescriptor*		FindNewFirst(void* pvNew, unsigned int uiNewSize);
	BOOL						Overlaps(void* pvNew, unsigned int uiNewSize, SMemoryCacheDescriptor* psExisting);
	void						Clear(void);
	void						Invalidate(SMemoryCacheDescriptor* psCacheDesc);

	SMemoryCacheDescriptor*		GetFirst(void);
	SMemoryCacheDescriptor*		GetNext(SMemoryCacheDescriptor* psCurrent);
	int							NumCached(void);
	int							NumIgnored(void);

	void						Dump(void);
};


#endif // __MEMORY_CACHE_H__

