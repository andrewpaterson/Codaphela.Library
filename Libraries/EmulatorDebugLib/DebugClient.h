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
#ifndef __DEBUG_CLIENT_H__
#define __DEBUG_CLIENT_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "ThreadLib/ProcessChannel.h"
#include "DebugProtocol.h"


enum EDebugClientState
{
	DCS_Connecting,		//DM_Hello sent, no DM_Welcome yet.
	DCS_Stopped,
	DCS_Running,		//Running, or stepping (IsStepping).
	DCS_Closed,			//The emulator closed the channel, or Detach or Quit was sent.
};


//A process the emulator said started or ended (DM_ProcessStarted, DM_ProcessEnded).
struct SDebugProcessEvent
{
	bool			bStarted;
	SDebugProcess	sProcess;
};


//The debugger's end of a debugging connection.  The channel is connected before Init and is not owned.
//
//The requests only send, and return the request's ID (0 if it couldn't be sent).  Poll reads what the emulator
//has sent and keeps the latest of each: the processor's description, the state, the last stop and its registers,
//the last DM_Registers, DM_Memory, DM_Disassembly and DM_Error, each with the ID of the request it answered.
//Their counts go up as they arrive, so a caller can tell a new one from the one it has seen.
//
//A request that moves the processor is always answered by its state (DM_Stopped or DM_Running) or an error, so
//IsBusy says whether the last such request is still to be answered: no other should be sent until it has been.
class CDebugClient
{
protected:
	CProcessChannel*	mpcChannel;
	uint8*				mpuiMessage;	//DEBUG_MAX_MESSAGE bytes, for each message received and sent.
	EDebugClientState	meState;
	bool				mbStepping;
	uint32				muiNextRequest;
	uint32				muiMotionSent;		//The ID of the last request sent that moves the processor.
	uint32				muiMotionAnswered;	//The ID of the last one answered.

	SDebugWelcome		msWelcome;
	SDebugRegister		masRegisters[DEBUG_MAX_REGISTERS];

	EDebugStop			meStopReason;
	uint32				muiStopAddress;
	uint64				mauiValues[DEBUG_MAX_REGISTERS];	//At the last stop, or the last DM_Registers.
	size				muiNumStops;
	size				muiNumRegisterReplies;

	uint8*				mpuiMemory;		//DEBUG_MAX_MEMORY bytes.
	uint32				muiMemoryAddress;
	size				muiMemorySize;
	uint32				muiMemoryRequest;
	size				muiNumMemoryReplies;

	SDebugInstruction	masInstructions[DEBUG_MAX_DISASSEMBLY];
	size				muiNumInstructions;
	uint32				muiDisassemblyRequest;
	size				muiNumDisassemblies;

	CArrayTemplate<SDebugProcess>		masProcesses;		//Running, as the emulator has told (DM_ProcessStarted, DM_ProcessEnded).
	CArrayTemplate<SDebugProcessEvent>	masProcessEvents;	//Every start and end told, in order.

	uint32				muiErrorType;
	uint32				muiErrorRequest;
	CChars				mszError;
	size				muiNumErrors;

public:
	void				Init(CProcessChannel* pcChannel);
	void				Kill(void);

