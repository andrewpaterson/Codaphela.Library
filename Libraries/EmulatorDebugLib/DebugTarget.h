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
#ifndef __DEBUG_TARGET_H__
#define __DEBUG_TARGET_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MicroprocessorLib/Breakpoints.h"


#define DEBUG_REGISTER_NAME_SIZE	8


//How a register is shown.
enum EDebugRegisterKind
{
	DRK_Value,		//A number.
	DRK_Address,	//A program or data address.
	DRK_Flags,		//Status bits.
};


//One of a processor's registers, as the debugger shows it.
struct SDebugRegister
{
	char	szName[DEBUG_REGISTER_NAME_SIZE];
	uint8	uiBits;
	uint8	uiKind;		//EDebugRegisterKind.
	uint8	auiPadding[2];
};


//A processor (and the computer around it) that CDebugServer controls.  The server only uses this, so it works
//the same for a W65C816, a 65C64 or anything else.  Addresses are the processor's, at most 32 bits.
//
//Run and Step return at the processor's enabled breakpoints (GetBreakpoints) without executing the instruction
//there; IsAtBreakpoint is then true and the next Run or Step executes it.  Stacks grow down.
class CDebugTarget
{
public:
	virtual const char*		GetProcessorName(void) =0;
	virtual size			NumAddressBits(void) =0;
	virtual size			NumRegisters(void) =0;
	virtual void			GetRegister(size uiRegister, SDebugRegister* psDest) =0;
	virtual uint64			GetRegisterValue(size uiRegister) =0;
	virtual bool			SetRegisterValue(size uiRegister, uint64 uiValue) =0;

	virtual uint32			GetProgramAddress(void) =0;
	virtual void			SetProgramAddress(uint32 uiAddress) =0;
	virtual uint32			GetStackPointer(void) =0;

	virtual CBreakpoints*	GetBreakpoints(void) =0;
	virtual bool			IsAtBreakpoint(void) =0;
	virtual bool			IsStopped(void) =0;

	//uiBudget is the target's own unit: instructions for a bare processor, cycles for a computer that keeps time.
	virtual void			Run(uint64 uiBudget) =0;
	//One instruction (or one step of one, or a cycle waiting).  False at a breakpoint or if the processor stopped.
	virtual bool			Step(void) =0;
	virtual void			Reset(void) =0;

	//Without side effects: devices are not read or written.  Return how many bytes there were memory for.
	virtual size			PeekMemory(uint32 uiAddress, void* pvDest, size uiSize) =0;
	virtual size			PokeMemory(uint32 uiAddress, void* pvSource, size uiSize) =0;
};


//Fills in a register's description.
void SetDebugRegister(SDebugRegister* psDest, const char* szName, uint8 uiBits, EDebugRegisterKind eKind);

//How many of uiSize bytes at uiAddress are in a uiMemorySize block (none if uiAddress is past it).
size ClampDebugMemory(uint32 uiAddress, size uiSize, uint32 uiMemorySize);


#endif // __DEBUG_TARGET_H__
