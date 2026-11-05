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
#include "DebugServer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Init(CProcessChannel* pcChannel, CDebugTarget* pcTarget)
{
	mpcChannel = pcChannel;
	mpcTarget = pcTarget;
	mpuiMessage = (uint8*)malloc(DEBUG_MAX_MESSAGE);
	meState = DSS_Stopped;
	mbStepped = false;
	mbConnected = false;
	mbClosed = false;
	mbQuit = false;
	masRanges.Init();
	mbOver = false;
	muiDepth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Kill(void)
{
	masRanges.Kill();
	free(mpuiMessage);
	mpuiMessage = NULL;
	mpcTarget = NULL;
	mpcChannel = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the messages that have arrived, then runs the target for uiBudget
// (its own unit) if it is running, or carries on stepping.  A budget of 0
// only reads the messages: a target that ran nothing would still be at the
// breakpoint it is continuing from.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Tick(uint64 uiBudget)
{
	Poll();
	if ((meState == DSS_Running) && (uiBudget != 0))
	{
		mpcTarget->Run(uiBudget);
		CheckRun();
	}
	else if (meState == DSS_Stepping)
	{
		StepRange(DEBUG_STEPS_PER_TICK);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Handles every message that has arrived, without waiting.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Poll(void)
{
	EChannelResult	eResult;
	uint32			uiType;
	size			uiSize;

	while (!mbClosed)
	{
		eResult = mpcChannel->Receive(&uiType, mpuiMessage, DEBUG_MAX_MESSAGE, &uiSize, 0);
		if (eResult == CR_Message)
		{
			Handle(uiType, mpuiMessage, uiSize);
		}
		else if (eResult == CR_TooLarge)
		{
			Error(uiType, "Message too large.");
		}
		else if (eResult == CR_Closed)
		{
			Detach();
		}
		else
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Handle(uint32 uiType, void* pvData, size uiSize)
{
	SDebugBreakpoint*	psBreakpoint;
	SDebugAddress*		psAddress;
	SDebugFlag*			psFlag;

	if (uiType == DM_Hello)
	{
		Hello(pvData, uiSize);
		return;
	}
	if (!mbConnected)
	{
		Error(uiType, "DM_Hello must be first.");
		return;
	}

	switch (uiType)
	{
	case DM_SetBreakpoints:
		SetBreakpoints(pvData, uiSize);
		break;

	case DM_AddBreakpoint:
	case DM_EnableBreakpoint:
		if (uiSize < sizeof(SDebugBreakpoint))
		{
			Error(uiType, "Message too short.");
			break;
		}
		psBreakpoint = (SDebugBreakpoint*)pvData;
		if (uiType == DM_AddBreakpoint)
		{
			if (!mpcTarget->GetBreakpoints()->Add(psBreakpoint->uiAddress, psBreakpoint->uiEnabled != 0))
			{
				mpcTarget->GetBreakpoints()->Enable(psBreakpoint->uiAddress, psBreakpoint->uiEnabled != 0);
			}
		}
		else if (!mpcTarget->GetBreakpoints()->Enable(psBreakpoint->uiAddress, psBreakpoint->uiEnabled != 0))
		{
			Error(uiType, "No breakpoint there.");
		}
		break;

	case DM_RemoveBreakpoint:
		if (uiSize < sizeof(SDebugAddress))
		{
			Error(uiType, "Message too short.");
			break;
		}
		psAddress = (SDebugAddress*)pvData;
		if (!mpcTarget->GetBreakpoints()->Remove(psAddress->uiAddress))
		{
			Error(uiType, "No breakpoint there.");
		}
		break;

	case DM_EnableAllBreakpoints:
		if (uiSize < sizeof(SDebugFlag))
		{
			Error(uiType, "Message too short.");
			break;
		}
		psFlag = (SDebugFlag*)pvData;
		mpcTarget->GetBreakpoints()->EnableAll(psFlag->uiFlag != 0);
		break;

	case DM_Continue:
		Continue();
		break;

	case DM_Pause:
		Pause();
		break;

	case DM_Step:
		Step(pvData, uiSize);
		break;

	case DM_StepRange:
		StartStepRange(pvData, uiSize);
		break;

	case DM_Reset:
		Reset();
		break;

	case DM_Load:
	case DM_WriteMemory:
		WriteMemory(uiType, pvData, uiSize);
		break;

	case DM_SetProgramAddress:
		if (uiSize < sizeof(SDebugAddress))
		{
			Error(uiType, "Message too short.");
			break;
		}
		psAddress = (SDebugAddress*)pvData;
		mpcTarget->SetProgramAddress(psAddress->uiAddress);
		mbStepped = false;
		break;

	case DM_ReadRegisters:
		SendRegisters();
		break;

	case DM_WriteRegister:
		WriteRegister(pvData, uiSize);
		break;

	case DM_ReadMemory:
		ReadMemory(pvData, uiSize);
		break;

	case DM_Detach:
		Detach();
		break;

	case DM_Quit:
		mbQuit = true;
		break;

	default:
		Error(uiType, "Unknown message.");
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Hello(void* pvData, size uiSize)
{
	SDebugHello*	psHello;

	if (uiSize < sizeof(SDebugHello))
	{
		Error(DM_Hello, "Message too short.");
		return;
	}
	psHello = (SDebugHello*)pvData;
	if (psHello->uiVersion != DEBUG_PROTOCOL_VERSION)
	{
		Error(DM_Hello, "Protocol version not supported.");
		return;
	}

	mbConnected = true;
	SendWelcome();
	SendState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SetBreakpoints(void* pvData, size uiSize)
{
	SDebugBreakpoints*	psBreakpoints;
	SDebugBreakpoint*	asBreakpoint;
	CBreakpoints*		pcBreakpoints;
	uint32				i;

	psBreakpoints = (SDebugBreakpoints*)pvData;
	if ((uiSize < sizeof(SDebugBreakpoints)) || (uiSize < sizeof(SDebugBreakpoints) + (size)psBreakpoints->uiCount * sizeof(SDebugBreakpoint)))
	{
		Error(DM_SetBreakpoints, "Message too short.");
		return;
	}

	pcBreakpoints = mpcTarget->GetBreakpoints();
	pcBreakpoints->Clear();
	asBreakpoint = (SDebugBreakpoint*)&psBreakpoints[1];
	for (i = 0; i < psBreakpoints->uiCount; i++)
	{
		pcBreakpoints->Add(asBreakpoint[i].uiAddress, asBreakpoint[i].uiEnabled != 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// After a step the instruction at the program address is executed first,
// even at a breakpoint, so Continue goes on from the step instead of
// stopping where it is.  After a breakpoint the target executes it itself.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Continue(void)
{
	if (meState == DSS_Running)
	{
		return;
	}

	if (mbStepped)
	{
		mbStepped = false;
		if (!ForceStep())
		{
			StopAfterStep();
			return;
		}
	}
	if (mpcTarget->IsStopped())
	{
		Stop(DS_Stopped);
		return;
	}

	meState = DSS_Running;
	Send(DM_Running, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Pause(void)
{
	if (meState != DSS_Stopped)
	{
		Stop(DS_Pause);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each instruction is executed even if it is at a breakpoint: the
// debugger asked for it.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Step(void* pvData, size uiSize)
{
	SDebugStep*	psStep;
	uint32		i;

	if (uiSize < sizeof(SDebugStep))
	{
		Error(DM_Step, "Message too short.");
		return;
	}
	psStep = (SDebugStep*)pvData;

	for (i = 0; i < psStep->uiInstructions; i++)
	{
		if (!ForceStep())
		{
			break;
		}
	}
	StopAfterStep();
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction is executed even at a breakpoint, then StepRange
// carries on over as many Ticks as it takes.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StartStepRange(void* pvData, size uiSize)
{
	SDebugStepRange*	psStepRange;
	SDebugRange*		asRange;
	uint32				i;

	psStepRange = (SDebugStepRange*)pvData;
	if ((uiSize < sizeof(SDebugStepRange)) || (uiSize < sizeof(SDebugStepRange) + (size)psStepRange->uiCount * sizeof(SDebugRange)))
	{
		Error(DM_StepRange, "Message too short.");
		return;
	}

	masRanges.Kill();
	masRanges.Init();
	asRange = (SDebugRange*)&psStepRange[1];
	for (i = 0; i < psStepRange->uiCount; i++)
	{
		masRanges.Add(&asRange[i]);
	}
	mbOver = psStepRange->uiOver != 0;

	muiDepth = mpcTarget->GetStackPointer();
	if (!ForceStep())
	{
		StopAfterStep();
		return;
	}
	meState = DSS_Stepping;
	StepRange(DEBUG_STEPS_PER_TICK);
}


//////////////////////////////////////////////////////////////////////////
//
// Steps while the program address is in the ranges, or (stepping over)
// while the stack is deeper than at the last instruction in them.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StepRange(size uiMaxSteps)
{
	size	i;
	uint32	uiStack;

	for (i = 0; i < uiMaxSteps; i++)
	{
		uiStack = mpcTarget->GetStackPointer();
		if (IsInRanges(mpcTarget->GetProgramAddress()))
		{
			muiDepth = uiStack;
		}
		else if (!mbOver || (uiStack >= muiDepth))
		{
			Stop(DS_Step);
			return;
		}

		if (!mpcTarget->Step())
		{
			Stop(mpcTarget->IsAtBreakpoint() ? DS_Breakpoint : DS_Stopped);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::IsInRanges(uint32 uiAddress)
{
	size			i;
	SDebugRange*	psRange;

	for (i = 0; i < masRanges.NumElements(); i++)
	{
		psRange = masRanges.Get(i);
		if ((uiAddress >= psRange->uiStart) && (uiAddress < psRange->uiEnd))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Reset(void)
{
	mpcTarget->Reset();
	Stop(DS_Reset);
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Load and DM_WriteMemory.  Memory is written whether the target is
// running or not.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::WriteMemory(uint32 uiRequest, void* pvData, size uiSize)
{
	SDebugMemory*	psMemory;
	size			uiWritten;

	psMemory = (SDebugMemory*)pvData;
	if ((uiSize < sizeof(SDebugMemory)) || (uiSize < sizeof(SDebugMemory) + (size)psMemory->uiSize))
	{
		Error(uiRequest, "Message too short.");
		return;
	}

	uiWritten = mpcTarget->PokeMemory(psMemory->uiAddress, &psMemory[1], psMemory->uiSize);
	if (uiWritten != psMemory->uiSize)
	{
		Error(uiRequest, "Not all of it is memory.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Answered with DM_Memory, the bytes there was memory for.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::ReadMemory(void* pvData, size uiSize)
{
	SDebugMemory*	psRequest;
	SDebugMemory*	psReply;
	uint32			uiAddress;
	uint32			uiLength;

	if (uiSize < sizeof(SDebugMemory))
	{
		Error(DM_ReadMemory, "Message too short.");
		return;
	}
	psRequest = (SDebugMemory*)pvData;
	if (psRequest->uiSize > DEBUG_MAX_MEMORY)
	{
		Error(DM_ReadMemory, "Too many bytes.");
		return;
	}

	//The request is in the buffer the reply is built in.
	uiAddress = psRequest->uiAddress;
	uiLength = psRequest->uiSize;
	psReply = (SDebugMemory*)mpuiMessage;
	psReply->uiAddress = uiAddress;
	psReply->uiSize = (uint32)mpcTarget->PeekMemory(uiAddress, &psReply[1], uiLength);
	Send(DM_Memory, sizeof(SDebugMemory) + psReply->uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::WriteRegister(void* pvData, size uiSize)
{
	SDebugRegisterValue*	psValue;

	if (uiSize < sizeof(SDebugRegisterValue))
	{
		Error(DM_WriteRegister, "Message too short.");
		return;
	}
	psValue = (SDebugRegisterValue*)pvData;
	if ((psValue->uiRegister >= mpcTarget->NumRegisters()) || !mpcTarget->SetRegisterValue(psValue->uiRegister, psValue->uiValue))
	{
		Error(DM_WriteRegister, "Register can't be written.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The debugger has gone: nothing stops the target now.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Detach(void)
{
	mpcTarget->GetBreakpoints()->Clear();
	mbClosed = true;
	mbConnected = false;
	if (!mpcTarget->IsStopped())
	{
		meState = DSS_Running;
	}
	else
	{
		meState = DSS_Stopped;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A step that the target refused at a breakpoint is executed anyway.
// False if the processor stopped.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::ForceStep(void)
{
	if (mpcTarget->Step())
	{
		return true;
	}
	if (mpcTarget->IsAtBreakpoint())
	{
		return mpcTarget->Step();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// After the target ran: whether it stopped at a breakpoint or by itself.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::CheckRun(void)
{
	if (mpcTarget->IsAtBreakpoint())
	{
		Stop(DS_Breakpoint);
	}
	else if (mpcTarget->IsStopped())
	{
		Stop(DS_Stopped);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StopAfterStep(void)
{
	Stop(mpcTarget->IsStopped() ? DS_Stopped : DS_Step);
}


//////////////////////////////////////////////////////////////////////////
//
// The target is stopped; the debugger is told why, where, and the
// registers.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Stop(EDebugStop eReason)
{
	SDebugStopped*	psStopped;

	meState = DSS_Stopped;
	mbStepped = eReason == DS_Step;
	if (!mbConnected)
	{
		return;
	}

	psStopped = (SDebugStopped*)mpuiMessage;
	memset(psStopped, 0, sizeof(SDebugStopped));
	psStopped->uiReason = eReason;
	psStopped->uiAddress = mpcTarget->GetProgramAddress();
	psStopped->uiNumRegisters = (uint32)AppendRegisters((uint64*)&psStopped[1]);
	Send(DM_Stopped, sizeof(SDebugStopped) + psStopped->uiNumRegisters * sizeof(uint64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendWelcome(void)
{
	SDebugWelcome*	psWelcome;
	SDebugRegister*	asRegister;
	size			uiNumRegisters;
	size			i;

	uiNumRegisters = mpcTarget->NumRegisters();
	if (uiNumRegisters > DEBUG_MAX_REGISTERS)
	{
		uiNumRegisters = DEBUG_MAX_REGISTERS;
	}

	psWelcome = (SDebugWelcome*)mpuiMessage;
	memset(psWelcome, 0, sizeof(SDebugWelcome));
	psWelcome->uiVersion = DEBUG_PROTOCOL_VERSION;
	psWelcome->uiAddressBits = (uint32)mpcTarget->NumAddressBits();
	psWelcome->uiNumRegisters = (uint32)uiNumRegisters;
	strncpy(psWelcome->szProcessor, mpcTarget->GetProcessorName(), DEBUG_PROCESSOR_NAME_SIZE - 1);

	asRegister = (SDebugRegister*)&psWelcome[1];
	for (i = 0; i < uiNumRegisters; i++)
	{
		memset(&asRegister[i], 0, sizeof(SDebugRegister));
		mpcTarget->GetRegister(i, &asRegister[i]);
	}
	Send(DM_Welcome, sizeof(SDebugWelcome) + uiNumRegisters * sizeof(SDebugRegister));
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Stopped (as if it had just stopped) or DM_Running.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendState(void)
{
	if (meState == DSS_Stopped)
	{
		Stop(mbStepped ? DS_Step : DS_Start);
	}
	else
	{
		Send(DM_Running, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendRegisters(void)
{
	SDebugRegisters*	psRegisters;

	psRegisters = (SDebugRegisters*)mpuiMessage;
	memset(psRegisters, 0, sizeof(SDebugRegisters));
	psRegisters->uiNumRegisters = (uint32)AppendRegisters((uint64*)&psRegisters[1]);
	Send(DM_Registers, sizeof(SDebugRegisters) + psRegisters->uiNumRegisters * sizeof(uint64));
}


//////////////////////////////////////////////////////////////////////////
//
// The registers' values at puiDest.  Returns how many.
//
//////////////////////////////////////////////////////////////////////////
size CDebugServer::AppendRegisters(uint64* puiDest)
{
	size	uiNumRegisters;
	size	i;

	uiNumRegisters = mpcTarget->NumRegisters();
	if (uiNumRegisters > DEBUG_MAX_REGISTERS)
	{
		uiNumRegisters = DEBUG_MAX_REGISTERS;
	}
	for (i = 0; i < uiNumRegisters; i++)
	{
		puiDest[i] = mpcTarget->GetRegisterValue(i);
	}
	return uiNumRegisters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Error(uint32 uiRequest, const char* szText)
{
	SDebugError*	psError;

	psError = (SDebugError*)mpuiMessage;
	memset(psError, 0, sizeof(SDebugError));
	psError->uiRequest = uiRequest;
	strncpy(psError->szText, szText, DEBUG_MAX_TEXT - 1);
	Send(DM_Error, sizeof(SDebugError));
}


//////////////////////////////////////////////////////////////////////////
//
// Sends the uiSize bytes built in mpuiMessage.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Send(uint32 uiType, size uiSize)
{
	if (!mbClosed)
	{
		mpcChannel->Send(uiType, mpuiMessage, uiSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDebugServerState CDebugServer::GetState(void)
{
	return meState;
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Hello has been received and the debugger hasn't gone.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::IsConnected(void)
{
	return mbConnected;
}


//////////////////////////////////////////////////////////////////////////
//
// The debugger detached or its channel closed.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::IsClosed(void)
{
	return mbClosed;
}


//////////////////////////////////////////////////////////////////////////
//
// The debugger sent DM_Quit.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::IsQuitting(void)
{
	return mbQuit;
}
