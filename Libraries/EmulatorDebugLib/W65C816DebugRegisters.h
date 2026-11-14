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
#ifndef __W65C816_DEBUG_REGISTERS_H__
#define __W65C816_DEBUG_REGISTERS_H__
#include "MicroprocessorLib/FastW65C816.h"
#include "DebugTarget.h"


enum EW65C816DebugRegister
{
	DW65C816R_C,
	DW65C816R_X,
	DW65C816R_Y,
	DW65C816R_S,
	DW65C816R_D,
	DW65C816R_DBR,
	DW65C816R_PBR,
	DW65C816R_PC,
	DW65C816R_P,
	DW65C816R_E,

	DW65C816R_NumRegisters
};


//A CFastW65C816's registers as a CDebugTarget describes them, for any target with one.
void	GetW65C816DebugRegister(size uiRegister, SDebugRegister* psDest);
uint64	GetW65C816DebugRegisterValue(CFastW65C816* pcMPU, size uiRegister);
bool	SetW65C816DebugRegisterValue(CFastW65C816* pcMPU, size uiRegister, uint64 uiValue);


#endif // __W65C816_DEBUG_REGISTERS_H__
