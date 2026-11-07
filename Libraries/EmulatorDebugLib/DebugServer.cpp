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
void CDebugServer::Init(CProcessChannel* pcChannel, CDebugTarget* pcTarget, bool bStopped)
{
	mpcChannel = pcChannel;
	mpcTarget = pcTarget;
	mpuiMessage = (uint8*)malloc(DEBUG_MAX_MESSAGE);
	meState = bStopped ? DSS_Stopped : DSS_Running;
	mbStepped = false;
	mbConnected = false;
	mbClosed = false;
	mbQuit = false;
	muiRequest = 0;
	muiMotionRequest = 0;
	masRanges.Init();
	mbOver = false;
	mbOut = false;
	muiDepth = 0;
	muiCalls = 0;
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
// A debugger's channel, when there was none or the last one closed.  It
// sends DM_Hello first, as on a channel given to Init.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Attach(CProcessChannel* pcChannel)
{
	mpcChannel = pcChannel;
	mbConnected = false;
	mbClosed = false;
	muiMotionRequest = 0;
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
	SendEvents();
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

	while (!mbClosed && (mpcChannel != NULL))
	{
		eResult = mpcChannel->Receive(&uiType, mpuiMessage, DEBUG_MAX_MESSAGE, &uiSize, 0);
		if (eResult == CR_Message)
		{
			Handle(uiType, mpuiMessage, uiSize);
		}
		else if (eResult == CR_TooLarge)
		{
			muiRequest = 0;
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
// The header gives the request's ID, then the body is handled.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Handle(uint32 uiType, uint8* puiData, size uiSize)
{
	SDebugHeader*	psHeader;

	muiRequest = 0;
	if (uiSize < sizeof(SDebugHeader))
	{
		Error(uiType, "Message too short.");
		return;
	}
	psHeader = (SDebugHeader*)puiData;
	muiRequest = psHeader->uiRequest;
	HandleBody(uiType, &puiData[sizeof(SDebugHeader)], uiSize - sizeof(SDebugHeader));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::HandleBody(uint32 uiType, void* pvData, size uiSize)
{
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
	case DM_RemoveBreakpoint:
		if (uiSize < sizeof(SDebugBreakpoint))
		{
			Error(uiType, "Message too short.");
			break;
		}
		ChangeBreakpoint(uiType, (SDebugBreakpoint*)pvData, uiType);
		break;

	case DM_EnableAllBreakpoints:
		if (uiSize < sizeof(SDebugFlag))
		{
			Error(uiType, "Message too short.");
			break;
		}
		psFlag = (SDebugFlag*)pvData;
		mpcTarget->EnableAllBreakpoints(psFlag->uiFlag != 0);
		break;

	case DM_Continue:
		muiMotionRequest = muiRequest;
		Continue();
		break;

	case DM_Pause:
		muiMotionRequest = muiRequest;
		Pause();
		break;

	case DM_Step:
		muiMotionRequest = muiRequest;
		Step(pvData, uiSize);
		break;

	case DM_StepRange:
		muiMotionRequest = muiRequest;
		StartStepRange(pvData, uiSize);
		break;

	case DM_StepOver:
		muiMotionRequest = muiRequest;
		StepOver();
		break;

	case DM_StepOut:
		muiMotionRequest = muiRequest;
		StepOut();
		break;

	case DM_Reset:
		muiMotionRequest = muiRequest;
		Reset(pvData, uiSize);
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

	case DM_Disassemble:
		Disassemble(pvData, uiSize);
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
// Answered by the welcome and the state, with the hello's ID.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Hello(void* pvData, size uiSize)
{
	SDebugHello*	psHello;
	SDebugProcess	sProcess;
	SDebugModule	sModule;
	size			i;

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
	muiMotionRequest = muiRequest;
	SendWelcome();

	//The processes running and the modules loaded now, then each one started, ended or loaded from here.
	for (i = 0; i < mpcTarget->NumProcesses(); i++)
	{
		if (mpcTarget->GetProcess(i, &sProcess))
		{
			SendProcess(DM_ProcessStarted, &sProcess);
		}
	}
	for (i = 0; i < mpcTarget->NumModules(); i++)
	{
		if (mpcTarget->GetModule(i, &sModule))
		{
			SendModule(&sModule);
		}
	}
	mpcTarget->WatchEvents(true);
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
	uint32				i;

	psBreakpoints = (SDebugBreakpoints*)pvData;
	if ((uiSize < sizeof(SDebugBreakpoints)) || (uiSize < sizeof(SDebugBreakpoints) + (size)psBreakpoints->uiCount * sizeof(SDebugBreakpoint)))
	{
		Error(DM_SetBreakpoints, "Message too short.");
		return;
	}

	mpcTarget->ClearBreakpoints();
	asBreakpoint = (SDebugBreakpoint*)&psBreakpoints[1];
	for (i = 0; i < psBreakpoints->uiCount; i++)
	{
		ChangeBreakpoint(DM_AddBreakpoint, &asBreakpoint[i], DM_SetBreakpoints);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds (uiChange DM_AddBreakpoint, enabling or disabling one already
// there), enables or disables (DM_EnableBreakpoint) or removes
// (DM_RemoveBreakpoint) the breakpoint in its space, or in each space for
// DEBUG_SPACE_ANY, for its process (or every one).  A space the target
// hasn't, a process it can't keep breakpoints for there, or no breakpoint
// to enable or remove, is an error of the message uiType.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::ChangeBreakpoint(uint32 uiChange, SDebugBreakpoint* psBreakpoint, uint32 uiType)
{
	CBreakpoints*	pcBreakpoints;
	size			uiFirst;
	size			uiEnd;
	size			i;
	bool			bChanged;
	bool			bEnabled;

	if (psBreakpoint->uiSpace > mpcTarget->NumBreakpointSpaces())
	{
		Error(uiType, "No such breakpoint space.");
		return;
	}

	uiFirst = (psBreakpoint->uiSpace == DEBUG_SPACE_ANY) ? 0 : (size)(psBreakpoint->uiSpace - 1);
	uiEnd = (psBreakpoint->uiSpace == DEBUG_SPACE_ANY) ? mpcTarget->NumBreakpointSpaces() : (uiFirst + 1);
	bEnabled = psBreakpoint->uiEnabled != 0;
	bChanged = false;
	for (i = uiFirst; i < uiEnd; i++)
	{
		pcBreakpoints = (psBreakpoint->uiProcess == DEBUG_PROCESS_ANY) ? mpcTarget->GetSpaceBreakpoints(i) : mpcTarget->GetProcessBreakpoints(i, psBreakpoint->uiProcess);
		if (pcBreakpoints == NULL)
		{
			Error(uiType, "No breakpoints for a process there.");
			return;
		}
		if (uiChange == DM_AddBreakpoint)
		{
			if (!pcBreakpoints->Add(psBreakpoint->uiAddress, bEnabled))
			{
				pcBreakpoints->Enable(psBreakpoint->uiAddress, bEnabled);
			}
			bChanged = true;
		}
		else if (uiChange == DM_EnableBreakpoint)
		{
			bChanged |= pcBreakpoints->Enable(psBreakpoint->uiAddress, bEnabled);
		}
		else
		{
			bChanged |= pcBreakpoints->Remove(psBreakpoint->uiAddress);
		}
	}
	if (!bChanged)
	{
		Error(uiType, "No breakpoint there.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// DM_ProcessStarted or DM_ProcessEnded, with no request.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendProcess(uint32 uiType, SDebugProcess* psProcess)
{
	memcpy(GetBody(), psProcess, sizeof(SDebugProcess));
	((SDebugProcess*)GetBody())->szName[DEBUG_PROCESS_NAME_SIZE - 1] = '\0';
	Send(uiType, 0, sizeof(SDebugProcess));
}


//////////////////////////////////////////////////////////////////////////
//
// DM_ModuleLoaded, with no request.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendModule(SDebugModule* psModule)
{
	memcpy(GetBody(), psModule, sizeof(SDebugModule));
	((SDebugModule*)GetBody())->szName[DEBUG_MODULE_NAME_SIZE - 1] = '\0';
	Send(DM_ModuleLoaded, 0, sizeof(SDebugModule));
}


//////////////////////////////////////////////////////////////////////////
//
// The processes started and ended and the modules loaded since the last
// tick, to the debugger.  One started or loaded while running stops it
// before it runs, so the debugger can give it its breakpoints.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendEvents(void)
{
	EDebugProcessEvent	eProcessEvent;
	SDebugProcess		sProcess;
	EDebugModuleEvent	eModuleEvent;
	SDebugModule		sModule;
	EDebugStop			eStop;
	bool				bStop;

	bStop = false;
	eStop = DS_Process;
	while (mbConnected && mpcTarget->TakeProcessEvent(&eProcessEvent, &sProcess))
	{
		SendProcess((eProcessEvent == DPE_Started) ? DM_ProcessStarted : DM_ProcessEnded, &sProcess);
		bStop |= eProcessEvent == DPE_Started;
	}
	while (mbConnected && mpcTarget->TakeModuleEvent(&eModuleEvent, &sModule))
	{
		if (eModuleEvent == DME_Loaded)
		{
			SendModule(&sModule);
			if (!bStop)
			{
				eStop = DS_Module;
			}
			bStop = true;
		}
		else
		{
			Send(DM_ModulesCleared, 0, 0);
		}
	}
	if (bStop && (meState == DSS_Running))
	{
		Stop(eStop);
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
		SendRunning(false);
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
	SendRunning(false);
}


//////////////////////////////////////////////////////////////////////////
//
// Answered by a stop even when it is already stopped.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Pause(void)
{
	Stop(DS_Pause);
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
	StartStepping(psStepRange->uiOver != 0, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A call is stepped as a range of just itself, stepping over: until it
// returns to the instruction after it.  Anything else (or anything, if
// the target has no decoder) is one step.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StepOver(void)
{
	SDebugInstruction	sInstruction;
	SDebugRange			sRange;
	uint32				uiAddress;

	uiAddress = mpcTarget->GetProgramAddress();
	if ((DecodeDebugInstructions(mpcTarget, uiAddress, 1, &sInstruction) != 1) || (sInstruction.uiKind != DI_Call))
	{
		ForceStep();
		StopAfterStep();
		return;
	}

	sRange.uiStart = uiAddress;
	sRange.uiEnd = uiAddress + sInstruction.uiLength;
	masRanges.Kill();
	masRanges.Init();
	masRanges.Add(&sRange);
	StartStepping(true, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Until a return that isn't of a call made since takes the stack above
// where it is now (the function's own pushes and pulls don't count).
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StepOut(void)
{
	if (mpcTarget->GetDecoder() == NULL)
	{
		Error(DM_StepOut, "The emulator can't decode instructions.");
		return;
	}
	masRanges.Kill();
	masRanges.Init();
	StartStepping(false, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The first instruction is executed even at a breakpoint, then StepRange
// carries on, over as many Ticks as it takes.  If it doesn't stop now the
// debugger is told it is stepping.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StartStepping(bool bOver, bool bOut)
{
	mbOver = bOver;
	mbOut = bOut;
	muiDepth = mpcTarget->GetStackPointer();
	muiCalls = 0;
	if (bOut)
	{
		if (!StepOutStep(true))
		{
			return;
		}
	}
	else if (!ForceStep())
	{
		StopAfterStep();
		return;
	}
	meState = DSS_Stepping;
	StepRange(DEBUG_STEPS_PER_TICK);
	if (meState == DSS_Stepping)
	{
		SendRunning(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One instruction of a step out, counting the calls it makes and their
// returns.  False when it has stopped: out of the function, at a
// breakpoint (bForce steps past one) or because the processor stopped.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugServer::StepOutStep(bool bForce)
{
	SDebugInstruction	sInstruction;
	bool				bStepped;

	if (DecodeDebugInstructions(mpcTarget, mpcTarget->GetProgramAddress(), 1, &sInstruction) != 1)
	{
		sInstruction.uiKind = DI_Other;
	}
	bStepped = bForce ? ForceStep() : mpcTarget->Step();
	if (!bStepped)
	{
		if (mpcTarget->IsAtBreakpoint())
		{
			Stop(DS_Breakpoint);
		}
		else
		{
			StopAfterStep();
		}
		return false;
	}

	if (sInstruction.uiKind == DI_Call)
	{
		muiCalls++;
	}
	else if (sInstruction.uiKind == DI_Return)
	{
		if (muiCalls > 0)
		{
			muiCalls--;
		}
		else if (mpcTarget->GetStackPointer() > muiDepth)
		{
			Stop(DS_Step);
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Steps while the program address is in the ranges, or (stepping over)
// while the stack is deeper than at the last instruction in them.  A step
// out steps until it returns (StepOutStep).
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::StepRange(size uiMaxSteps)
{
	size	i;
	uint32	uiStack;

	for (i = 0; i < uiMaxSteps; i++)
	{
		if (mbOut)
		{
			if (!StepOutStep(false))
			{
				return;
			}
			continue;
		}

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
// Stops at the reset, or runs on from it.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Reset(void* pvData, size uiSize)
{
	SDebugReset*	psReset;

	if (uiSize < sizeof(SDebugReset))
	{
		Error(DM_Reset, "Message too short.");
		return;
	}
	psReset = (SDebugReset*)pvData;

	mpcTarget->Reset();
	mbStepped = false;
	if (psReset->uiRun)
	{
		meState = DSS_Running;
		SendRunning(false);
	}
	else
	{
		Stop(DS_Reset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// DM_Load and DM_WriteMemory.  Memory is written whether the target is
// running or not.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::WriteMemory(uint32 uiType, void* pvData, size uiSize)
{
	SDebugMemory*	psMemory;
	size			uiWritten;

	psMemory = (SDebugMemory*)pvData;
	if ((uiSize < sizeof(SDebugMemory)) || (uiSize < sizeof(SDebugMemory) + (size)psMemory->uiSize))
	{
		Error(uiType, "Message too short.");
		return;
	}

	uiWritten = mpcTarget->PokeMemory(psMemory->uiAddress, &psMemory[1], psMemory->uiSize);
	if (uiWritten != psMemory->uiSize)
	{
		Error(uiType, "Not all of it is memory.");
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
	psReply = (SDebugMemory*)GetBody();
	psReply->uiAddress = uiAddress;
	psReply->uiSize = (uint32)mpcTarget->PeekMemory(uiAddress, &psReply[1], uiLength);
	Send(DM_Memory, muiRequest, sizeof(SDebugMemory) + psReply->uiSize);
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
// The target's decoder's instructions, answered with DM_Disassembly.  If
// none of them starts at uiLand they start at uiLand instead.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Disassemble(void* pvData, size uiSize)
{
	SDebugDisassemble*	psRequest;
	SDebugDisassembly*	psReply;
	SDebugInstruction*	asInstructions;
	uint32				uiAddress;
	uint32				uiCount;
	uint32				uiLand;
	size				uiDecoded;
	size				i;
	bool				bLanded;

	if (uiSize < sizeof(SDebugDisassemble))
	{
		Error(DM_Disassemble, "Message too short.");
		return;
	}
	if (mpcTarget->GetDecoder() == NULL)
	{
		Error(DM_Disassemble, "The emulator can't decode instructions.");
		return;
	}

	//The request is in the buffer the reply is built in.
	psRequest = (SDebugDisassemble*)pvData;
	uiAddress = psRequest->uiAddress;
	uiCount = psRequest->uiCount;
	uiLand = psRequest->uiLand;
	if (uiCount > DEBUG_MAX_DISASSEMBLY)
	{
		uiCount = DEBUG_MAX_DISASSEMBLY;
	}

	psReply = (SDebugDisassembly*)GetBody();
	asInstructions = (SDebugInstruction*)&psReply[1];
	uiDecoded = DecodeDebugInstructions(mpcTarget, uiAddress, uiCount, asInstructions);
	if (uiLand != DEBUG_NO_ADDRESS)
	{
		bLanded = false;
		for (i = 0; i < uiDecoded; i++)
		{
			bLanded |= asInstructions[i].uiAddress == uiLand;
		}
		if (!bLanded)
		{
			uiDecoded = DecodeDebugInstructions(mpcTarget, uiLand, uiCount, asInstructions);
		}
	}

	psReply->uiCount = (uint32)uiDecoded;
	psReply->uiPadding = 0;
	Send(DM_Disassembly, muiRequest, sizeof(SDebugDisassembly) + uiDecoded * sizeof(SDebugInstruction));
}


//////////////////////////////////////////////////////////////////////////
//
// The debugger has gone: nothing stops the target now.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Detach(void)
{
	mpcTarget->ClearBreakpoints();
	mpcTarget->WatchEvents(false);
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
// registers, with the ID of the request that last moved it.
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

	psStopped = (SDebugStopped*)GetBody();
	memset(psStopped, 0, sizeof(SDebugStopped));
	psStopped->uiReason = eReason;
	psStopped->uiAddress = mpcTarget->GetProgramAddress();
	psStopped->uiSpace = mpcTarget->GetCurrentSpace();
	psStopped->uiProcess = mpcTarget->GetCurrentProcess();
	psStopped->uiNumRegisters = (uint32)AppendRegisters((uint64*)&psStopped[1]);
	Send(DM_Stopped, muiMotionRequest, sizeof(SDebugStopped) + psStopped->uiNumRegisters * sizeof(uint64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendRunning(bool bStepping)
{
	SDebugRunning*	psRunning;

	if (!mbConnected)
	{
		return;
	}
	psRunning = (SDebugRunning*)GetBody();
	psRunning->uiStepping = bStepping;
	Send(DM_Running, muiMotionRequest, sizeof(SDebugRunning));
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

	psWelcome = (SDebugWelcome*)GetBody();
	memset(psWelcome, 0, sizeof(SDebugWelcome));
	psWelcome->uiVersion = DEBUG_PROTOCOL_VERSION;
	psWelcome->uiAddressBits = (uint32)mpcTarget->NumAddressBits();
	psWelcome->uiAddressFormat = (uint32)mpcTarget->GetAddressFormat();
	psWelcome->uiCapabilities = ((mpcTarget->GetDecoder() != NULL) ? DEBUG_CAN_DECODE : 0) | (mpcTarget->HasProcesses() ? DEBUG_CAN_PROCESSES : 0) |
								 (mpcTarget->HasModules() ? DEBUG_CAN_MODULES : 0);
	psWelcome->uiNumRegisters = (uint32)uiNumRegisters;
	strncpy(psWelcome->szProcessor, mpcTarget->GetProcessorName(), DEBUG_PROCESSOR_NAME_SIZE - 1);
	psWelcome->uiNumSpaces = (uint32)mpcTarget->NumBreakpointSpaces();
	if (psWelcome->uiNumSpaces > DEBUG_MAX_SPACES)
	{
		psWelcome->uiNumSpaces = DEBUG_MAX_SPACES;
	}
	for (i = 0; i < psWelcome->uiNumSpaces; i++)
	{
		strncpy(psWelcome->aszSpaces[i], mpcTarget->GetBreakpointSpaceName(i), DEBUG_SPACE_NAME_SIZE - 1);
	}

	asRegister = (SDebugRegister*)&psWelcome[1];
	for (i = 0; i < uiNumRegisters; i++)
	{
		memset(&asRegister[i], 0, sizeof(SDebugRegister));
		mpcTarget->GetRegister(i, &asRegister[i]);
	}
	Send(DM_Welcome, muiRequest, sizeof(SDebugWelcome) + uiNumRegisters * sizeof(SDebugRegister));
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
		SendRunning(meState == DSS_Stepping);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::SendRegisters(void)
{
	SDebugRegisters*	psRegisters;

	psRegisters = (SDebugRegisters*)GetBody();
	memset(psRegisters, 0, sizeof(SDebugRegisters));
	psRegisters->uiNumRegisters = (uint32)AppendRegisters((uint64*)&psRegisters[1]);
	Send(DM_Registers, muiRequest, sizeof(SDebugRegisters) + psRegisters->uiNumRegisters * sizeof(uint64));
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
// With the ID of the request being handled.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Error(uint32 uiType, const char* szText)
{
	SDebugError*	psError;

	psError = (SDebugError*)GetBody();
	memset(psError, 0, sizeof(SDebugError));
	psError->uiType = uiType;
	strncpy(psError->szText, szText, DEBUG_MAX_TEXT - 1);
	Send(DM_Error, muiRequest, sizeof(SDebugError));
}


//////////////////////////////////////////////////////////////////////////
//
// Where a reply's struct is built, after its header.
//
//////////////////////////////////////////////////////////////////////////
uint8* CDebugServer::GetBody(void)
{
	return &mpuiMessage[sizeof(SDebugHeader)];
}


//////////////////////////////////////////////////////////////////////////
//
// Sends the uiBodySize bytes built at GetBody, after a header with
// uiRequest.
//
//////////////////////////////////////////////////////////////////////////
void CDebugServer::Send(uint32 uiType, uint32 uiRequest, size uiBodySize)
{
	SDebugHeader*	psHeader;

	if (mbClosed || (mpcChannel == NULL))
	{
		return;
	}
	psHeader = (SDebugHeader*)mpuiMessage;
	psHeader->uiRequest = uiRequest;
	psHeader->uiReserved = 0;
	mpcChannel->Send(uiType, mpuiMessage, sizeof(SDebugHeader) + uiBodySize);
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
