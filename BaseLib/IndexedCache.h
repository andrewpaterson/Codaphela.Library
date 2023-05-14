/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __INDEXED_CACHE_H__
#define __INDEXED_CACHE_H__
#include "MemoryCache.h"
#include "IndexedDataDescriptor.h"
#include "IndexedCacheResult.h"


#define CACHE_DESCRIPTOR_FLAG_DIRTY		0x02


struct SIndexedCacheDescriptor : public SMemoryCacheDescriptor
{
	unsigned int				iFlags;
	OIndex						oi;
};


class CIndexedCache
{
protected:
	CMemoryCache				mcCache;

public:
	void						Init(size_t iCacheSize, CMemoryCacheEvictionCallback* pcEvictionCallback);
	void						Zero(void);
	void						Kill(void);

	CIndexedCacheResult			Allocate(OIndex oi, unsigned uiDataSize);
	void						Clear(void);
	void						Invalidate(void* pcCache);
	void						SetDirty(void* pvCache);
	void*						GetCache(SIndexedCacheDescriptor* psDescriptor);

	SIndexedCacheDescriptor*	GetDescriptor(void* pvData);  // <-- Hmmmm....
	void*						StartIteration(void);
	void*						Iterate(void* psCurrent);
	int							NumCached(void);
	int							NumCached(int iSize);

	int							GetIndexCacheDescritorSize(void);
	size_t						GetCacheSize(void);
	size_t						GetAllocatedSize(void);
	bool						CanCache(unsigned int uiDataSize);

	void*						TestGetDescriptor(OIndex oi);
	void						Dump(void);
};


#endif // !__INDEXED_CACHE_H__

