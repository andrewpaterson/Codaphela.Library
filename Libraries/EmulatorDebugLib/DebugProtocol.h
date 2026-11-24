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
//a CProcessChannel message: an EDebugMessage type, an SDebugHeader, then one of the structs below, little endian,
//laid out as written (every field is aligned to its size).  Lists follow their struct, their count in it.
//
//Each request has an ID (SDebugHeader::uiRequest, counting up from 1); the reply to it, or the DM_Error if it
//failed, has the same ID.  A request that moves the processor (DM_Continue, DM_Pause, DM_Step, DM_StepRange,
//DM_StepOver, DM_StepOut, DM_Reset) is always answered by DM_Stopped or DM_Running with its ID, and later stops
//have the ID of the last such request.  So the debugger knows what the processor is doing without guessing.
//
//The client sends DM_Hello first.  The server answers DM_Welcome, then DM_Stopped or DM_Running.  A server that
//starts stopped (an emulator told to wait for its debugger) lets the debugger set breakpoints and Load before
//anything runs; one that starts running can be attached to while it runs.
//
//The emulator describes itself: its registers (with their flags' letters), how its addresses are written, and
//(DM_Disassembly) its instructions.  A debugger needs no knowledge of the processor.
//
//A breakpoint is in one of the target's breakpoint spaces, which the welcome names (a computer with a kernel and
//user mode has "Kernel" and "User": a breakpoint in one stops only in that mode), or in every one (DEBUG_SPACE_ANY).
//A message's uiSpace is DEBUG_SPACE_ANY or a space's index plus one.  Memory is read in a space too (DM_ReadMemory's
//ranges): as that mode sees it now, user mode's the process the stop names (SDebugStopped::uiUserProcess), so a
//debugger stopped in a kernel's call can read the calling process's stack; DEBUG_SPACE_ANY is the mode it is in.
//
//An emulator whose computer runs processes (DEBUG_CAN_PROCESSES) tells the debugger of those running when it says
//hello, and of each one started (DM_ProcessStarted) and ended (DM_ProcessEnded).  A process started while running
//stops it (DS_Process) before the process's first instruction, so the debugger can give it its breakpoints first: a
//breakpoint's uiProcess is the process it stops in, or DEBUG_PROCESS_ANY.  A stop says the space and process it is
//in, so a debugger with programs at the same addresses in different spaces knows whose code it is.
//
//An emulator whose operating system loads modules into memory (DEBUG_CAN_MODULES: a kernal's drivers) tells the
//debugger of those loaded when it says hello, and of each one loaded (DM_ModuleLoaded: its file, where it is and how
//big).  A module loaded while running stops it (DS_Module) before the module's code runs, so the debugger can put
//breakpoints in it first.  When the computer resets they are gone (DM_ModulesCleared): it loads them again.
//
//An emulator that can watch memory (DEBUG_CAN_WATCH) takes data breakpoints: a breakpoint whose uiKind is
//DBK_Write or DBK_ReadWrite watches uiBytes from its address in its space (and process), and the processor stops
//after the instruction that wrote (or read) one of them (DS_DataBreakpoint, with the address and whether it was a
//write).  Only the processor's own accesses are watched, not a device's.  Instruction fetches never are.
#define DEBUG_PROTOCOL_VERSION		8
#define DEBUG_PROCESSOR_NAME_SIZE	32
#define DEBUG_MAX_REGISTERS			16
#define DEBUG_MAX_MEMORY			0x8000		//The most bytes a Load, WriteMemory or Memory message carries.
#define DEBUG_MAX_LIST				0x1000		//The most breakpoints or step ranges in one message.
#define DEBUG_MAX_MEMORY_RANGES		256			//The most ranges one ReadMemory asks for.
#define DEBUG_MAX_DISASSEMBLY		64			//The most instructions in one DM_Disassembly.
#define DEBUG_MAX_TEXT				256
#define DEBUG_MAX_MESSAGE			(sizeof(SDebugHeader) + sizeof(SDebugMemoryList) + DEBUG_MAX_MEMORY_RANGES * sizeof(SDebugMemoryRange) + DEBUG_MAX_MEMORY)
#define DEBUG_NO_ADDRESS			0xFFFFFFFF
#define DEBUG_MAX_SPACES			4
#define DEBUG_SPACE_NAME_SIZE		16


