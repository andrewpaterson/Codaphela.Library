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
#define DEBUG_REGISTER_FLAGS_SIZE	16
#define DEBUG_INSTRUCTION_BYTES		16
#define DEBUG_INSTRUCTION_TEXT		48


//How a register is shown.
enum EDebugRegisterKind
{
	DRK_Value,		//A number.
	DRK_Address,	//A program or data address.
	DRK_Flags,		//Status bits, named by SDebugRegister::szFlags.
};


//How the processor's addresses are written.
enum EDebugAddressFormat
{
	DAF_Flat,		//$00001234, as many digits as the address bits.
	DAF_Bank,		//$12:3456: the bits above 16 are a bank.
};


//What an instruction does to the flow.
enum EDebugInstruction
{
	DI_Other,
	DI_Call,		//It returns to the instruction after it (a subroutine call, or a software interrupt).
	DI_Return,		//From a subroutine or an interrupt.
	DI_Jump,		//Always goes to another address (uiTarget when it is known).
	DI_Branch,		//May go to uiTarget.
};


//One of a processor's registers, as the debugger shows it.  A flags register's szFlags are its bits' letters, the
//highest first and the last for bit 0 ('-' for a bit without one); shown upper case when set.
struct SDebugRegister
{
	char	szName[DEBUG_REGISTER_NAME_SIZE];
	uint8	uiBits;
	uint8	uiKind;		//EDebugRegisterKind.
	uint8	auiPadding[2];
	char	szFlags[DEBUG_REGISTER_FLAGS_SIZE];
};


//One instruction as the processor decodes it now (with its current modes): where it is, its bytes, what it does to
//the flow and its text in the processor's assembly syntax (empty if the decoder has none).
struct SDebugInstruction
{
	uint32	uiAddress;
	uint32	uiTarget;		//Where a call, jump or branch goes, if uiHasTarget.
	uint8	uiLength;
	uint8	uiKind;			//EDebugInstruction.
	uint8	uiHasTarget;
	uint8	uiPadding;
	uint8	auiBytes[DEBUG_INSTRUCTION_BYTES];
	char	szText[DEBUG_INSTRUCTION_TEXT];
};


class CDebugTarget;


//Knows a processor's instructions: the one place a debugger learns them from.  It lives with whatever knows the
//processor (an assembler, the emulator) so EmulatorDebugLib has no instruction tables of its own.
class CDebugDecoder
{
public:
	//Up to uiCount instructions from uiAddress, read with pcTarget->PeekMemory and decoded with its registers as they
	//are now.  Returns how many: fewer where memory ends.
	virtual size	Decode(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest) =0;
};


//A process the target's operating system runs (a kernal's program), as the debugger is told of it.  Its ID is the
//target's own; the name is what the system ran (its program's file, "\\boot.exe").
#define DEBUG_PROCESS_ANY			0xFFFFFFFF
#define DEBUG_SPACE_ANY				0			//A breakpoint space: every one; otherwise a space's index plus one.
#define DEBUG_PROCESS_NAME_SIZE		64

struct SDebugProcess
{
	uint32	uiProcess;
	int32	iExitCode;		//Once it has ended.
	char	szName[DEBUG_PROCESS_NAME_SIZE];
};


enum EDebugProcessEvent
{
	DPE_Started,
	DPE_Ended,
};


//A module the target's operating system loaded (a kernal's driver): its file, where it is and how big.
#define DEBUG_MODULE_NAME_SIZE		64

struct SDebugModule
{
	uint32	uiAddress;
	uint32	uiSize;
	char	szName[DEBUG_MODULE_NAME_SIZE];
};


enum EDebugModuleEvent
{
	DME_Loaded,
	DME_Cleared,		//Every module is gone (the computer reset).
};


