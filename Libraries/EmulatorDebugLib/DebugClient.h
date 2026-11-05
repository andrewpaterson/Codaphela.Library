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
#include "ThreadLib/ProcessChannel.h"
#include "DebugProtocol.h"


enum EDebugClientState
{
	DCS_Connecting,		//DM_Hello sent, no DM_Welcome yet.
	DCS_Stopped,
	DCS_Running,
	DCS_Closed,			//The emulator closed the channel, or Detach or Quit was sent.
};


//The debugger's end of a debugging connection.  The channel is connected before Init and is not owned.
//
//The requests only send.  Poll reads what the emulator has sent and keeps the latest of each: the processor's
//description, the state, the last stop and its registers, the last DM_Registers, the last DM_Memory and the last
//DM_Error.  Their counts go up as they arrive, so a caller can tell a new one from the one it has seen.
class CDebugClient
{
protected:
	CProcessChannel*	mpcChannel;
	uint8*				mpuiMessage;	//DEBUG_MAX_MESSAGE bytes, for each message received and sent.
	EDebugClientState	meState;

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
	size				muiNumMemoryReplies;

	uint32				muiErrorRequest;
	CChars				mszError;
	size				muiNumErrors;

public:
	void				Init(CProcessChannel* pcChannel);
	void				Kill(void);

	bool				Hello(void);
	bool				SetBreakpoints(SDebugBreakpoint* asBreakpoints, size uiCount);
	bool				AddBreakpoint(uint32 uiAddress, bool bEnabled);
	bool				RemoveBreakpoint(uint32 uiAddress);
	bool				EnableBreakpoint(uint32 uiAddress, bool bEnabled);
	bool				EnableAllBreakpoints(bool bEnabled);
	bool				Continue(void);
	bool				Pause(void);
	bool				Step(uint32 uiInstructions);
	bool				StepRange(SDebugRange* asRanges, size uiCount, bool bOver);
	bool				StepOver(void);
	bool				StepOut(void);
	bool				Reset(void);
	bool				Load(uint32 uiAddress, void* pvData, size uiSize);
	bool				SetProgramAddress(uint32 uiAddress);
	bool				ReadRegisters(void);
	bool				WriteRegister(size uiRegister, uint64 uiValue);
	bool				ReadMemory(uint32 uiAddress, size uiSize);
	bool				WriteMemory(uint32 uiAddress, void* pvData, size uiSize);
	bool				Detach(void);
	bool				Quit(void);

	bool				Poll(int iTimeoutMilliseconds);

	EDebugClientState	GetState(void);
	bool				IsStopped(void);
	bool				IsRunning(void);
	const char*			GetProcessorName(void);
	size				NumAddressBits(void);
	size				NumRegisters(void);
	SDebugRegister*		GetRegister(size uiRegister);
	uint64				GetRegisterValue(size uiRegister);

	EDebugStop			GetStopReason(void);
	uint32				GetStopAddress(void);
	size				NumStops(void);
	size				NumRegisterReplies(void);

	uint8*				GetMemory(void);
	uint32				GetMemoryAddress(void);
	size				GetMemorySize(void);
	size				NumMemoryReplies(void);

	const char*			GetError(void);
	uint32				GetErrorRequest(void);
	size				NumErrors(void);

protected:
	void				Handle(uint32 uiType, void* pvData, size uiSize);
	void				Welcome(void* pvData, size uiSize);
	void				Stopped(void* pvData, size uiSize);
	void				Registers(void* pvData, size uiSize);
	void				Memory(void* pvData, size uiSize);
	void				Error(void* pvData, size uiSize);
	bool				SendMemory(uint32 uiType, uint32 uiAddress, void* pvData, size uiSize);
	bool				Send(uint32 uiType, void* pvData, size uiSize);
};


#endif // __DEBUG_CLIENT_H__
