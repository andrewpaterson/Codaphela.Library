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
#ifdef __LIBC65__
#include <stdlib.h>
#include "SlabAllocator.h"


//////////////////////////////////////////////////////////////////////////
//
// Libc65 has no aligned allocation: a slab is allocated with room to align it, and the pointer malloc gave is kept
// just before it for FreeSlab.
//
//////////////////////////////////////////////////////////////////////////
void* CSlabAllocator::AllocateSlab(void)
{
	char*	pcMemory;
	char*	pcSlab;

	pcMemory = (char*)malloc(SLAB_SIZE * 2);
	if (pcMemory == NULL)
	{
		return NULL;
	}
	pcSlab = (char*)(((size)pcMemory + sizeof(void*) + SLAB_SIZE - 1) & ~((size)SLAB_SIZE - 1));
	((void**)pcSlab)[-1] = pcMemory;
	return pcSlab;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSlabAllocator::FreeSlab(void* pvSlab)
{
	if (pvSlab)
	{
		free(((void**)pvSlab)[-1]);
	}
}


#endif // __LIBC65__

