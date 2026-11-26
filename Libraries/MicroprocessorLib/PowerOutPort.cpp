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
#include "Voltage.h"
#include "Timeline.h"
#include "Trace.h"
#include "PortHolder.h"
#include "PowerStepEvent.h"
#include "PowerOutPort.h"


//////////////////////////////////////////////////////////////////////////
//
// On, at fVoltage, with no rise time.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Init(const char* szName, CPortHolder* pcHolder, float32 fVoltage)
{
	CPort::Init(PT_PowerOut, szName, pcHolder);
	mfVoltage = fVoltage;
	miRiseTime = 0;
	mpcStepEvent = NULL;
	SetSwitch(true, 0, fVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets its step, which the timeline has already dropped, and is on at
// its voltage until Start.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Reset(void)
{
	mpcStepEvent = NULL;
	SetSwitch(true, 0, mfVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::SetSwitch(bool bOn, int64 iSwitchTime, float32 fSwitchVoltage)
{
	mbOn = bOn;
	miSwitchTime = iSwitchTime;
	mfSwitchVoltage = fSwitchVoltage;
	mfToldVoltage = fSwitchVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage it had when it was last switched, then a step every
// CalculateStepTime towards its voltage (on) or 0V (off).
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerOutPort::GetVoltageOut(int64 iTime)
{
	int64	iStepTime;
	float32	fVoltage;

	if (iTime < miSwitchTime)
	{
		return mfSwitchVoltage;
	}
	iStepTime = CalculateStepTime();
	if (iStepTime == 0)
	{
		return mbOn ? mfVoltage : 0.0f;
	}
	if (mbOn)
	{
		fVoltage = mfSwitchVoltage + ((iTime - miSwitchTime) / iStepTime) * POWER_STEP_VOLTAGE;
		return (fVoltage < mfVoltage) ? fVoltage : mfVoltage;
	}
	else
	{
		fVoltage = mfSwitchVoltage - ((iTime - miSwitchTime) / iStepTime) * POWER_STEP_VOLTAGE;
		return (fVoltage > 0.0f) ? fVoltage : 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How long each step takes, at the rise time's rate; 0 with no rise time.
//
//////////////////////////////////////////////////////////////////////////
int64 CPowerOutPort::CalculateStepTime(void) const
{
	int64	iStepTime;

	if ((miRiseTime <= 0) || (mfVoltage <= 0.0f))
	{
		return 0;
	}
	iStepTime = (int64)(miRiseTime * POWER_STEP_VOLTAGE / mfVoltage);
	return (iStepTime < 1) ? 1 : iStepTime;
}


//////////////////////////////////////////////////////////////////////////
//
// As the simulation starts: on, rising from 0V over its rise time (at
// once with none), or off at 0V.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Start(CTimeline* pcTimeline, bool bOn)
{
	int64	iTime;

	iTime = pcTimeline->GetTime();
	if (bOn && (CalculateStepTime() == 0))
	{
		SetSwitch(true, iTime, mfVoltage);
	}
	else
	{
		SetSwitch(bOn, iTime, 0.0f);
		ScheduleNextStep(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starts it rising (bOn) or falling from the voltage it has now.  With no
// rise time it switches next tick.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Switch(CTimeline* pcTimeline, bool bOn)
{
	int64	iTime;
	float32	fVoltage;
	float32	fToldVoltage;

	iTime = pcTimeline->GetTime();
	fVoltage = GetVoltageOut(iTime);
	fToldVoltage = mfToldVoltage;
	RemoveStep(pcTimeline);
	SetSwitch(bOn, (CalculateStepTime() == 0) ? iTime + 1 : iTime, fVoltage);
	mfToldVoltage = fToldVoltage;
	ScheduleNextStep(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// A loaded state: as it was switched at iSwitchTime from fSwitchVoltage,
// its ports told of where it is now, and its next step scheduled.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::SetSwitched(CTimeline* pcTimeline, bool bOn, int64 iSwitchTime, float32 fSwitchVoltage)
{
	RemoveStep(pcTimeline);
	SetSwitch(bOn, iSwitchTime, fSwitchVoltage);
	mfToldVoltage = GetVoltageOut(pcTimeline->GetTime());
	ScheduleNextStep(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the trace's other ports, and its holder, that its voltage has
// moved, then schedules the next step.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Step(CTimeline* pcTimeline)
{
	float32	fOldVoltage;
	size	i;
	CPort*	pcPort;

	fOldVoltage = mfToldVoltage;
	mfToldVoltage = GetVoltageOut(pcTimeline->GetTime());
	if ((mpcTrace != NULL) && (mfToldVoltage != fOldVoltage))
	{
		for (i = 0; i < mpcTrace->NumConnectedPorts(); i++)
		{
			pcPort = mpcTrace->GetConnectedPort(i);
			if (pcPort != this)
			{
				pcPort->SupplyChanged(pcTimeline, fOldVoltage);
			}
		}
	}
	mpcHolder->TraceConnected(pcTimeline, this);
	if (mpcStepEvent == NULL)
	{
		ScheduleNextStep(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A step next tick, unless one is pending, so its holder looks again
// once everything else has started.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::StepNextTick(CTimeline* pcTimeline)
{
	if (mpcStepEvent == NULL)
	{
		mpcStepEvent = CPowerStepEvent::Schedule(this, pcTimeline->GetTime() + 1, pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The next step, unless it has arrived and its ports know.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::ScheduleNextStep(CTimeline* pcTimeline)
{
	int64	iTime;
	int64	iStepTime;
	int64	iNextTime;
	float32	fVoltage;

	iTime = pcTimeline->GetTime();
	fVoltage = GetVoltageOut(iTime);
	iStepTime = CalculateStepTime();
	if (iStepTime == 0)
	{
		if (GetVoltageOut(miSwitchTime) == mfToldVoltage)
		{
			return;
		}
		iNextTime = (miSwitchTime > iTime) ? miSwitchTime : iTime + 1;
	}
	else
	{
		if (HasArrived(fVoltage) && (fVoltage == mfToldVoltage))
		{
			return;
		}
		iNextTime = miSwitchTime + ((iTime - miSwitchTime) / iStepTime + 1) * iStepTime;
	}
	mpcStepEvent = CPowerStepEvent::Schedule(this, iNextTime, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPowerOutPort::HasArrived(float32 fVoltage) const
{
	return mbOn ? (fVoltage >= mfVoltage) : (fVoltage <= 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::RemoveStep(CTimeline* pcTimeline)
{
	if (mpcStepEvent != NULL)
	{
		pcTimeline->Remove(mpcStepEvent);
		mpcStepEvent = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called by its step as it executes or is removed.
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::StepRemoved(CPowerStepEvent* pcEvent)
{
	if (mpcStepEvent == pcEvent)
	{
		mpcStepEvent = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Power.VCC (3.3V)".
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::AppendDebugString(CChars* psz)
{
	psz->Append("Power.");
	AppendDescription(psz);
}


//////////////////////////////////////////////////////////////////////////
//
// Like "VCC (3.3V)".
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::AppendDescription(CChars* psz)
{
	psz->Append(mszName);
	psz->Append(" (");
	AppendVoltage(psz, mfVoltage);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerOutPort::GetVoltage(void) const { return mfVoltage; }
void CPowerOutPort::SetRiseTime(int64 iRiseTime) { miRiseTime = iRiseTime; }
int64 CPowerOutPort::GetRiseTime(void) const { return miRiseTime; }
bool CPowerOutPort::IsOn(void) const { return mbOn; }
int64 CPowerOutPort::GetSwitchTime(void) const { return miSwitchTime; }
float32 CPowerOutPort::GetSwitchVoltage(void) const { return mfSwitchVoltage; }

