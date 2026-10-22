/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __SLAB_ALLOCATOR_H__
#define __SLAB_ALLOCATOR_H__
#include "Mallocator.h"


//Every slab is SLAB_SIZE bytes and starts on a SLAB_SIZE boundary so the slab holding a pointer is found by masking the pointer.
//64KB is the Windows allocation granularity so VirtualAlloc returns slabs with no wasted address space.
#define SLAB_SIZE	((size)0x10000)


class CSlabAllocator : public CMallocator
{
CONSTRUCTABLE(CSlabAllocator)
public:
	void		Kill(void);

	void*		Malloc(size uiSize);
	void*		Realloc(void* pv, size uiSize);
	bool		Free(void* pv);

	bool		IsLocal(void);
	size		SizeOffset(void);

	const char*	ShortName(void);

protected:
	void*		AllocateSlab(void);
	void		FreeSlab(void* pvSlab);
};


extern CSlabAllocator	gcSlabAllocator;


void* GetSlab(void* pv);


#endif // __SLAB_ALLOCATOR_H__

