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
#ifndef __DEBUG_SERVER_H__
#define __DEBUG_SERVER_H__
#include "BaseLib/ArrayTemplate.h"
#include "ThreadLib/ProcessChannel.h"
#include "DebugProtocol.h"
#include "DebugTarget.h"


#define DEBUG_STEPS_PER_TICK	0x10000		//The most steps of a DM_StepRange in one Tick.


enum EDebugServerState
{
	DSS_Stopped,
	DSS_Running,
	DSS_Stepping,	//A DM_StepRange, DM_StepOver or DM_StepOut, over as many Ticks as it takes.
};


typedef CArrayTemplate<SDebugRange>	CArrayDebugRanges;


//The emulator's end of a debugging connection: it does what the debugger's messages ask to a CDebugTarget and
//tells the debugger when the target stops.  The channel is connected before Init or Attach and is not owned; with
//none the target just runs (or holds stopped) until a debugger is attached.
//
//The emulator calls Tick as often as it would run the processor (each window tick), with the budget it would
//run it for.  Tick reads the messages that have arrived, then runs the target for the budget if it is running,
//or carries on a step.  It never waits.  A server starts stopped (bStopped) or running.
//
//If the debugger detaches or its channel closes the breakpoints are cleared and the target runs on; another
//channel can then be attached.
class CDebugServer
{
protected:
	CProcessChannel*	mpcChannel;
	CDebugTarget*		mpcTarget;
	uint8*				mpuiMessage;	//DEBUG_MAX_MESSAGE bytes, for each message received and sent.
	EDebugServerState	meState;
	bool				mbStepped;		//The last stop was the end of a step: Continue executes that instruction first.
	bool				mbConnected;	//DM_Hello has been received.
	bool				mbClosed;		//The channel closed or the debugger detached.
	bool				mbQuit;

	CArrayDebugRanges	masRanges;		//A DM_StepRange's.
	bool				mbOver;
	bool				mbOut;			//A DM_StepOut: steps until a return of its own takes the stack above muiDepth.
	uint32				muiDepth;		//The stack pointer at the last instruction in a range, or where a step out started.
	size				muiCalls;		//Calls a step out has made that haven't returned.

public:
	void				Init(CProcessChannel* pcChannel, CDebugTarget* pcTarget, bool bStopped = true);
	void				Kill(void);
	void				Attach(CProcessChannel* pcChannel);

	void				Tick(uint64 uiBudget);
	void				Poll(void);

	EDebugServerState	GetState(void);
	bool				IsConnected(void);
	bool				IsClosed(void);
	bool				IsQuitting(void);

protected:
	void				Handle(uint32 uiType, void* pvData, size uiSize);
	void				Hello(void* pvData, size uiSize);
	void				SetBreakpoints(void* pvData, size uiSize);
	void				Continue(void);
	void				Pause(void);
	void				Step(void* pvData, size uiSize);
	void				StartStepRange(void* pvData, size uiSize);
	void				StepOver(void);
	void				StepOut(void);
	void				StartStepping(bool bOver, bool bOut);
	bool				StepOutStep(bool bForce);
	void				StepRange(size uiMaxSteps);
	bool				IsInRanges(uint32 uiAddress);
	void				Reset(void);
	void				WriteMemory(uint32 uiRequest, void* pvData, size uiSize);
	void				ReadMemory(void* pvData, size uiSize);
	void				WriteRegister(void* pvData, size uiSize);
	void				Detach(void);

	bool				ForceStep(void);
	void				CheckRun(void);
	void				Stop(EDebugStop eReason);
	void				StopAfterStep(void);
	void				SendWelcome(void);
	void				SendState(void);
	void				SendRegisters(void);
	size				AppendRegisters(uint64* puiDest);
	void				Error(uint32 uiRequest, const char* szText);
	void				Send(uint32 uiType, size uiSize);
};


#endif // __DEBUG_SERVER_H__
