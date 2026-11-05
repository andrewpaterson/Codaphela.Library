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
#include "MicroprocessorLib/Fast65C816.h"
#include "DebugTarget.h"


enum E65C816DebugRegister
{
	D816R_C,
	D816R_X,
	D816R_Y,
	D816R_S,
	D816R_D,
	D816R_DBR,
	D816R_PBR,
	D816R_PC,
	D816R_P,
	D816R_E,

	D816R_NumRegisters
};


//A CFast65C816's registers as a CDebugTarget describes them, for any target with one.
void	Get65C816DebugRegister(size uiRegister, SDebugRegister* psDest);
uint64	Get65C816DebugRegisterValue(CFast65C816* pcMPU, size uiRegister);
bool	Set65C816DebugRegisterValue(CFast65C816* pcMPU, size uiRegister, uint64 uiValue);


#endif // __W65C816_DEBUG_REGISTERS_H__