//A processor (and the computer around it) that CDebugServer controls.  The server only uses this, so it works
//the same for a W65C816, a 65C64 or anything else.  Addresses are the processor's, at most 32 bits.
//
//Run and Step return at the processor's enabled breakpoints (GetBreakpoints) without executing the instruction
//there; IsAtBreakpoint is then true and the next Run or Step executes it.  Stacks grow down.
//
//A target with more than one breakpoint space (a computer whose modes see different memory at the same address)
//keeps a CBreakpoints for each and stops only at those of the mode it is in.  GetBreakpoints is the first space's.
//
//A target that runs processes (HasProcesses) tells of each one started and ended while events are watched
//(WatchEvents), and can keep breakpoints of a space for one process (GetProcessBreakpoints): they stop only while that
//process runs.  Its processes' breakpoints go when the process ends (its ID may be given to another).  A target that
//loads modules (HasModules) tells of each one loaded, and of them all going, while events are watched.
class CDebugTarget
{
public:
	virtual const char*			GetProcessorName(void) =0;
	virtual size				NumAddressBits(void) =0;
	virtual EDebugAddressFormat	GetAddressFormat(void) =0;
	virtual size				NumRegisters(void) =0;
	virtual void				GetRegister(size uiRegister, SDebugRegister* psDest) =0;
	virtual uint64				GetRegisterValue(size uiRegister) =0;
	virtual bool				SetRegisterValue(size uiRegister, uint64 uiValue) =0;

	virtual uint32				GetProgramAddress(void) =0;
	virtual void				SetProgramAddress(uint32 uiAddress) =0;
	virtual uint32				GetStackPointer(void) =0;

	virtual CBreakpoints*		GetBreakpoints(void) =0;
	virtual size				NumBreakpointSpaces(void);
	virtual const char*			GetBreakpointSpaceName(size uiSpace);
	virtual CBreakpoints*		GetSpaceBreakpoints(size uiSpace);
	virtual CBreakpoints*		GetProcessBreakpoints(size uiSpace, uint32 uiProcess);
	virtual void				ClearBreakpoints(void);
	virtual void				EnableAllBreakpoints(bool bEnabled);

	virtual bool				HasProcesses(void);
	virtual size				NumProcesses(void);
	virtual bool				GetProcess(size uiIndex, SDebugProcess* psDest);
	virtual void				WatchEvents(bool bWatch);
	virtual bool				TakeProcessEvent(EDebugProcessEvent* peEvent, SDebugProcess* psDest);
	virtual bool				HasModules(void);
	virtual size				NumModules(void);
	virtual bool				GetModule(size uiIndex, SDebugModule* psDest);
	virtual bool				TakeModuleEvent(EDebugModuleEvent* peEvent, SDebugModule* psDest);
	virtual uint32				GetCurrentSpace(void);
	virtual uint32				GetCurrentProcess(void);
	virtual bool				IsAtBreakpoint(void) =0;
	virtual bool				IsStopped(void) =0;

	//uiBudget is the target's own unit: instructions for a bare processor, cycles for a computer that keeps time.
	virtual void				Run(uint64 uiBudget) =0;
	//One instruction (or one step of one, or a cycle waiting).  False at a breakpoint or if the processor stopped.
	virtual bool				Step(void) =0;
	virtual void				Reset(void) =0;

	//Without side effects: devices are not read or written.  Return how many bytes there were memory for.
	virtual size				PeekMemory(uint32 uiAddress, void* pvDest, size uiSize) =0;
	virtual size				PokeMemory(uint32 uiAddress, void* pvSource, size uiSize) =0;

	//NULL if nothing decodes this target's instructions: it can't be disassembled, and calls are stepped into.
	virtual CDebugDecoder*		GetDecoder(void) =0;
};


//Fills in a register's description (szFlags only for a flags register).
void SetDebugRegister(SDebugRegister* psDest, const char* szName, uint8 uiBits, EDebugRegisterKind eKind, const char* szFlags = "");

//How many of uiSize bytes at uiAddress are in a uiMemorySize block (none if uiAddress is past it).
size ClampDebugMemory(uint32 uiAddress, size uiSize, uint32 uiMemorySize);

//What the target's decoder makes of the instructions at uiAddress; 0 if it has none.
size DecodeDebugInstructions(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest);

//Fills in an instruction's address, length and bytes (from pcTarget's memory) with no text, kind DI_Other.
void StartDebugInstruction(SDebugInstruction* psDest, CDebugTarget* pcTarget, uint32 uiAddress, size uiLength);


#endif // __DEBUG_TARGET_H__
