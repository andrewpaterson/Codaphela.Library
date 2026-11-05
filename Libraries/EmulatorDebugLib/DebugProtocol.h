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
#ifndef __DEBUG_PROTOCOL_H__
#define __DEBUG_PROTOCOL_H__
#include "BaseLib/PrimitiveTypes.h"
#include "DebugTarget.h"


//The messages between a debugger (CDebugClient) and an emulator (CDebugServer) over a CProcessChannel.  Each is
//a CProcessChannel message: an EDebugMessage type and one of the structs below, little endian, laid out as
//written (every field is aligned to its size).  Lists follow their struct, their count in it.
//
//The client sends DM_Hello first.  The server answers DM_Welcome, then DM_Stopped or DM_Running.  A server
//that starts stopped (an emulator told to wait for its debugger) lets the debugger set breakpoints and Load before
//anything runs; one that starts running can be attached to while it runs.


#define DEBUG_PROTOCOL_VERSION		1
#define DEBUG_PROCESSOR_NAME_SIZE	32
#define DEBUG_MAX_REGISTERS			16
#define DEBUG_MAX_MEMORY			0x8000		//The most bytes a Load, WriteMemory or Memory message carries.
#define DEBUG_MAX_LIST				0x1000		//The most breakpoints or step ranges in one message.
#define DEBUG_MAX_TEXT				256
#define DEBUG_MAX_MESSAGE			(sizeof(SDebugMemory) + DEBUG_MAX_MEMORY)


enum EDebugMessage
{
	//Debugger to emulator.
	DM_Hello = 1,				//SDebugHello.
	DM_SetBreakpoints,			//SDebugBreakpoints then SDebugBreakpoint[uiCount]: replaces them all.
	DM_AddBreakpoint,			//SDebugBreakpoint.
	DM_RemoveBreakpoint,		//SDebugAddress.
	DM_EnableBreakpoint,		//SDebugBreakpoint.
	DM_EnableAllBreakpoints,	//SDebugFlag.
	DM_Continue,				//Nothing.
	DM_Pause,					//Nothing.
	DM_Step,					//SDebugStep.
	DM_StepRange,				//SDebugStepRange then SDebugRange[uiCount].
	DM_Reset,					//Nothing.
	DM_Load,					//SDebugMemory then the bytes: written to memory (stopped or not).
	DM_SetProgramAddress,		//SDebugAddress.
	DM_ReadRegisters,			//Nothing; answered by DM_Registers.
	DM_WriteRegister,			//SDebugRegisterValue.
	DM_ReadMemory,				//SDebugMemory (no bytes); answered by DM_Memory.
	DM_WriteMemory,				//SDebugMemory then the bytes.
	DM_Detach,					//Nothing: the breakpoints are cleared and it runs on without a debugger.
	DM_Quit,					//Nothing: the emulator exits.
	DM_StepOver,				//Nothing: one instruction, or the whole of a call (and what it calls).
	DM_StepOut,					//Nothing: until the function it is in returns (the stack is above where it was).

	//Emulator to debugger.
	DM_Welcome = 64,			//SDebugWelcome then SDebugRegister[uiNumRegisters].
	DM_Stopped,					//SDebugStopped then uint64[uiNumRegisters].
	DM_Running,					//Nothing.
	DM_Registers,				//SDebugRegisters then uint64[uiNumRegisters].
	DM_Memory,					//SDebugMemory then the bytes there were memory for (uiSize of them).
	DM_Error,					//SDebugError.
};


//Why the processor stopped.
enum EDebugStop
{
	DS_Start,		//The server started stopped, before the first instruction.
	DS_Breakpoint,
	DS_Step,		//A DM_Step or DM_StepRange finished.
	DS_Pause,
	DS_Stopped,		//The processor stopped itself (STP).
	DS_Reset,
};


struct SDebugHello
{
	uint32	uiVersion;
};


struct SDebugWelcome
{
	uint32	uiVersion;
	uint32	uiAddressBits;
	uint32	uiNumRegisters;
	char	szProcessor[DEBUG_PROCESSOR_NAME_SIZE];
};


struct SDebugAddress
{
	uint32	uiAddress;
};


struct SDebugFlag
{
	uint32	uiFlag;
};


struct SDebugBreakpoint
{
	uint32	uiAddress;
	uint32	uiEnabled;
};


struct SDebugBreakpoints
{
	uint32	uiCount;
};


struct SDebugStep
{
	uint32	uiInstructions;
};


//Addresses uiStart to uiEnd - 1.
struct SDebugRange
{
	uint32	uiStart;
	uint32	uiEnd;
};


//Steps while the program address is in one of the ranges (a source line's code, or a whole function to step
//out of it).  With uiOver it also steps on while the stack is deeper than it was at the last instruction in a
//range: calls (and interrupts) are stepped over.  An enabled breakpoint outside the ranges stops it.
struct SDebugStepRange
{
	uint32	uiOver;
	uint32	uiCount;
};


struct SDebugRegisterValue
{
	uint32	uiRegister;
	uint32	uiPadding;
	uint64	uiValue;
};


struct SDebugMemory
{
	uint32	uiAddress;
	uint32	uiSize;
};


struct SDebugStopped
{
	uint32	uiReason;		//EDebugStop.
	uint32	uiAddress;		//The program address.
	uint32	uiNumRegisters;
	uint32	uiPadding;
};


struct SDebugRegisters
{
	uint32	uiNumRegisters;
	uint32	uiPadding;
};


struct SDebugError
{
	uint32	uiRequest;		//The EDebugMessage that failed.
	char	szText[DEBUG_MAX_TEXT];
};


#endif // __DEBUG_PROTOCOL_H__
