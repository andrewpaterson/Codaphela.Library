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
#include <stdlib.h>
#include <string.h>
#include "Flat65C64DebugTarget.h"


//////////////////////////////////////////////////////////////////////////
//
// The memory starts as zeroes.
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C64DebugTarget::Init(uint32 uiMemorySize)
{
	muiMemorySize = uiMemorySize;
	mpuiMemory = (uint8*)malloc(uiMemorySize);
	memset(mpuiMemory, 0, uiMemorySize);
	mcMPU.Init(ReadWord, WriteByte, this, mpuiMemory, uiMemorySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C64DebugTarget::Kill(void)
{
	mcMPU.Kill();
	free(mpuiMemory);
	mpuiMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFast65C64* CFlat65C64DebugTarget::GetMPU(void)		{ return &mcMPU; }
uint8* CFlat65C64DebugTarget::GetMemory(void)		{ return mpuiMemory; }
const char* CFlat65C64DebugTarget::GetProcessorName(void)	{ return "65C64"; }
size CFlat65C64DebugTarget::NumAddressBits(void)	{ return 32; }
size CFlat65C64DebugTarget::NumRegisters(void)		{ return F64R_NumRegisters; }
uint32 CFlat65C64DebugTarget::GetProgramAddress(void)	{ return mcMPU.GetProgramCounter(); }
void CFlat65C64DebugTarget::SetProgramAddress(uint32 uiAddress)	{ mcMPU.TestSetProgramCounter(uiAddress); }
uint32 CFlat65C64DebugTarget::GetStackPointer(void)	{ return mcMPU.GetS(); }
CBreakpoints* CFlat65C64DebugTarget::GetBreakpoints(void)	{ return mcMPU.GetBreakpoints(); }
bool CFlat65C64DebugTarget::IsAtBreakpoint(void)	{ return mcMPU.IsAtBreakpoint(); }
bool CFlat65C64DebugTarget::IsStopped(void)			{ return mcMPU.IsStopped(); }
void CFlat65C64DebugTarget::Run(uint64 uiBudget)	{ mcMPU.Run((size)uiBudget); }
bool CFlat65C64DebugTarget::Step(void)				{ return mcMPU.Step(); }
void CFlat65C64DebugTarget::Reset(void)				{ mcMPU.Reset(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C64DebugTarget::GetRegister(size uiRegister, SDebugRegister* psDest)
{
	switch (uiRegister)
	{
	case F64R_A:	SetDebugRegister(psDest, "A", 64, DRK_Value);		break;
	case F64R_B:	SetDebugRegister(psDest, "B", 64, DRK_Value);		break;
	case F64R_X:	SetDebugRegister(psDest, "X", 64, DRK_Value);		break;
	case F64R_Y:	SetDebugRegister(psDest, "Y", 64, DRK_Value);		break;
	case F64R_S:	SetDebugRegister(psDest, "S", 32, DRK_Address);	break;
	case F64R_V:	SetDebugRegister(psDest, "V", 32, DRK_Address);	break;
	case F64R_W:	SetDebugRegister(psDest, "W", 32, DRK_Value);		break;
	case F64R_C:	SetDebugRegister(psDest, "C", 32, DRK_Address);	break;
	case F64R_P:	SetDebugRegister(psDest, "P", 16, DRK_Flags);		break;
	default:		SetDebugRegister(psDest, "", 0, DRK_Value);		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CFlat65C64DebugTarget::GetRegisterValue(size uiRegister)
{
	switch (uiRegister)
	{
	case F64R_A:	return mcMPU.GetA();
	case F64R_B:	return mcMPU.GetB();
	case F64R_X:	return mcMPU.GetX();
	case F64R_Y:	return mcMPU.GetY();
	case F64R_S:	return mcMPU.GetS();
	case F64R_V:	return mcMPU.GetV();
	case F64R_W:	return mcMPU.GetW();
	case F64R_C:	return mcMPU.GetProgramCounter();
	case F64R_P:	return mcMPU.GetP();
	default:		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlat65C64DebugTarget::SetRegisterValue(size uiRegister, uint64 uiValue)
{
	switch (uiRegister)
	{
	case F64R_A:	mcMPU.TestSetRegister(R65_A, uiValue);				return true;
	case F64R_B:	mcMPU.TestSetRegister(R65_B, uiValue);				return true;
	case F64R_X:	mcMPU.TestSetRegister(R65_X, uiValue);				return true;
	case F64R_Y:	mcMPU.TestSetRegister(R65_Y, uiValue);				return true;
	case F64R_S:	mcMPU.TestSetRegister(R65_S, uiValue);				return true;
	case F64R_V:	mcMPU.TestSetRegister(R65_V, uiValue);				return true;
	case F64R_W:	mcMPU.TestSetRegister(R65_W, uiValue);				return true;
	case F64R_C:	mcMPU.TestSetProgramCounter((uint32)uiValue);		return true;
	case F64R_P:	mcMPU.TestSetP((uint16)uiValue);					return true;
	default:		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlat65C64DebugTarget::PeekMemory(uint32 uiAddress, void* pvDest, size uiSize)
{
	uiSize = ClampDebugMemory(uiAddress, uiSize, muiMemorySize);
	memcpy(pvDest, &mpuiMemory[uiAddress], uiSize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlat65C64DebugTarget::PokeMemory(uint32 uiAddress, void* pvSource, size uiSize)
{
	uiSize = ClampDebugMemory(uiAddress, uiSize, muiMemorySize);
	memcpy(&mpuiMemory[uiAddress], pvSource, uiSize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Only past the flat block.
//
//////////////////////////////////////////////////////////////////////////
uint16 CFlat65C64DebugTarget::ReadWord(void* pvContext, uint32 uiAddress)
{
	return 0xFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C64DebugTarget::WriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDebugInstruction CFlat65C64DebugTarget::GetInstruction(uint32 uiAddress, size* puiCallLength)
{
	uint16	uiOpcode;

	*puiCallLength = 0;
	if (PeekMemory(uiAddress, &uiOpcode, 2) != 2)
	{
		return DI_Other;
	}
	*puiCallLength = CFast65C64::CallLength(uiOpcode);
	if (*puiCallLength != 0)
	{
		return DI_Call;
	}
	return CFast65C64::IsReturn(uiOpcode) ? DI_Return : DI_Other;
}
