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
#ifdef _WIN32
#include "WindowsHeaders.h"
#include "SystemMemory.h"


//How many times an address found free is tried for before giving up.
#define SYSTEM_MEMORY_MAP_TRIES		16


//////////////////////////////////////////////////////////////////////////
//
// Windows can't give back part of what it reserved, so more than is wanted is reserved to find where a boundary
// falls, released, and what is wanted reserved again at the boundary.  Another thread can take the address between
// the two: then it is done again.
//
//////////////////////////////////////////////////////////////////////////
void* SystemMemoryMap(lsize uiBytes, lsize uiAlignment)
{
	void*	pvReserved;
	void*	pvMemory;
	lsize	uiStart;
	size	i;

	for (i = 0; i < SYSTEM_MEMORY_MAP_TRIES; i++)
	{
		pvReserved = VirtualAlloc(NULL, uiBytes + uiAlignment, MEM_RESERVE, PAGE_NOACCESS);
		if (!pvReserved)
		{
			return NULL;
		}
		uiStart = ((lsize)pvReserved + uiAlignment - 1) & ~(uiAlignment - 1);
		VirtualFree(pvReserved, 0, MEM_RELEASE);

		pvMemory = VirtualAlloc((void*)uiStart, uiBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (pvMemory)
		{
			return pvMemory;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SystemMemoryUnmap(void* pvMemory, lsize uiBytes)
{
	(void)uiBytes;
	VirtualFree(pvMemory, 0, MEM_RELEASE);
}


#endif // _WIN32

