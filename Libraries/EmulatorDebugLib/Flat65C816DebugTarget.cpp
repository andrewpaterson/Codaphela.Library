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
#include "Flat65C816DebugTarget.h"


//////////////////////////////////////////////////////////////////////////
//
// The memory starts as NOPs.
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::Init(uint32 uiMemorySize)
{
	mpcDecoder = NULL;
	muiMemorySize = uiMemorySize;
	mpuiMemory = (uint8*)malloc(uiMemorySize);
	memset(mpuiMemory, 0xEA, uiMemorySize);
	mcMPU.Init(ReadByte, WriteByte, this, mpuiMemory, uiMemorySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::Kill(void)
{
	mcMPU.Kill();
	free(mpuiMemory);
	mpuiMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFast65C816* CFlat65C816DebugTarget::GetMPU(void)	{ return &mcMPU; }
uint8* CFlat65C816DebugTarget::GetMemory(void)		{ return mpuiMemory; }
const char* CFlat65C816DebugTarget::GetProcessorName(void)	{ return "W65C816"; }
size CFlat65C816DebugTarget::NumAddressBits(void)	{ return 24; }
size CFlat65C816DebugTarget::NumRegisters(void)		{ return D816R_NumRegisters; }
uint32 CFlat65C816DebugTarget::GetProgramAddress(void)	{ return mcMPU.GetProgramAddress(); }
uint32 CFlat65C816DebugTarget::GetStackPointer(void)	{ return mcMPU.GetS(); }
CBreakpoints* CFlat65C816DebugTarget::GetBreakpoints(void)	{ return mcMPU.GetBreakpoints(); }
bool CFlat65C816DebugTarget::IsAtBreakpoint(void)	{ return mcMPU.IsAtBreakpoint(); }
bool CFlat65C816DebugTarget::IsStopped(void)		{ return mcMPU.IsStopped(); }
void CFlat65C816DebugTarget::Run(uint64 uiBudget)	{ mcMPU.Run((size)uiBudget); }
bool CFlat65C816DebugTarget::Step(void)				{ return mcMPU.Step(); }
void CFlat65C816DebugTarget::Reset(void)			{ mcMPU.Reset(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::GetRegister(size uiRegister, SDebugRegister* psDest)	{ Get65C816DebugRegister(uiRegister, psDest); }
uint64 CFlat65C816DebugTarget::GetRegisterValue(size uiRegister)	{ return Get65C816DebugRegisterValue(&mcMPU, uiRegister); }
bool CFlat65C816DebugTarget::SetRegisterValue(size uiRegister, uint64 uiValue)	{ return Set65C816DebugRegisterValue(&mcMPU, uiRegister, uiValue); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::SetProgramAddress(uint32 uiAddress)
{
	mcMPU.TestSetProgramCounter((uint8)(uiAddress >> 16), (uint16)uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlat65C816DebugTarget::PeekMemory(uint32 uiAddress, void* pvDest, size uiSize)
{
	uiSize = ClampDebugMemory(uiAddress, uiSize, muiMemorySize);
	memcpy(pvDest, &mpuiMemory[uiAddress], uiSize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlat65C816DebugTarget::PokeMemory(uint32 uiAddress, void* pvSource, size uiSize)
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
uint8 CFlat65C816DebugTarget::ReadByte(void* pvContext, uint32 uiAddress)
{
	return 0xFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::WriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlat65C816DebugTarget::SetDecoder(CDebugDecoder* pcDecoder)
{
	mpcDecoder = pcDecoder;
}


CDebugDecoder* CFlat65C816DebugTarget::GetDecoder(void)				{ return mpcDecoder; }
EDebugAddressFormat CFlat65C816DebugTarget::GetAddressFormat(void)	{ return DAF_Bank; }
