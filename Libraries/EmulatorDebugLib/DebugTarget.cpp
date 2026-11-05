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
#include <string.h>
#include "DebugTarget.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetDebugRegister(SDebugRegister* psDest, const char* szName, uint8 uiBits, EDebugRegisterKind eKind)
{
	memset(psDest, 0, sizeof(SDebugRegister));
	strncpy(psDest->szName, szName, DEBUG_REGISTER_NAME_SIZE - 1);
	psDest->uiBits = uiBits;
	psDest->uiKind = (uint8)eKind;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ClampDebugMemory(uint32 uiAddress, size uiSize, uint32 uiMemorySize)
{
	if (uiAddress >= uiMemorySize)
	{
		return 0;
	}
	if (uiSize > uiMemorySize - uiAddress)
	{
		return uiMemorySize - uiAddress;
	}
	return uiSize;
}
