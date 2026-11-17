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
#ifdef LINUX_GNU
#include <sys/mman.h>
#include "SystemMemory.h"


//////////////////////////////////////////////////////////////////////////
//
// More than is wanted is mapped, so that a boundary falls inside it, and what is before the boundary and after the
// end is given back.
//
//////////////////////////////////////////////////////////////////////////
void* SystemMemoryMap(lsize uiBytes, lsize uiAlignment)
{
	void*	pvMapped;
	lsize	uiMapped;
	lsize	uiStart;
	lsize	uiEnd;

	pvMapped = mmap(NULL, uiBytes + uiAlignment, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (pvMapped == MAP_FAILED)
	{
		return NULL;
	}

	uiMapped = (lsize)pvMapped;
	uiStart = (uiMapped + uiAlignment - 1) & ~(uiAlignment - 1);
	uiEnd = uiStart + uiBytes;
	if (uiStart > uiMapped)
	{
		munmap(pvMapped, uiStart - uiMapped);
	}
	if (uiMapped + uiBytes + uiAlignment > uiEnd)
	{
		munmap((void*)uiEnd, uiMapped + uiBytes + uiAlignment - uiEnd);
	}
	return (void*)uiStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SystemMemoryUnmap(void* pvMemory, lsize uiBytes)
{
	munmap(pvMemory, uiBytes);
}


#endif // LINUX_GNU