	uint32				Hello(void);
	uint32				SetBreakpoints(SDebugBreakpoint* asBreakpoints, size uiCount);
	uint32				AddBreakpoint(uint32 uiAddress, bool bEnabled, uint32 uiSpace = DEBUG_SPACE_ANY, uint32 uiProcess = DEBUG_PROCESS_ANY);
	uint32				RemoveBreakpoint(uint32 uiAddress, uint32 uiSpace = DEBUG_SPACE_ANY, uint32 uiProcess = DEBUG_PROCESS_ANY);
	uint32				EnableBreakpoint(uint32 uiAddress, bool bEnabled, uint32 uiSpace = DEBUG_SPACE_ANY, uint32 uiProcess = DEBUG_PROCESS_ANY);
	uint32				EnableAllBreakpoints(bool bEnabled);
	uint32				Continue(void);
	uint32				Pause(void);
	uint32				Step(uint32 uiInstructions);
	uint32				StepRange(SDebugRange* asRanges, size uiCount, bool bOver);
	uint32				StepOver(void);
	uint32				StepOut(void);
	uint32				Reset(bool bRun);
	uint32				Load(uint32 uiAddress, void* pvData, size uiSize);
	uint32				SetProgramAddress(uint32 uiAddress);
	uint32				ReadRegisters(void);
	uint32				WriteRegister(size uiRegister, uint64 uiValue);
	uint32				ReadMemory(uint32 uiAddress, size uiSize);
	uint32				WriteMemory(uint32 uiAddress, void* pvData, size uiSize);
	uint32				Disassemble(uint32 uiAddress, size uiCount, uint32 uiLand = DEBUG_NO_ADDRESS);
	uint32				Detach(void);
	uint32				Quit(void);

	bool				Poll(int iTimeoutMilliseconds);

	EDebugClientState	GetState(void);
	bool				IsStopped(void);
	bool				IsRunning(void);
	bool				IsStepping(void);
	bool				IsBusy(void);

	const char*			GetProcessorName(void);
	size				NumAddressBits(void);
	EDebugAddressFormat	GetAddressFormat(void);
	bool				CanDecode(void);
	size				NumBreakpointSpaces(void);
	const char*			GetBreakpointSpaceName(size uiIndex);
	bool				FindBreakpointSpace(const char* szName, uint32* puiSpace);
	bool				HasProcesses(void);
	size				NumProcesses(void);
	SDebugProcess*		GetProcess(size uiIndex);
	bool				FindProcess(const char* szName, uint32* puiProcess);
	size				NumProcessEvents(void);
	SDebugProcessEvent*	GetProcessEvent(size uiIndex);
	size				NumRegisters(void);
	SDebugRegister*		GetRegister(size uiRegister);
	uint64				GetRegisterValue(size uiRegister);
	void				AppendAddress(CChars* psz, uint32 uiAddress);

	EDebugStop			GetStopReason(void);
	uint32				GetStopAddress(void);
	size				NumStops(void);
	size				NumRegisterReplies(void);

	uint8*				GetMemory(void);
	uint32				GetMemoryAddress(void);
	size				GetMemorySize(void);
	uint32				GetMemoryRequest(void);
	size				NumMemoryReplies(void);

	size				NumInstructions(void);
	SDebugInstruction*	GetInstruction(size uiIndex);
	uint32				GetDisassemblyRequest(void);
	size				NumDisassemblies(void);

	const char*			GetError(void);
	uint32				GetErrorType(void);
	uint32				GetErrorRequest(void);
	size				NumErrors(void);

protected:
	void				Handle(uint32 uiType, uint8* puiData, size uiSize);
	void				Welcome(void* pvData, size uiSize);
	void				Stopped(uint32 uiRequest, void* pvData, size uiSize);
	void				Running(uint32 uiRequest, void* pvData, size uiSize);
	void				Registers(void* pvData, size uiSize);
	void				Memory(uint32 uiRequest, void* pvData, size uiSize);
	void				Disassembly(uint32 uiRequest, void* pvData, size uiSize);
	void				Error(uint32 uiRequest, void* pvData, size uiSize);
	void				Process(bool bStarted, void* pvData, size uiSize);
	void				Answered(uint32 uiRequest);
	uint32				SendMemory(uint32 uiType, uint32 uiAddress, void* pvData, size uiSize);
	uint32				SendMotion(uint32 uiType, void* pvData, size uiSize);
	uint8*				GetBody(void);
	uint32				Send(uint32 uiType, void* pvData, size uiSize);
};


#endif // __DEBUG_CLIENT_H__