enum EDebugMessage
{
	//Debugger to emulator.
	DM_Hello = 1,				//SDebugHello.
	DM_SetBreakpoints,			//SDebugBreakpoints then SDebugBreakpoint[uiCount]: replaces them all.
	DM_AddBreakpoint,			//SDebugBreakpoint.
	DM_RemoveBreakpoint,		//SDebugBreakpoint (uiEnabled unused).
	DM_EnableBreakpoint,		//SDebugBreakpoint.
	DM_EnableAllBreakpoints,	//SDebugFlag.
	DM_Continue,				//Nothing.
	DM_Pause,					//Nothing.
	DM_Step,					//SDebugStep.
	DM_StepRange,				//SDebugStepRange then SDebugRange[uiCount].
	DM_Reset,					//SDebugReset.
	DM_Load,					//SDebugMemory then the bytes: written to memory (stopped or not), in its space.
	DM_SetProgramAddress,		//SDebugAddress.
	DM_ReadRegisters,			//Nothing; answered by DM_Registers.
	DM_WriteRegister,			//SDebugRegisterValue.
	DM_ReadMemory,				//SDebugMemoryList then SDebugMemoryRange[uiCount]; answered by DM_Memory.
	DM_WriteMemory,				//SDebugMemory then the bytes, in its space.
	DM_Detach,					//Nothing: the breakpoints are cleared and it runs on without a debugger.
	DM_Quit,					//Nothing: the emulator exits.
	DM_StepOver,				//Nothing: one instruction, or the whole of a call (and what it calls).
	DM_StepOut,					//Nothing: until the function it is in returns.
	DM_Disassemble,				//SDebugDisassemble; answered by DM_Disassembly.

	//Emulator to debugger.
	DM_Welcome = 64,			//SDebugWelcome then SDebugRegister[uiNumRegisters].
	DM_Stopped,					//SDebugStopped then uint64[uiNumRegisters].
	DM_Running,					//SDebugRunning.
	DM_Registers,				//SDebugRegisters then uint64[uiNumRegisters].
	DM_Memory,					//SDebugMemoryList then SDebugMemoryRange[uiCount], each uiSize the bytes there were memory for
								//from its start, then each one's bytes in turn (DEBUG_MAX_MEMORY in all, the last cut short).
	DM_Error,					//SDebugError.
	DM_Disassembly,				//SDebugDisassembly then SDebugInstruction[uiCount].
	DM_ProcessStarted,			//SDebugProcess.
	DM_ProcessEnded,			//SDebugProcess, with its exit code.
	DM_ModuleLoaded,			//SDebugModule.
	DM_ModulesCleared,			//Nothing: the computer reset.
};


//Why the processor stopped.
enum EDebugStop
{
	DS_Start,		//The server started stopped, before the first instruction.
	DS_Breakpoint,
	DS_Step,		//A step finished.
	DS_Pause,
	DS_Stopped,		//The processor stopped itself (STP).
	DS_Reset,
	DS_Process,		//A process was started (DM_ProcessStarted): it hasn't run yet.
	DS_Module,		//A module was loaded (DM_ModuleLoaded): it hasn't run yet.
	DS_DataBreakpoint,	//An instruction read or wrote a data breakpoint's address (SDebugStopped::uiDataAddress); it has run.
};


//What a welcome says the emulator can do.
#define DEBUG_CAN_DECODE			0x0001		//It decodes instructions: DM_Disassemble, and calls are stepped over.
#define DEBUG_CAN_PROCESSES			0x0002		//It runs processes: DM_ProcessStarted, DM_ProcessEnded, and breakpoints in one.
#define DEBUG_CAN_MODULES			0x0004		//It loads modules: DM_ModuleLoaded, DM_ModulesCleared.
#define DEBUG_CAN_WATCH				0x0008		//It takes data breakpoints (DBK_Write, DBK_ReadWrite).


//Before every message's struct.
struct SDebugHeader
{
	uint32	uiRequest;		//The request's ID; a reply's is the request's.  0 for none.
	uint32	uiReserved;
};


struct SDebugHello
{
	uint32	uiVersion;
};


