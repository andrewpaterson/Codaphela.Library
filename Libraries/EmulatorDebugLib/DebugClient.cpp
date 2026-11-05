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
#include "BaseLib/StringHelper.h"
#include "DebugFormat.h"
#include "DebugClient.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Init(CProcessChannel* pcChannel)
{
	mpcChannel = pcChannel;
	mpuiMessage = (uint8*)malloc(DEBUG_MAX_MESSAGE);
	meState = DCS_Connecting;
	mbStepping = false;
	muiNextRequest = 1;
	muiMotionSent = 0;
	muiMotionAnswered = 0;

	memset(&msWelcome, 0, sizeof(msWelcome));
	memset(masRegisters, 0, sizeof(masRegisters));

	meStopReason = DS_Start;
	muiStopAddress = 0;
	memset(mauiValues, 0, sizeof(mauiValues));
	muiNumStops = 0;
	muiNumRegisterReplies = 0;

	mpuiMemory = (uint8*)malloc(DEBUG_MAX_MEMORY);
	muiMemoryAddress = 0;
	muiMemorySize = 0;
	muiMemoryRequest = 0;
	muiNumMemoryReplies = 0;

	memset(masInstructions, 0, sizeof(masInstructions));
	muiNumInstructions = 0;
	muiDisassemblyRequest = 0;
	muiNumDisassemblies = 0;

	muiErrorType = 0;
	muiErrorRequest = 0;
	mszError.Init();
	muiNumErrors = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Kill(void)
{
	mszError.Kill();
	free(mpuiMemory);
	mpuiMemory = NULL;
	free(mpuiMessage);
	mpuiMessage = NULL;
	mpcChannel = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Answered by the welcome and the state, so it is a request that is
// waited for, as one that moves the processor is.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Hello(void)
{
	SDebugHello	sHello;

	sHello.uiVersion = DEBUG_PROTOCOL_VERSION;
	meState = DCS_Connecting;
	return SendMotion(DM_Hello, &sHello, sizeof(sHello));
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces all the emulator's breakpoints.  Too many for one message are
// added one at a time after it: the ID is the last message's.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::SetBreakpoints(SDebugBreakpoint* asBreakpoints, size uiCount)
{
	SDebugBreakpoints*	psBreakpoints;
	size				i;
	uint32				uiRequest;

	if (uiCount > DEBUG_MAX_LIST)
	{
		uiRequest = SetBreakpoints(asBreakpoints, DEBUG_MAX_LIST);
		for (i = DEBUG_MAX_LIST; (i < uiCount) && (uiRequest != 0); i++)
		{
			uiRequest = AddBreakpoint(asBreakpoints[i].uiAddress, asBreakpoints[i].uiEnabled != 0);
		}
		return uiRequest;
	}

	psBreakpoints = (SDebugBreakpoints*)GetBody();
	psBreakpoints->uiCount = (uint32)uiCount;
	if (uiCount != 0)
	{
		memcpy(&psBreakpoints[1], asBreakpoints, uiCount * sizeof(SDebugBreakpoint));
	}
	return Send(DM_SetBreakpoints, psBreakpoints, sizeof(SDebugBreakpoints) + uiCount * sizeof(SDebugBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
// If there is already a breakpoint at uiAddress it is enabled or disabled.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::AddBreakpoint(uint32 uiAddress, bool bEnabled, uint32 uiSpace)
{
	SDebugBreakpoint	sBreakpoint;

	sBreakpoint.uiAddress = uiAddress;
	sBreakpoint.uiEnabled = bEnabled;
	sBreakpoint.uiSpace = uiSpace;
	return Send(DM_AddBreakpoint, &sBreakpoint, sizeof(sBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::RemoveBreakpoint(uint32 uiAddress, uint32 uiSpace)
{
	SDebugBreakpoint	sBreakpoint;

	sBreakpoint.uiAddress = uiAddress;
	sBreakpoint.uiEnabled = false;
	sBreakpoint.uiSpace = uiSpace;
	return Send(DM_RemoveBreakpoint, &sBreakpoint, sizeof(sBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::EnableBreakpoint(uint32 uiAddress, bool bEnabled, uint32 uiSpace)
{
	SDebugBreakpoint	sBreakpoint;

	sBreakpoint.uiAddress = uiAddress;
	sBreakpoint.uiEnabled = bEnabled;
	sBreakpoint.uiSpace = uiSpace;
	return Send(DM_EnableBreakpoint, &sBreakpoint, sizeof(sBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::EnableAllBreakpoints(bool bEnabled)
{
	SDebugFlag	sFlag;

	sFlag.uiFlag = bEnabled;
	return Send(DM_EnableAllBreakpoints, &sFlag, sizeof(sFlag));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Continue(void)
{
	return SendMotion(DM_Continue, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Pause(void)
{
	return SendMotion(DM_Pause, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Step(uint32 uiInstructions)
{
	SDebugStep	sStep;

	sStep.uiInstructions = uiInstructions;
	return SendMotion(DM_Step, &sStep, sizeof(sStep));
}


//////////////////////////////////////////////////////////////////////////
//
// See SDebugStepRange.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::StepRange(SDebugRange* asRanges, size uiCount, bool bOver)
{
	SDebugStepRange*	psStepRange;

	if (uiCount > DEBUG_MAX_LIST)
	{
		return 0;
	}
	psStepRange = (SDebugStepRange*)GetBody();
	psStepRange->uiOver = bOver;
	psStepRange->uiCount = (uint32)uiCount;
	memcpy(&psStepRange[1], asRanges, uiCount * sizeof(SDebugRange));
	return SendMotion(DM_StepRange, psStepRange, sizeof(SDebugStepRange) + uiCount * sizeof(SDebugRange));
}


//////////////////////////////////////////////////////////////////////////
//
// One instruction, or a whole call if it is one.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::StepOver(void)
{
	return SendMotion(DM_StepOver, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Until the function the processor is in returns.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::StepOut(void)
{
	return SendMotion(DM_StepOut, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Stops at the reset, or with bRun runs on from it.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Reset(bool bRun)
{
	SDebugReset	sReset;

	sReset.uiRun = bRun;
	return SendMotion(DM_Reset, &sReset, sizeof(sReset));
}


//////////////////////////////////////////////////////////////////////////
//
// The program's bytes at uiAddress, in as many messages as it takes.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Load(uint32 uiAddress, void* pvData, size uiSize)
{
	return SendMemory(DM_Load, uiAddress, pvData, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::SetProgramAddress(uint32 uiAddress)
{
	SDebugAddress	sAddress;

	sAddress.uiAddress = uiAddress;
	return Send(DM_SetProgramAddress, &sAddress, sizeof(sAddress));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::ReadRegisters(void)
{
	return Send(DM_ReadRegisters, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::WriteRegister(size uiRegister, uint64 uiValue)
{
	SDebugRegisterValue	sValue;

	sValue.uiRegister = (uint32)uiRegister;
	sValue.uiPadding = 0;
	sValue.uiValue = uiValue;
	return Send(DM_WriteRegister, &sValue, sizeof(sValue));
}


//////////////////////////////////////////////////////////////////////////
//
// At most DEBUG_MAX_MEMORY bytes, answered by DM_Memory.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::ReadMemory(uint32 uiAddress, size uiSize)
{
	SDebugMemory	sMemory;

	if (uiSize > DEBUG_MAX_MEMORY)
	{
		return 0;
	}
	sMemory.uiAddress = uiAddress;
	sMemory.uiSize = (uint32)uiSize;
	return Send(DM_ReadMemory, &sMemory, sizeof(sMemory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::WriteMemory(uint32 uiAddress, void* pvData, size uiSize)
{
	return SendMemory(DM_WriteMemory, uiAddress, pvData, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
// At most DEBUG_MAX_DISASSEMBLY instructions from uiAddress, answered by
// DM_Disassembly.  See SDebugDisassemble for uiLand.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Disassemble(uint32 uiAddress, size uiCount, uint32 uiLand)
{
	SDebugDisassemble	sDisassemble;

	sDisassemble.uiAddress = uiAddress;
	sDisassemble.uiCount = (uint32)((uiCount > DEBUG_MAX_DISASSEMBLY) ? DEBUG_MAX_DISASSEMBLY : uiCount);
	sDisassemble.uiLand = uiLand;
	return Send(DM_Disassemble, &sDisassemble, sizeof(sDisassemble));
}


//////////////////////////////////////////////////////////////////////////
//
// The emulator clears the breakpoints and runs on.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Detach(void)
{
	uint32	uiRequest;

	uiRequest = Send(DM_Detach, NULL, 0);
	meState = DCS_Closed;
	return uiRequest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Quit(void)
{
	uint32	uiRequest;

	uiRequest = Send(DM_Quit, NULL, 0);
	meState = DCS_Closed;
	return uiRequest;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads every message that has arrived, waiting up to
// iTimeoutMilliseconds for the first.  Returns true if any did.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Poll(int iTimeoutMilliseconds)
{
	EChannelResult	eResult;
	uint32			uiType;
	size			uiSize;
	bool			bAny;

	bAny = false;
	while (meState != DCS_Closed)
	{
		eResult = mpcChannel->Receive(&uiType, mpuiMessage, DEBUG_MAX_MESSAGE, &uiSize, bAny ? 0 : iTimeoutMilliseconds);
		if (eResult == CR_Message)
		{
			Handle(uiType, mpuiMessage, uiSize);
			bAny = true;
		}
		else if (eResult == CR_Closed)
		{
			meState = DCS_Closed;
		}
		else
		{
			break;
		}
	}
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Handle(uint32 uiType, uint8* puiData, size uiSize)
{
	uint32	uiRequest;
	void*	pvBody;

	if (uiSize < sizeof(SDebugHeader))
	{
		return;
	}
	uiRequest = ((SDebugHeader*)puiData)->uiRequest;
	pvBody = &puiData[sizeof(SDebugHeader)];
	uiSize -= sizeof(SDebugHeader);

	switch (uiType)
	{
	case DM_Welcome:
		Welcome(pvBody, uiSize);
		break;
	case DM_Stopped:
		Stopped(uiRequest, pvBody, uiSize);
		break;
	case DM_Running:
		Running(uiRequest, pvBody, uiSize);
		break;
	case DM_Registers:
		Registers(pvBody, uiSize);
		break;
	case DM_Memory:
		Memory(uiRequest, pvBody, uiSize);
		break;
	case DM_Disassembly:
		Disassembly(uiRequest, pvBody, uiSize);
		break;
	case DM_Error:
		Error(uiRequest, pvBody, uiSize);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Welcome(void* pvData, size uiSize)
{
	SDebugWelcome*	psWelcome;
	uint32			uiNumRegisters;
	size			i;

	if (uiSize < sizeof(SDebugWelcome))
	{
		return;
	}
	psWelcome = (SDebugWelcome*)pvData;
	uiNumRegisters = psWelcome->uiNumRegisters;
	if ((uiNumRegisters > DEBUG_MAX_REGISTERS) || (uiSize < sizeof(SDebugWelcome) + uiNumRegisters * sizeof(SDebugRegister)))
	{
		return;
	}

	memcpy(&msWelcome, psWelcome, sizeof(SDebugWelcome));
	msWelcome.szProcessor[DEBUG_PROCESSOR_NAME_SIZE - 1] = '\0';
	if ((msWelcome.uiNumSpaces == 0) || (msWelcome.uiNumSpaces > DEBUG_MAX_SPACES))
	{
		msWelcome.uiNumSpaces = 1;
		msWelcome.aszSpaces[0][0] = '\0';
	}
	for (i = 0; i < DEBUG_MAX_SPACES; i++)
	{
		msWelcome.aszSpaces[i][DEBUG_SPACE_NAME_SIZE - 1] = '\0';
	}
	memset(masRegisters, 0, sizeof(masRegisters));
	memcpy(masRegisters, &psWelcome[1], uiNumRegisters * sizeof(SDebugRegister));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Stopped(uint32 uiRequest, void* pvData, size uiSize)
{
	SDebugStopped*	psStopped;
	uint32			uiNumRegisters;

	if (uiSize < sizeof(SDebugStopped))
	{
		return;
	}
	psStopped = (SDebugStopped*)pvData;
	uiNumRegisters = psStopped->uiNumRegisters;
	if ((uiNumRegisters > DEBUG_MAX_REGISTERS) || (uiSize < sizeof(SDebugStopped) + uiNumRegisters * sizeof(uint64)))
	{
		return;
	}

	meState = DCS_Stopped;
	mbStepping = false;
	meStopReason = (EDebugStop)psStopped->uiReason;
	muiStopAddress = psStopped->uiAddress;
	memcpy(mauiValues, &psStopped[1], uiNumRegisters * sizeof(uint64));
	muiNumStops++;
	Answered(uiRequest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Running(uint32 uiRequest, void* pvData, size uiSize)
{
	if (uiSize < sizeof(SDebugRunning))
	{
		return;
	}
	meState = DCS_Running;
	mbStepping = ((SDebugRunning*)pvData)->uiStepping != 0;
	Answered(uiRequest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Registers(void* pvData, size uiSize)
{
	SDebugRegisters*	psRegisters;
	uint32				uiNumRegisters;

	if (uiSize < sizeof(SDebugRegisters))
	{
		return;
	}
	psRegisters = (SDebugRegisters*)pvData;
	uiNumRegisters = psRegisters->uiNumRegisters;
	if ((uiNumRegisters > DEBUG_MAX_REGISTERS) || (uiSize < sizeof(SDebugRegisters) + uiNumRegisters * sizeof(uint64)))
	{
		return;
	}

	memcpy(mauiValues, &psRegisters[1], uiNumRegisters * sizeof(uint64));
	muiNumRegisterReplies++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Memory(uint32 uiRequest, void* pvData, size uiSize)
{
	SDebugMemory*	psMemory;

	if (uiSize < sizeof(SDebugMemory))
	{
		return;
	}
	psMemory = (SDebugMemory*)pvData;
	if ((psMemory->uiSize > DEBUG_MAX_MEMORY) || (uiSize < sizeof(SDebugMemory) + psMemory->uiSize))
	{
		return;
	}

	muiMemoryAddress = psMemory->uiAddress;
	muiMemorySize = psMemory->uiSize;
	memcpy(mpuiMemory, &psMemory[1], muiMemorySize);
	muiMemoryRequest = uiRequest;
	muiNumMemoryReplies++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Disassembly(uint32 uiRequest, void* pvData, size uiSize)
{
	SDebugDisassembly*	psDisassembly;
	size				i;

	if (uiSize < sizeof(SDebugDisassembly))
	{
		return;
	}
	psDisassembly = (SDebugDisassembly*)pvData;
	if ((psDisassembly->uiCount > DEBUG_MAX_DISASSEMBLY) || (uiSize < sizeof(SDebugDisassembly) + psDisassembly->uiCount * sizeof(SDebugInstruction)))
	{
		return;
	}

	muiNumInstructions = psDisassembly->uiCount;
	memcpy(masInstructions, &psDisassembly[1], muiNumInstructions * sizeof(SDebugInstruction));
	for (i = 0; i < muiNumInstructions; i++)
	{
		masInstructions[i].szText[DEBUG_INSTRUCTION_TEXT - 1] = '\0';
		if (masInstructions[i].uiLength > DEBUG_INSTRUCTION_BYTES)
		{
			masInstructions[i].uiLength = DEBUG_INSTRUCTION_BYTES;
		}
	}
	muiDisassemblyRequest = uiRequest;
	muiNumDisassemblies++;
}


//////////////////////////////////////////////////////////////////////////
//
// An error for the request that moves the processor answers it too.
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Error(uint32 uiRequest, void* pvData, size uiSize)
{
	SDebugError*	psError;

	if (uiSize < sizeof(SDebugError))
	{
		return;
	}
	psError = (SDebugError*)pvData;
	psError->szText[DEBUG_MAX_TEXT - 1] = '\0';
	muiErrorType = psError->uiType;
	muiErrorRequest = uiRequest;
	mszError.Kill();
	mszError.Init(psError->szText);
	muiNumErrors++;
	if (uiRequest == muiMotionSent)
	{
		Answered(uiRequest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Answered(uint32 uiRequest)
{
	if (uiRequest > muiMotionAnswered)
	{
		muiMotionAnswered = uiRequest;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Load or DM_WriteMemory, DEBUG_MAX_MEMORY bytes at a time.  The ID is
// the last message's.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::SendMemory(uint32 uiType, uint32 uiAddress, void* pvData, size uiSize)
{
	SDebugMemory*	psMemory;
	size			uiChunk;
	size			uiDone;
	uint32			uiRequest;

	uiDone = 0;
	do
	{
		uiChunk = uiSize - uiDone;
		if (uiChunk > DEBUG_MAX_MEMORY)
		{
			uiChunk = DEBUG_MAX_MEMORY;
		}
		psMemory = (SDebugMemory*)GetBody();
		psMemory->uiAddress = (uint32)(uiAddress + uiDone);
		psMemory->uiSize = (uint32)uiChunk;
		memcpy(&psMemory[1], &((uint8*)pvData)[uiDone], uiChunk);
		uiRequest = Send(uiType, psMemory, sizeof(SDebugMemory) + uiChunk);
		if (uiRequest == 0)
		{
			return 0;
		}
		uiDone += uiChunk;
	}
	while (uiDone < uiSize);
	return uiRequest;
}


//////////////////////////////////////////////////////////////////////////
//
// A request that will be answered by the processor's state.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::SendMotion(uint32 uiType, void* pvData, size uiSize)
{
	uint32	uiRequest;

	uiRequest = Send(uiType, pvData, uiSize);
	if (uiRequest != 0)
	{
		muiMotionSent = uiRequest;
	}
	return uiRequest;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a request's struct can be built, after its header.
//
//////////////////////////////////////////////////////////////////////////
uint8* CDebugClient::GetBody(void)
{
	return &mpuiMessage[sizeof(SDebugHeader)];
}


//////////////////////////////////////////////////////////////////////////
//
// The header with the next ID, then uiSize bytes of pvData (which may be
// built at GetBody already).  Returns the ID, 0 if it couldn't be sent.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDebugClient::Send(uint32 uiType, void* pvData, size uiSize)
{
	SDebugHeader*	psHeader;
	uint32			uiRequest;

	if (meState == DCS_Closed)
	{
		return 0;
	}
	if ((uiSize != 0) && (pvData != GetBody()))
	{
		memmove(GetBody(), pvData, uiSize);
	}
	uiRequest = muiNextRequest;
	psHeader = (SDebugHeader*)mpuiMessage;
	psHeader->uiRequest = uiRequest;
	psHeader->uiReserved = 0;
	if (!mpcChannel->Send(uiType, mpuiMessage, sizeof(SDebugHeader) + uiSize))
	{
		return 0;
	}
	muiNextRequest++;
	return uiRequest;
}


//////////////////////////////////////////////////////////////////////////
//
// The address as the emulator's welcome said to write it.
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::AppendAddress(CChars* psz, uint32 uiAddress)
{
	AppendDebugAddress(psz, uiAddress, GetAddressFormat(), NumAddressBits());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDebugClientState CDebugClient::GetState(void)			{ return meState; }
bool CDebugClient::IsStopped(void)						{ return meState == DCS_Stopped; }
bool CDebugClient::IsRunning(void)						{ return meState == DCS_Running; }
bool CDebugClient::IsStepping(void)						{ return (meState == DCS_Running) && mbStepping; }
bool CDebugClient::IsBusy(void)							{ return muiMotionAnswered < muiMotionSent; }
const char* CDebugClient::GetProcessorName(void)		{ return msWelcome.szProcessor; }
size CDebugClient::NumAddressBits(void)					{ return msWelcome.uiAddressBits; }
EDebugAddressFormat CDebugClient::GetAddressFormat(void)	{ return (EDebugAddressFormat)msWelcome.uiAddressFormat; }
bool CDebugClient::CanDecode(void)						{ return (msWelcome.uiCapabilities & DEBUG_CAN_DECODE) != 0; }
size CDebugClient::NumRegisters(void)					{ return msWelcome.uiNumRegisters; }
EDebugStop CDebugClient::GetStopReason(void)			{ return meStopReason; }
uint32 CDebugClient::GetStopAddress(void)				{ return muiStopAddress; }
size CDebugClient::NumStops(void)						{ return muiNumStops; }
size CDebugClient::NumRegisterReplies(void)				{ return muiNumRegisterReplies; }
uint8* CDebugClient::GetMemory(void)					{ return mpuiMemory; }
uint32 CDebugClient::GetMemoryAddress(void)				{ return muiMemoryAddress; }
size CDebugClient::GetMemorySize(void)					{ return muiMemorySize; }
uint32 CDebugClient::GetMemoryRequest(void)				{ return muiMemoryRequest; }
size CDebugClient::NumMemoryReplies(void)				{ return muiNumMemoryReplies; }
size CDebugClient::NumInstructions(void)				{ return muiNumInstructions; }
uint32 CDebugClient::GetDisassemblyRequest(void)		{ return muiDisassemblyRequest; }
size CDebugClient::NumDisassemblies(void)				{ return muiNumDisassemblies; }
const char* CDebugClient::GetError(void)				{ return mszError.Text(); }
uint32 CDebugClient::GetErrorType(void)					{ return muiErrorType; }
uint32 CDebugClient::GetErrorRequest(void)				{ return muiErrorRequest; }
size CDebugClient::NumErrors(void)						{ return muiNumErrors; }


//////////////////////////////////////////////////////////////////////////
//
// NULL past the last DM_Disassembly's instructions.
//
//////////////////////////////////////////////////////////////////////////
SDebugInstruction* CDebugClient::GetInstruction(size uiIndex)
{
	if (uiIndex >= muiNumInstructions)
	{
		return NULL;
	}
	return &masInstructions[uiIndex];
}


//////////////////////////////////////////////////////////////////////////
//
// The emulator's breakpoint spaces (at least one): a breakpoint in one
// stops only there (a computer's kernel or user mode).
//
//////////////////////////////////////////////////////////////////////////
size CDebugClient::NumBreakpointSpaces(void)
{
	return (msWelcome.uiNumSpaces != 0) ? msWelcome.uiNumSpaces : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDebugClient::GetBreakpointSpaceName(size uiIndex)
{
	return (uiIndex < NumBreakpointSpaces()) ? msWelcome.aszSpaces[uiIndex] : "";
}


//////////////////////////////////////////////////////////////////////////
//
// The space named szName (ignoring case) as a message's uiSpace: its
// index plus one.  False if the emulator has none of that name.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::FindBreakpointSpace(const char* szName, uint32* puiSpace)
{
	size	i;

	for (i = 0; i < NumBreakpointSpaces(); i++)
	{
		if ((msWelcome.aszSpaces[i][0] != '\0') && (StringInsensitiveCompare(msWelcome.aszSpaces[i], szName) == 0))
		{
			*puiSpace = (uint32)(i + 1);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the registers DM_Welcome described.
//
//////////////////////////////////////////////////////////////////////////
SDebugRegister* CDebugClient::GetRegister(size uiRegister)
{
	if (uiRegister >= msWelcome.uiNumRegisters)
	{
		return NULL;
	}
	return &masRegisters[uiRegister];
}


//////////////////////////////////////////////////////////////////////////
//
// At the last stop or DM_Registers, whichever came last.
//
//////////////////////////////////////////////////////////////////////////
uint64 CDebugClient::GetRegisterValue(size uiRegister)
{
	if (uiRegister >= DEBUG_MAX_REGISTERS)
	{
		return 0;
	}
	return mauiValues[uiRegister];
}
