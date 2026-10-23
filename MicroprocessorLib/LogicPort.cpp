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
#include "BaseLib/Logger.h"
#include "LongTime.h"
#include "Voltage.h"
#include "Timeline.h"
#include "Trace.h"
#include "Pins.h"
#include "PowerInPort.h"
#include "DriveEvent.h"
#include "SlewEvent.h"
#include "TransitionEvent.h"
#include "LogicPort.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the port to pcPins, which owns it.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Init(EPortType eType, CPins* pcPins, const char* szName, CVoltageConfigurationSource* pcVoltageConfigurationSource)
{
	CPort::Init(eType, szName, pcPins);
	mpcVoltageConfigurationSource = pcVoltageConfigurationSource;
	mapcEvents.Init();
	mcOutput.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The port's events must already have been taken off the timeline, see
// Disconnect.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Kill(void)
{
	mapcEvents.Kill();
	CPort::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// After the events at or before its time, so events at the same time
// stay in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Add(CPortEvent* pcEvent)
{
	size	i;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (mapcEvents.GetPtr(i)->GetTime() > pcEvent->GetTime())
		{
			mapcEvents.InsertAt(&pcEvent, i);
			return;
		}
	}
	mapcEvents.Add(pcEvent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Remove(CPortEvent* pcEvent)
{
	size	i;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (mapcEvents.GetPtr(i) == pcEvent)
		{
			mapcEvents.RemoveAt(i, true);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Slews to the family's output voltage for bValue after its hold time.
// Nothing happens when the chip has no power.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::WriteBool(CTimeline* pcTimeline, bool bValue)
{
	int64	iTime;
	float32	fVcc;
	float32	fOutVoltage;
	int64	iHoldTime;

	iTime = pcTimeline->GetTime();
	fVcc = GetVCC(iTime);

	fOutVoltage = mpcVoltageConfigurationSource->GetVoltageOut(bValue, fVcc);
	iHoldTime = mpcVoltageConfigurationSource->CalculateHoldTime(fOutVoltage, GetVoltageOut(iTime), fVcc);
	if (iHoldTime != MAX_INT64)
	{
		CSlewEvent::Schedule(this, fOutVoltage, iHoldTime, pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Write(CTimeline* pcTimeline, bool bValue)
{
	WriteBool(pcTimeline, bValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Stops driving the trace 0.01ns from now.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::WriteImpedance(CTimeline* pcTimeline)
{
	CDriveEvent::Schedule(this, NanosecondsToTime(0.01), NoVoltage(), pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// Undriven when the port isn't connected or nothing drives its trace.
//
//////////////////////////////////////////////////////////////////////////
STraceValue CLogicPort::ReadValue(int64 iTime)
{
	float32		fVoltage;
	STraceValue	sValue;

	if (mpcTrace != NULL)
	{
		fVoltage = mpcTrace->GetVoltage(iTime);
		if (IsVoltage(fVoltage))
		{
			sValue.Init(mpcVoltageConfigurationSource->GetValue(fVoltage, GetVCC(iTime)));
			return sValue;
		}
	}
	sValue.Init(TV_Undriven);
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STraceValue CLogicPort::Read(CTimeline* pcTimeline)
{
	return ReadValue(pcTimeline->GetTime());
}


//////////////////////////////////////////////////////////////////////////
//
// Disconnects from the trace and takes the port's events off the
// timeline.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Disconnect(CTimeline* pcTimeline)
{
	CArrayEventPtr	apcEvents;
	size			i;

	CPort::Disconnect(pcTimeline);

	apcEvents.Init();
	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		apcEvents.Add(mapcEvents.GetPtr(i));
	}
	pcTimeline->RemoveAll(&apcEvents);
	apcEvents.Kill();
	mapcEvents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the output of the drive, which is executing.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::DriveEvent(CTimeline* pcTimeline, CDriveEvent* pcDriveEvent)
{
	pcDriveEvent->CopyOutput(&mcOutput);
}


//////////////////////////////////////////////////////////////////////////
//
// Works out the slew, which is executing, tells the other logic ports on
// the trace about it and then drives it.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::SlewEvent(CTimeline* pcTimeline, CSlewEvent* pcSlewEvent)
{
	CPortOutput		cSlew;
	size			i;
	CPort*			pcConnectedPort;

	pcSlewEvent->Update(pcTimeline);
	pcSlewEvent->CopyOutput(&cSlew);

	if (mpcTrace != NULL)
	{
		for (i = 0; i < mpcTrace->NumConnectedPorts(); i++)
		{
			pcConnectedPort = mpcTrace->GetConnectedPort(i);
			if ((pcConnectedPort != this) && pcConnectedPort->IsLogicPort())
			{
				((CLogicPort*)pcConnectedPort)->TraceSlew(pcTimeline, &cSlew);
			}
		}
	}

	mcOutput = cSlew;
}


//////////////////////////////////////////////////////////////////////////
//
// Schedules a transition for when another port's slew crosses this
// port's input threshold: the low threshold when the slew ends low, the
// high one when it ends high and halfway between them otherwise.  Returns
// true if one was scheduled.
//
// Logicim also tries slews that never end, when the slewing chip has no
// power; working out their transition never ends either, so they are
// skipped here.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicPort::TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew)
{
	float32	fStartVoltage;
	int64	iSlewTime;
	float32	fEndVoltage;
	int64	iTime;
	float32	fLowVoltageIn;
	float32	fHighVoltageIn;
	int64	iTransitionTime;
	float32	fTransitionVoltage;

	if (mpcVoltageConfigurationSource == NULL)
	{
		return false;
	}

	fStartVoltage = pcSlew->GetStartVoltage();
	iSlewTime = pcSlew->GetSlewTime();
	fEndVoltage = pcSlew->GetEndVoltage();
	if ((fStartVoltage == fEndVoltage) || (iSlewTime == MAX_INT64))
	{
		return false;
	}

	iTime = pcTimeline->GetTime();
	fLowVoltageIn = mpcVoltageConfigurationSource->GetLowVoltageIn(GetVCC(iTime));
	fHighVoltageIn = mpcVoltageConfigurationSource->GetHighVoltageIn(GetVCC(iTime));
	if (fEndVoltage <= fLowVoltageIn)
	{
		fTransitionVoltage = fLowVoltageIn;
		iTransitionTime = CalculateLowTransitionTime(pcSlew, fStartVoltage, fEndVoltage, fTransitionVoltage);
	}
	else if (fEndVoltage >= fHighVoltageIn)
	{
		fTransitionVoltage = fHighVoltageIn;
		iTransitionTime = CalculateHighTransitionTime(pcSlew, fStartVoltage, fEndVoltage, fTransitionVoltage);
	}
	else
	{
		fTransitionVoltage = (fHighVoltageIn + fLowVoltageIn) / 2;
		iTransitionTime = CalculateTransitionTime(fStartVoltage, fEndVoltage, iSlewTime, fTransitionVoltage);
	}

	if (iTransitionTime > 0)
	{
		CTransitionEvent::Schedule(this, fTransitionVoltage, iTransitionTime, pcTimeline);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Called once the port's trace is connected.  A slew in progress on the
// trace schedules a transition, otherwise a trace already at a valid
// level does, straight away.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::TraceConnected(CTimeline* pcTimeline)
{
	size			i;
	CPort*			pcConnectedPort;
	CPortOutput*	pcOutput;
	bool			bSlewInProgress;

	CPort::TraceConnected(pcTimeline);

	bSlewInProgress = false;
	for (i = 0; i < mpcTrace->NumConnectedPorts(); i++)
	{
		pcConnectedPort = mpcTrace->GetConnectedPort(i);
		if (pcConnectedPort->IsLogicPort())
		{
			pcOutput = ((CLogicPort*)pcConnectedPort)->GetOutput();
			if (pcOutput->IsSlew() && pcOutput->ForTime(pcTimeline->GetTime()))
			{
				bSlewInProgress |= TraceSlew(pcTimeline, pcOutput);
			}
		}
	}

	if (!bSlewInProgress)
	{
		CreateTransitionEventFromExistingVoltage(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::CreateTransitionEventFromExistingVoltage(CTimeline* pcTimeline)
{
	int64	iTime;
	float32	fEndVoltage;
	float32	fLowVoltageIn;
	float32	fHighVoltageIn;

	if (mpcVoltageConfigurationSource == NULL)
	{
		return;
	}

	iTime = pcTimeline->GetTime();
	fEndVoltage = mpcTrace->GetVoltage(iTime);
	fLowVoltageIn = mpcVoltageConfigurationSource->GetLowVoltageIn(GetVCC(iTime));
	fHighVoltageIn = mpcVoltageConfigurationSource->GetHighVoltageIn(GetVCC(iTime));

	if (fEndVoltage <= fLowVoltageIn)
	{
		CTransitionEvent::Schedule(this, fLowVoltageIn, 1, pcTimeline);
	}
	else if (fEndVoltage >= fHighVoltageIn)
	{
		CTransitionEvent::Schedule(this, fHighVoltageIn, 1, pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The straight line estimate, then stepped on until the slew's voltage is
// really at or below the threshold.
//
//////////////////////////////////////////////////////////////////////////
int64 CLogicPort::CalculateLowTransitionTime(CPortOutput* pcSlew, float32 fStartVoltage, float32 fEndVoltage, float32 fLowVoltageIn)
{
	int64	iTransitionTime;
	float32	fVoltage;

	iTransitionTime = CalculateTransitionTime(fStartVoltage, fEndVoltage, pcSlew->GetSlewTime(), fLowVoltageIn);
	for (;;)
	{
		fVoltage = pcSlew->CalculateVoltageAtTime(pcSlew->GetTime() + iTransitionTime, fStartVoltage);
		if (fVoltage > fLowVoltageIn)
		{
			iTransitionTime++;
		}
		else
		{
			return iTransitionTime;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CLogicPort::CalculateHighTransitionTime(CPortOutput* pcSlew, float32 fStartVoltage, float32 fEndVoltage, float32 fHighVoltageIn)
{
	int64	iTransitionTime;
	float32	fVoltage;

	iTransitionTime = CalculateTransitionTime(fStartVoltage, fEndVoltage, pcSlew->GetSlewTime(), fHighVoltageIn);
	for (;;)
	{
		fVoltage = pcSlew->CalculateVoltageAtTime(pcSlew->GetTime() + iTransitionTime, fStartVoltage);
		if (fVoltage < fHighVoltageIn)
		{
			iTransitionTime++;
		}
		else
		{
			return iTransitionTime;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// When the line from fStartVoltage to fEndVoltage over iSlewTime reaches
// fY.
//
//////////////////////////////////////////////////////////////////////////
int64 CLogicPort::CalculateTransitionTime(float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime, float32 fY)
{
	float32	fC;
	float32	fM;

	fC = fStartVoltage;
	fM = (fEndVoltage - fStartVoltage) / iSlewTime;
	return RoundFloat32ToInt64((fY - fC) / fM);
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the chip an input has changed.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::TransitionEvent(CTimeline* pcTimeline, CTransitionEvent* pcEvent)
{
	GetPins()->InputTransition(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
// The pending drives at or after iTime.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::GetFutureDriveEvents(int64 iTime, CArrayEventPtr* papcDriveEvents)
{
	size		i;
	CPortEvent*	pcEvent;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		pcEvent = mapcEvents.GetPtr(i);
		if (pcEvent->IsDriveEvent() && (pcEvent->GetTime() >= iTime))
		{
			papcDriveEvents->Add(pcEvent);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the output and the events, which the timeline has already
// dropped.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Reset(void)
{
	mcOutput.Init();
	mapcEvents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CLogicPort::GetVoltageOut(int64 iTime)
{
	return mcOutput.GetVoltage(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CLogicPort::GetVCC(int64 iTime)
{
	return GetPins()->GetVoltageCommon()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CLogicPort::GetGND(int64 iTime)
{
	return GetPins()->GetVoltageGround()->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Events [2]:  Slew: Time [1024],  Drive: Time [2048],  ".
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::AppendEvents(CChars* psz)
{
	size	i;

	psz->Append("Events [");
	psz->Append(mapcEvents.NumElements());
	psz->Append("]:  ");
	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		mapcEvents.GetPtr(i)->PrintShort(psz);
		psz->Append(",  ");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogicPort::NumEvents(void) { return mapcEvents.NumElements(); }
CPortEvent* CLogicPort::GetEvent(size uiIndex) { return mapcEvents.SafeGetPtr(uiIndex); }
bool CLogicPort::IsLogicPort(void) { return true; }
CVoltageConfigurationSource* CLogicPort::GetVoltageConfigurationSource(void) { return mpcVoltageConfigurationSource; }
CPortOutput* CLogicPort::GetOutput(void) { return &mcOutput; }
CPins* CLogicPort::GetPins(void) { return (CPins*)mpcHolder; }

