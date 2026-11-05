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
	muiNumMemoryReplies = 0;

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
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Hello(void)
{
	SDebugHello	sHello;

	sHello.uiVersion = DEBUG_PROTOCOL_VERSION;
	meState = DCS_Connecting;
	return Send(DM_Hello, &sHello, sizeof(sHello));
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces all the emulator's breakpoints.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::SetBreakpoints(SDebugBreakpoint* asBreakpoints, size uiCount)
{
	SDebugBreakpoints*	psBreakpoints;
	size				i;

	if (uiCount > DEBUG_MAX_LIST)
	{
		//Too many for one message: the rest are added one at a time.
		if (!SetBreakpoints(asBreakpoints, DEBUG_MAX_LIST))
		{
			return false;
		}
		for (i = DEBUG_MAX_LIST; i < uiCount; i++)
		{
			if (!AddBreakpoint(asBreakpoints[i].uiAddress, asBreakpoints[i].uiEnabled != 0))
			{
				return false;
			}
		}
		return true;
	}

	psBreakpoints = (SDebugBreakpoints*)mpuiMessage;
	psBreakpoints->uiCount = (uint32)uiCount;
	memcpy(&psBreakpoints[1], asBreakpoints, uiCount * sizeof(SDebugBreakpoint));
	return Send(DM_SetBreakpoints, mpuiMessage, sizeof(SDebugBreakpoints) + uiCount * sizeof(SDebugBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
// If there is already a breakpoint at uiAddress it is enabled or disabled.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::AddBreakpoint(uint32 uiAddress, bool bEnabled)
{
	SDebugBreakpoint	sBreakpoint;

	sBreakpoint.uiAddress = uiAddress;
	sBreakpoint.uiEnabled = bEnabled;
	return Send(DM_AddBreakpoint, &sBreakpoint, sizeof(sBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::RemoveBreakpoint(uint32 uiAddress)
{
	SDebugAddress	sAddress;

	sAddress.uiAddress = uiAddress;
	return Send(DM_RemoveBreakpoint, &sAddress, sizeof(sAddress));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::EnableBreakpoint(uint32 uiAddress, bool bEnabled)
{
	SDebugBreakpoint	sBreakpoint;

	sBreakpoint.uiAddress = uiAddress;
	sBreakpoint.uiEnabled = bEnabled;
	return Send(DM_EnableBreakpoint, &sBreakpoint, sizeof(sBreakpoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::EnableAllBreakpoints(bool bEnabled)
{
	SDebugFlag	sFlag;

	sFlag.uiFlag = bEnabled;
	return Send(DM_EnableAllBreakpoints, &sFlag, sizeof(sFlag));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Continue(void)
{
	return Send(DM_Continue, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Pause(void)
{
	return Send(DM_Pause, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// One instruction, or a whole call if it is one.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::StepOver(void)
{
	return Send(DM_StepOver, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Until the function the processor is in returns.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::StepOut(void)
{
	return Send(DM_StepOut, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Step(uint32 uiInstructions)
{
	SDebugStep	sStep;

	sStep.uiInstructions = uiInstructions;
	return Send(DM_Step, &sStep, sizeof(sStep));
}


//////////////////////////////////////////////////////////////////////////
//
// See SDebugStepRange.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::StepRange(SDebugRange* asRanges, size uiCount, bool bOver)
{
	SDebugStepRange*	psStepRange;

	if (uiCount > DEBUG_MAX_LIST)
	{
		return false;
	}
	psStepRange = (SDebugStepRange*)mpuiMessage;
	psStepRange->uiOver = bOver;
	psStepRange->uiCount = (uint32)uiCount;
	memcpy(&psStepRange[1], asRanges, uiCount * sizeof(SDebugRange));
	return Send(DM_StepRange, mpuiMessage, sizeof(SDebugStepRange) + uiCount * sizeof(SDebugRange));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Reset(void)
{
	return Send(DM_Reset, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The program's bytes at uiAddress, in as many messages as it takes.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Load(uint32 uiAddress, void* pvData, size uiSize)
{
	return SendMemory(DM_Load, uiAddress, pvData, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::SetProgramAddress(uint32 uiAddress)
{
	SDebugAddress	sAddress;

	sAddress.uiAddress = uiAddress;
	return Send(DM_SetProgramAddress, &sAddress, sizeof(sAddress));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::ReadRegisters(void)
{
	return Send(DM_ReadRegisters, NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::WriteRegister(size uiRegister, uint64 uiValue)
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
bool CDebugClient::ReadMemory(uint32 uiAddress, size uiSize)
{
	SDebugMemory	sMemory;

	if (uiSize > DEBUG_MAX_MEMORY)
	{
		return false;
	}
	sMemory.uiAddress = uiAddress;
	sMemory.uiSize = (uint32)uiSize;
	return Send(DM_ReadMemory, &sMemory, sizeof(sMemory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::WriteMemory(uint32 uiAddress, void* pvData, size uiSize)
{
	return SendMemory(DM_WriteMemory, uiAddress, pvData, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The emulator clears the breakpoints and runs on.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Detach(void)
{
	bool	bResult;

	bResult = Send(DM_Detach, NULL, 0);
	meState = DCS_Closed;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Quit(void)
{
	bool	bResult;

	bResult = Send(DM_Quit, NULL, 0);
	meState = DCS_Closed;
	return bResult;
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
void CDebugClient::Handle(uint32 uiType, void* pvData, size uiSize)
{
	switch (uiType)
	{
	case DM_Welcome:
		Welcome(pvData, uiSize);
		break;
	case DM_Stopped:
		Stopped(pvData, uiSize);
		break;
	case DM_Running:
		meState = DCS_Running;
		break;
	case DM_Registers:
		Registers(pvData, uiSize);
		break;
	case DM_Memory:
		Memory(pvData, uiSize);
		break;
	case DM_Error:
		Error(pvData, uiSize);
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
	memset(masRegisters, 0, sizeof(masRegisters));
	memcpy(masRegisters, &psWelcome[1], uiNumRegisters * sizeof(SDebugRegister));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Stopped(void* pvData, size uiSize)
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
	meStopReason = (EDebugStop)psStopped->uiReason;
	muiStopAddress = psStopped->uiAddress;
	memcpy(mauiValues, &psStopped[1], uiNumRegisters * sizeof(uint64));
	muiNumStops++;
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
void CDebugClient::Memory(void* pvData, size uiSize)
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
	muiNumMemoryReplies++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugClient::Error(void* pvData, size uiSize)
{
	SDebugError*	psError;

	if (uiSize < sizeof(SDebugError))
	{
		return;
	}
	psError = (SDebugError*)pvData;
	psError->szText[DEBUG_MAX_TEXT - 1] = '\0';
	muiErrorRequest = psError->uiRequest;
	mszError.Kill();
	mszError.Init(psError->szText);
	muiNumErrors++;
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Load or DM_WriteMemory, DEBUG_MAX_MEMORY bytes at a time.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::SendMemory(uint32 uiType, uint32 uiAddress, void* pvData, size uiSize)
{
	SDebugMemory*	psMemory;
	size			uiChunk;
	size			uiDone;

	uiDone = 0;
	do
	{
		uiChunk = uiSize - uiDone;
		if (uiChunk > DEBUG_MAX_MEMORY)
		{
			uiChunk = DEBUG_MAX_MEMORY;
		}
		psMemory = (SDebugMemory*)mpuiMessage;
		psMemory->uiAddress = (uint32)(uiAddress + uiDone);
		psMemory->uiSize = (uint32)uiChunk;
		memcpy(&psMemory[1], &((uint8*)pvData)[uiDone], uiChunk);
		if (!Send(uiType, mpuiMessage, sizeof(SDebugMemory) + uiChunk))
		{
			return false;
		}
		uiDone += uiChunk;
	}
	while (uiDone < uiSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugClient::Send(uint32 uiType, void* pvData, size uiSize)
{
	if (meState == DCS_Closed)
	{
		return false;
	}
	return mpcChannel->Send(uiType, pvData, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDebugClientState CDebugClient::GetState(void)			{ return meState; }
bool CDebugClient::IsStopped(void)						{ return meState == DCS_Stopped; }
bool CDebugClient::IsRunning(void)						{ return meState == DCS_Running; }
const char* CDebugClient::GetProcessorName(void)		{ return msWelcome.szProcessor; }
size CDebugClient::NumAddressBits(void)					{ return msWelcome.uiAddressBits; }
size CDebugClient::NumRegisters(void)					{ return msWelcome.uiNumRegisters; }
EDebugStop CDebugClient::GetStopReason(void)			{ return meStopReason; }
uint32 CDebugClient::GetStopAddress(void)				{ return muiStopAddress; }
size CDebugClient::NumStops(void)						{ return muiNumStops; }
size CDebugClient::NumRegisterReplies(void)				{ return muiNumRegisterReplies; }
uint8* CDebugClient::GetMemory(void)					{ return mpuiMemory; }
uint32 CDebugClient::GetMemoryAddress(void)				{ return muiMemoryAddress; }
size CDebugClient::GetMemorySize(void)					{ return muiMemorySize; }
size CDebugClient::NumMemoryReplies(void)				{ return muiNumMemoryReplies; }
const char* CDebugClient::GetError(void)				{ return mszError.Text(); }
uint32 CDebugClient::GetErrorRequest(void)				{ return muiErrorRequest; }
size CDebugClient::NumErrors(void)						{ return muiNumErrors; }


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
