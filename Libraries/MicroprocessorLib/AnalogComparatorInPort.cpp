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
#include "TransitionEvent.h"
#include "AnalogComparatorInPort.h"


//////////////////////////////////////////////////////////////////////////
//
// No voltage configuration: it reads voltages, not levels.
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorInPort::Init(CPins* pcPins, const char* szName)
{
	CLogicPort::Init(PT_Input, pcPins, szName, NULL);
	mpcOther = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorInPort::SetOther(CLogicPort* pcOther)
{
	mpcOther = pcOther;
}


//////////////////////////////////////////////////////////////////////////
//
// The trace's voltage, NaN when undriven or not connected.
//
//////////////////////////////////////////////////////////////////////////
float32 CAnalogComparatorInPort::GetVoltage(int64 iTime)
{
	return (mpcTrace != NULL) ? mpcTrace->GetVoltage(iTime) : NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorInPort::ReadTraceLevel(CTimeline* pcTimeline)
{
	CTransitionEvent::Schedule(this, GetVoltage(pcTimeline->GetTime()), 1, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// A transition where the slew passes the other input's voltage by the
// offset, or at its end if it ends within the offset of it; next tick if
// it doesn't move the voltage or the other input has none.  None if it
// starts on its end's side, or never ends (its chip has no power).
//
//////////////////////////////////////////////////////////////////////////
bool CAnalogComparatorInPort::TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew, bool bTraceWasUndriven)
{
	float32	fStartVoltage;
	float32	fEndVoltage;
	float32	fOther;
	float32	fThreshold;
	int64	iTransitionTime;

	if (pcSlew->GetSlewTime() == MAX_INT64)
	{
		return false;
	}
	fStartVoltage = pcSlew->GetStartVoltage();
	fEndVoltage = pcSlew->GetEndVoltage();
	fOther = NoVoltage();
	if ((mpcOther != NULL) && (mpcOther->GetTrace() != NULL))
	{
		fOther = mpcOther->GetTrace()->GetVoltage(pcTimeline->GetTime());
	}
	if ((fStartVoltage == fEndVoltage) || !IsVoltage(fOther))
	{
		CTransitionEvent::Schedule(this, fEndVoltage, 1, pcTimeline);
		return true;
	}

	if (fEndVoltage >= fOther + ANALOG_COMPARATOR_OFFSET)
	{
		fThreshold = fOther + ANALOG_COMPARATOR_OFFSET;
		if (fStartVoltage >= fThreshold)
		{
			return false;
		}
		iTransitionTime = CalculateHighTransitionTime(pcSlew, fStartVoltage, fThreshold);
	}
	else if (fEndVoltage <= fOther - ANALOG_COMPARATOR_OFFSET)
	{
		fThreshold = fOther - ANALOG_COMPARATOR_OFFSET;
		if (fStartVoltage <= fThreshold)
		{
			return false;
		}
		iTransitionTime = CalculateLowTransitionTime(pcSlew, fStartVoltage, fThreshold);
	}
	else
	{
		fThreshold = fEndVoltage;
		iTransitionTime = pcSlew->GetSlewTime();
	}
	CTransitionEvent::Schedule(this, fThreshold, (iTransitionTime > 0) ? iTransitionTime : 1, pcTimeline);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorInPort::TraceUndriven(CTimeline* pcTimeline)
{
	CTransitionEvent::Schedule(this, NoVoltage(), 1, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorInPort::TraceVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage)
{
	ReadTraceLevel(pcTimeline);
}

