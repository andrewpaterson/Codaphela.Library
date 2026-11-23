/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DEBUG_MEMORY_CACHE_H__
#define __DEBUG_MEMORY_CACHE_H__
#include "BaseLib/ArrayTemplate.h"
#include "DebugProtocol.h"


#define DEBUG_MEMORY_PAGE	256


enum EDebugMemoryPage
{
	DMP_Wanted,		//Read, but not here: to ask for.
	DMP_Asked,		//Asked for, its reply still to come.
	DMP_Read,
};


struct SDebugMemoryPage
{
	uint32	uiSpace;		//DEBUG_SPACE_ANY or a space's index plus one.
	uint32	uiAddress;		//A multiple of DEBUG_MEMORY_PAGE.
	uint32	uiState;		//EDebugMemoryPage.
	uint32	uiValid;		//Once read, how many of its bytes, from its start, there was memory for.
	uint8	auiBytes[DEBUG_MEMORY_PAGE];
};


//The target's memory as a debugger has read it since it last stopped, a page at a time in each space.  Reading what
//isn't here yet wants its pages: they are asked for together (TakeWanted, one DM_ReadMemory) and read again once
//their reply has come (Fill), so the Call Stack, Locals and Watch windows each read without waiting on the others and
//a page is read from the target once.
class CDebugMemoryCache
{
protected:
	CArrayTemplate<SDebugMemoryPage>	masPages;

public:
	void				Init(void);
	void				Kill(void);
	void				Clear(void);

	bool				Read(uint32 uiSpace, uint32 uiAddress, void* pvDest, size uiSize);
	size				TakeWanted(SDebugMemoryRange* asRanges, size uiMax);
	void				Fill(SDebugMemoryRange* psRange, uint8* puiBytes);

protected:
	SDebugMemoryPage*	Find(uint32 uiSpace, uint32 uiAddress);
};


#endif // __DEBUG_MEMORY_CACHE_H__

