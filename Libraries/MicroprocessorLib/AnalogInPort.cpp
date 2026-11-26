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
#include "Trace.h"
#include "Pins.h"
#include "TransitionEvent.h"
#include "AnalogInPort.h"


//////////////////////////////////////////////////////////////////////////
//
// No voltage configuration: it reads voltages, not levels.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::Init(CPins* pcPins, const char* szName)
{
	CLogicPort::Init(PT_Input, pcPins, szName, NULL);
	mfTarget = NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
// The trace's voltage now, next tick.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::ReadTraceLevel(CTimeline* pcTimeline)
{
	if (mpcTrace != NULL)
	{
		CTransitionEvent::Schedule(this, mpcTrace->GetVoltage(pcTimeline->GetTime()), 1, pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The slew's end voltage, halfway along it.  A slew that never ends (its
// chip has no power) is no change.
//
//////////////////////////////////////////////////////////////////////////
bool CAnalogInPort::TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew, bool bTraceWasUndriven)
{
	int64	iDelay;
	float32	fStartVoltage;
	float32	fEndVoltage;

	if (pcSlew->GetSlewTime() == MAX_INT64)
	{
		return false;
	}
	fStartVoltage = pcSlew->GetStartVoltage();
	fEndVoltage = pcSlew->GetEndVoltage();
	iDelay = 1;
	if (fStartVoltage != fEndVoltage)
	{
		iDelay = pcSlew->CalculateTimeAtVoltage(fStartVoltage, (fStartVoltage + fEndVoltage) / 2);
		if (iDelay < 1)
		{
			iDelay = 1;
		}
	}
	CTransitionEvent::Schedule(this, fEndVoltage, iDelay, pcTimeline);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::TraceUndriven(CTimeline* pcTimeline)
{
	CTransitionEvent::Schedule(this, NoVoltage(), 1, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::TraceVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage)
{
	ReadTraceLevel(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// Remembers the voltage the trace is going to, then tells the chip.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::TransitionEvent(CTimeline* pcTimeline, CTransitionEvent* pcEvent)
{
	mfTarget = pcEvent->GetVoltage();
	CLogicPort::TransitionEvent(pcTimeline, pcEvent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogInPort::Reset(void)
{
	CLogicPort::Reset();
	mfTarget = NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CAnalogInPort::GetTarget(void) const { return mfTarget; }