struct SDebugWelcome
{
	uint32	uiVersion;
	uint32	uiAddressBits;
	uint32	uiAddressFormat;	//EDebugAddressFormat.
	uint32	uiCapabilities;		//DEBUG_CAN_*.
	uint32	uiNumRegisters;
	char	szProcessor[DEBUG_PROCESSOR_NAME_SIZE];
	uint32	uiNumSpaces;		//Breakpoint spaces, at least one.
	char	aszSpaces[DEBUG_MAX_SPACES][DEBUG_SPACE_NAME_SIZE];
};


struct SDebugAddress
{
	uint32	uiAddress;
};


struct SDebugFlag
{
	uint32	uiFlag;
};


//What a breakpoint stops on.
enum EDebugBreakpointKind
{
	DBK_Execute,		//Before the instruction at its address.
	DBK_Write,			//After an instruction that writes one of its bytes.
	DBK_ReadWrite,		//After an instruction that reads or writes one of its bytes.
};

#define DEBUG_MAX_WATCH_BYTES	8

struct SDebugBreakpoint
{
	uint32	uiAddress;
	uint32	uiEnabled;
	uint32	uiSpace;	//DEBUG_SPACE_ANY or a space's index plus one.
	uint32	uiProcess;	//DEBUG_PROCESS_ANY or the process it stops in.
	uint32	uiKind;		//EDebugBreakpointKind.
	uint32	uiBytes;	//A data breakpoint's, 1 to DEBUG_MAX_WATCH_BYTES; 0 for DBK_Execute.

	void	Init(uint32 uiAddress, bool bEnabled, uint32 uiSpace = DEBUG_SPACE_ANY, uint32 uiProcess = DEBUG_PROCESS_ANY, EDebugBreakpointKind eKind = DBK_Execute, uint32 uiBytes = 0);
	bool	IsData(void) const;
	bool	SameAs(SDebugBreakpoint* psOther) const;
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


//With uiRun it runs again after the reset (answered DM_Running), otherwise it stops there (DM_Stopped).
struct SDebugReset
{
	uint32	uiRun;
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
	uint32	uiSpace;	//DEBUG_SPACE_ANY or a space's index plus one.
	uint32	uiPadding;
};

struct SDebugMemoryList
{
	uint32	uiCount;
	uint32	uiPadding;
};

//uiSize bytes from uiAddress in uiSpace (DEBUG_SPACE_ANY or a space's index plus one).
struct SDebugMemoryRange
{
	uint32	uiAddress;
	uint32	uiSize;
	uint32	uiSpace;
	uint32	uiPadding;
};


//uiCount instructions from uiAddress.  If uiLand is an address and none of them starts there (they go past it, or
//stop before it) they are decoded from uiLand instead: the program address is always among them, on an
//instruction's start.
struct SDebugDisassemble
{
	uint32	uiAddress;
	uint32	uiCount;
	uint32	uiLand;			//DEBUG_NO_ADDRESS for none.
};


struct SDebugDisassembly
{
	uint32	uiCount;
	uint32	uiPadding;
};


struct SDebugStopped
{
	uint32	uiReason;		//EDebugStop.
	uint32	uiAddress;		//The program address.
	uint32	uiNumRegisters;
	uint32	uiSpace;		//The breakpoint space it is in (its index plus one), DEBUG_SPACE_ANY if the target has none.
	uint32	uiProcess;		//The process it is in, DEBUG_PROCESS_ANY if none is kept apart there (a kernel's code).
	uint32	uiUserProcess;	//Whose memory user mode's is now: the process it is in, or the one whose call or interrupt a
							//kernel is in; DEBUG_PROCESS_ANY if none.
	uint32	uiDataAddress;	//DS_DataBreakpoint's: the watched address read or written,
	uint32	uiDataWrite;	//1 if it was written,
	uint32	uiDataBreakpoint;	//the data breakpoint's address,
	uint32	uiDataBytes;	//and its bytes, as they are now:
	uint8	auiData[DEBUG_MAX_WATCH_BYTES];
};


struct SDebugRunning
{
	uint32	uiStepping;		//A step that takes longer than one message: it stops (DM_Stopped) when it is done.
};


struct SDebugRegisters
{
	uint32	uiNumRegisters;
	uint32	uiPadding;
};


struct SDebugError
{
	uint32	uiType;			//The EDebugMessage that failed (the header has its ID).
	char	szText[DEBUG_MAX_TEXT];
};


#endif // __DEBUG_PROTOCOL_H__
