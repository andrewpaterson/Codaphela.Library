/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "Voltage.h"
#include "Timeline.h"
#include "LogicPort.h"
#include "TransitionEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// Makes a transition iDelay from now and adds it to the timeline, which
// owns it.  NULL if it couldn't be added.
//
//////////////////////////////////////////////////////////////////////////
CTransitionEvent* CTransitionEvent::Schedule(CLogicPort* pcPort, float32 fVoltage, int64 iDelay, CTimeline* pcTimeline)
{
	CTransitionEvent*	pcEvent;

	pcEvent = NewMalloc<CTransitionEvent>();
	pcEvent->Init(pcPort, pcTimeline->GetTime() + iDelay, fVoltage);
	if (pcTimeline->AddNewFutureEvent(pcEvent))
	{
		return pcEvent;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransitionEvent::Init(CLogicPort* pcPort, int64 iTime, float32 fVoltage)
{
	CPortInputEvent::Init(pcPort, iTime);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransitionEvent::Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fVoltage)
{
	CPortInputEvent::Init(pcPort, iTime, iID);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransitionEvent::Execute(CTimeline* pcTimeline)
{
	CPortInputEvent::Execute(pcTimeline);
	mpcPort->TransitionEvent(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Transition: Time [1024] 2.0V".
//
//////////////////////////////////////////////////////////////////////////
void CTransitionEvent::PrintShort(CChars* psz)
{
	CPortInputEvent::PrintShort(psz);
	psz->Append(' ');
	AppendVoltage(psz, mfVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CTransitionEvent::GetVoltage(void)
{
	return mfVoltage;
}

