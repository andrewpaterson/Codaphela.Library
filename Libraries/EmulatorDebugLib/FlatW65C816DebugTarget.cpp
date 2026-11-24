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
#include "FlatW65C816DebugTarget.h"


//////////////////////////////////////////////////////////////////////////
//
// The memory starts as NOPs.
//
//////////////////////////////////////////////////////////////////////////
void CFlatW65C816DebugTarget::Init(uint32 uiMemorySize)
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
void CFlatW65C816DebugTarget::Kill(void)
{
	mcMPU.Kill();
	free(mpuiMemory);
	mpuiMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFastW65C816* CFlatW65C816DebugTarget::GetMPU(void)	{ return &mcMPU; }
uint8* CFlatW65C816DebugTarget::GetMemory(void) const		{ return mpuiMemory; }
const char* CFlatW65C816DebugTarget::GetProcessorName(void) const	{ return "W65C816"; }
size CFlatW65C816DebugTarget::NumAddressBits(void) const	{ return 24; }
size CFlatW65C816DebugTarget::NumRegisters(void) const		{ return DW65C816R_NumRegisters; }
uint32 CFlatW65C816DebugTarget::GetProgramAddress(void)	{ return mcMPU.GetProgramAddress(); }
uint32 CFlatW65C816DebugTarget::GetStackPointer(void) const	{ return mcMPU.GetS(); }
CBreakpoints* CFlatW65C816DebugTarget::GetBreakpoints(void)	{ return mcMPU.GetBreakpoints(); }
bool CFlatW65C816DebugTarget::IsAtBreakpoint(void) const	{ return mcMPU.IsAtBreakpoint(); }
CDataWatches* CFlatW65C816DebugTarget::GetSpaceWatches(size uiSpace)	{ return (uiSpace == 0) ? mcMPU.GetWatches() : NULL; }
void CFlatW65C816DebugTarget::WatchesChanged(void)		{ mcMPU.WatchesChanged(); }
const SDataWatchHit* CFlatW65C816DebugTarget::GetWatchHit(void) const	{ return mcMPU.GetWatchHit(); }
bool CFlatW65C816DebugTarget::IsStopped(void) const		{ return mcMPU.IsStopped(); }
void CFlatW65C816DebugTarget::Run(uint64 uiBudget)	{ mcMPU.Run((size)uiBudget); }
bool CFlatW65C816DebugTarget::Step(void)				{ return mcMPU.Step(); }
void CFlatW65C816DebugTarget::Reset(void)			{ mcMPU.Reset(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlatW65C816DebugTarget::GetRegister(size uiRegister, SDebugRegister* psDest)	{ GetW65C816DebugRegister(uiRegister, psDest); }
uint64 CFlatW65C816DebugTarget::GetRegisterValue(size uiRegister)	{ return GetW65C816DebugRegisterValue(&mcMPU, uiRegister); }
bool CFlatW65C816DebugTarget::SetRegisterValue(size uiRegister, uint64 uiValue)	{ return SetW65C816DebugRegisterValue(&mcMPU, uiRegister, uiValue); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlatW65C816DebugTarget::SetProgramAddress(uint32 uiAddress)
{
	mcMPU.TestSetProgramCounter((uint8)(uiAddress >> 16), (uint16)uiAddress);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlatW65C816DebugTarget::PeekMemory(uint32 uiAddress, void* pvDest, size uiSize)
{
	uiSize = ClampDebugMemory(uiAddress, uiSize, muiMemorySize);
	memcpy(pvDest, &mpuiMemory[uiAddress], uiSize);
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFlatW65C816DebugTarget::PokeMemory(uint32 uiAddress, void* pvSource, size uiSize)
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
uint8 CFlatW65C816DebugTarget::ReadByte(void* pvContext, uint32 uiAddress)
{
	return 0xFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlatW65C816DebugTarget::WriteByte(void* pvContext, uint32 uiAddress, uint8 uiValue)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlatW65C816DebugTarget::SetDecoder(CDebugDecoder* pcDecoder)
{
	mpcDecoder = pcDecoder;
}


CDebugDecoder* CFlatW65C816DebugTarget::GetDecoder(void)				{ return mpcDecoder; }
EDebugAddressFormat CFlatW65C816DebugTarget::GetAddressFormat(void) const	{ return DAF_Bank; }
