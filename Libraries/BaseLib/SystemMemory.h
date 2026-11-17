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
#ifndef __SYSTEM_MEMORY_H__
#define __SYSTEM_MEMORY_H__
#include "PrimitiveTypes.h"


//What memory from the operating system comes in multiples of, as far as its users here are concerned: both a Linux
//page (4 KB) and what Windows hands out addresses in (64 KB) divide it.
#define SYSTEM_MEMORY_PAGE	((lsize)0x10000)


//uiBytes rounded up to whole pages.
inline lsize SystemMemoryPages(lsize uiBytes)
{
	return (uiBytes + SYSTEM_MEMORY_PAGE - 1) & ~(SYSTEM_MEMORY_PAGE - 1);
}


//Memory straight from the operating system, zeroed, in whole pages (uiBytes is a multiple of SYSTEM_MEMORY_PAGE), starting
//on a boundary of uiAlignment bytes (a power of two, at least a page): what an allocator builds on.  NULL if there is none.  Not on the 65C64 or the
//W65C816, which have no operating system to ask.
void*	SystemMemoryMap(lsize uiBytes, lsize uiAlignment);

//Gives back all of what SystemMemoryMap returned for uiBytes.
void	SystemMemoryUnmap(void* pvMemory, lsize uiBytes);


#endif // __SYSTEM_MEMORY_H__

