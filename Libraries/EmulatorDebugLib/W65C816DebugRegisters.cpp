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
#include "W65C816DebugRegisters.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Get65C816DebugRegister(size uiRegister, SDebugRegister* psDest)
{
	switch (uiRegister)
	{
	case D816R_C:	SetDebugRegister(psDest, "C", 16, DRK_Value);		break;
	case D816R_X:	SetDebugRegister(psDest, "X", 16, DRK_Value);		break;
	case D816R_Y:	SetDebugRegister(psDest, "Y", 16, DRK_Value);		break;
	case D816R_S:	SetDebugRegister(psDest, "S", 16, DRK_Address);	break;
	case D816R_D:	SetDebugRegister(psDest, "D", 16, DRK_Address);	break;
	case D816R_DBR:	SetDebugRegister(psDest, "DBR", 8, DRK_Value);		break;
	case D816R_PBR:	SetDebugRegister(psDest, "PBR", 8, DRK_Value);		break;
	case D816R_PC:	SetDebugRegister(psDest, "PC", 16, DRK_Address);	break;
	case D816R_P:	SetDebugRegister(psDest, "P", 8, DRK_Flags, "NVMXDIZC");	break;
	case D816R_E:	SetDebugRegister(psDest, "E", 1, DRK_Flags, "E");			break;
	default:		SetDebugRegister(psDest, "", 0, DRK_Value);		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 Get65C816DebugRegisterValue(CFast65C816* pcMPU, size uiRegister)
{
	switch (uiRegister)
	{
	case D816R_C:	return pcMPU->GetC();
	case D816R_X:	return pcMPU->GetX();
	case D816R_Y:	return pcMPU->GetY();
	case D816R_S:	return pcMPU->GetS();
	case D816R_D:	return pcMPU->GetD();
	case D816R_DBR:	return pcMPU->GetDataBank();
	case D816R_PBR:	return pcMPU->GetProgramBank();
	case D816R_PC:	return pcMPU->GetProgramCounter();
	case D816R_P:	return pcMPU->GetP();
	case D816R_E:	return pcMPU->IsEmulation();
	default:		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Set65C816DebugRegisterValue(CFast65C816* pcMPU, size uiRegister, uint64 uiValue)
{
	switch (uiRegister)
	{
	case D816R_C:	pcMPU->TestSetC((uint16)uiValue);										return true;
	case D816R_X:	pcMPU->TestSetX((uint16)uiValue);										return true;
	case D816R_Y:	pcMPU->TestSetY((uint16)uiValue);										return true;
	case D816R_S:	pcMPU->TestSetS((uint16)uiValue);										return true;
	case D816R_D:	pcMPU->TestSetD((uint16)uiValue);										return true;
	case D816R_DBR:	pcMPU->TestSetDataBank((uint8)uiValue);										return true;
	case D816R_PBR:	pcMPU->TestSetProgramCounter((uint8)uiValue, pcMPU->GetProgramCounter());	return true;
	case D816R_PC:	pcMPU->TestSetProgramCounter(pcMPU->GetProgramBank(), (uint16)uiValue);		return true;
	case D816R_P:	pcMPU->TestSetP((uint8)uiValue, pcMPU->IsEmulation());						return true;
	case D816R_E:	pcMPU->TestSetP(pcMPU->GetP(), uiValue != 0);								return true;
	default:		return false;
	}
}
