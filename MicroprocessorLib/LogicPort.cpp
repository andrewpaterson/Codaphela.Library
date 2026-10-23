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
// Stops driving the trace 0.01ns from now.  Slews and drives the port
// has pending after then are dropped, so the last thing written wins;
// Logicim lets an earlier write's slew drive the trace again.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::WriteImpedance(CTimeline* pcTimeline)
{
	int64			iDelay;
	CArrayEventPtr	apcLater;

	iDelay = NanosecondsToTime(0.01);
	apcLater.Init();
	GetOutputEventsAfter(pcTimeline->GetTime() + iDelay, &apcLater);
	if (apcLater.NumElements() > 0)
	{
		pcTimeline->RemoveAll(&apcLater);
	}
	apcLater.Kill();

	CDriveEvent::Schedule(this, iDelay, NoVoltage(), pcTimeline);
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
// Takes the port's events off the timeline then disconnects from the
// trace, telling the ports that see it go undriven, see CPort.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::Disconnect(CTimeline* pcTimeline)
{
	RemoveEvents(pcTimeline);
	CPort::Disconnect(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the port's events off the timeline.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::RemoveEvents(CTimeline* pcTimeline)
{
	CArrayEventPtr	apcEvents;
	size			i;

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
	float32	fVoltageBefore;

	fVoltageBefore = (mpcTrace != NULL) ? mpcTrace->GetVoltage(pcTimeline->GetTime()) : NoVoltage();
	pcDriveEvent->CopyOutput(&mcOutput);
	NotifyIfTraceUndriven(pcTimeline, fVoltageBefore);
}


//////////////////////////////////////////////////////////////////////////
//
// If the port's new output has left its trace undriven, as a drive of
// NaN or a slew with no power can, the other ports on it are told.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::NotifyIfTraceUndriven(CTimeline* pcTimeline, float32 fVoltageBefore)
{
	if ((mpcTrace != NULL) && IsVoltage(fVoltageBefore))
	{
		if (!IsVoltage(mpcTrace->GetVoltage(pcTimeline->GetTime())))
		{
			mpcTrace->NotifyUndriven(pcTimeline, this);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A transition to undriven next tick, so the chip sees its input go.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::TraceUndriven(CTimeline* pcTimeline)
{
	CTransitionEvent::Schedule(this, NoVoltage(), 1, pcTimeline);
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
	bool			bTraceWasUndriven;
	float32			fVoltageBefore;

	fVoltageBefore = (mpcTrace != NULL) ? mpcTrace->GetVoltage(pcTimeline->GetTime()) : NoVoltage();
	bTraceWasUndriven = (mpcTrace != NULL) && !IsVoltage(fVoltageBefore);
	pcSlewEvent->Update(pcTimeline);
	pcSlewEvent->CopyOutput(&cSlew);

	if (mpcTrace != NULL)
	{
		for (i = 0; i < mpcTrace->NumConnectedPorts(); i++)
		{
			pcConnectedPort = mpcTrace->GetConnectedPort(i);
			if ((pcConnectedPort != this) && pcConnectedPort->IsLogicPort())
			{
				((CLogicPort*)pcConnectedPort)->TraceSlew(pcTimeline, &cSlew, bTraceWasUndriven);
			}
		}
	}

	mcOutput = cSlew;
	NotifyIfTraceUndriven(pcTimeline, fVoltageBefore);
}


//////////////////////////////////////////////////////////////////////////
//
// Schedules a transition for when another port's slew crosses this
// port's input threshold: the low threshold when the slew ends low, the
// high one when it ends high and halfway between them otherwise.  Returns
// true if one was scheduled.
//
// A slew that doesn't change the voltage crosses nothing, but if the
// trace was undriven before it the input has still changed, from
// undriven to the slew's level, so it is a transition next tick.  A slew
// onto an undriven trace never changes the voltage, see
// CSlewEvent::Update.  Logicim misses this, so driving low from undriven
// went unseen.
//
// Logicim also tries slews that never end, when the slewing chip has no
// power; working out their transition never ends either, so they are
// skipped here.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicPort::TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew, bool bTraceWasUndriven)
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
	if (iSlewTime == MAX_INT64)
	{
		return false;
	}
	if (fStartVoltage == fEndVoltage)
	{
		if (bTraceWasUndriven)
		{
			return CreateTransitionEventFromVoltage(pcTimeline, fEndVoltage);
		}
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
	CreateTransitionEventFromVoltage(pcTimeline, mpcTrace->GetVoltage(pcTimeline->GetTime()));
}


//////////////////////////////////////////////////////////////////////////
//
// A transition next tick if fVoltage is at a valid level for this port.
// Returns true if one was scheduled.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicPort::CreateTransitionEventFromVoltage(CTimeline* pcTimeline, float32 fVoltage)
{
	int64	iTime;
	float32	fLowVoltageIn;
	float32	fHighVoltageIn;

	if (mpcVoltageConfigurationSource == NULL)
	{
		return false;
	}

	iTime = pcTimeline->GetTime();
	fLowVoltageIn = mpcVoltageConfigurationSource->GetLowVoltageIn(GetVCC(iTime));
	fHighVoltageIn = mpcVoltageConfigurationSource->GetHighVoltageIn(GetVCC(iTime));

	if (fVoltage <= fLowVoltageIn)
	{
		CTransitionEvent::Schedule(this, fLowVoltageIn, 1, pcTimeline);
		return true;
	}
	else if (fVoltage >= fHighVoltageIn)
	{
		CTransitionEvent::Schedule(this, fHighVoltageIn, 1, pcTimeline);
		return true;
	}
	return false;
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
// The pending slews and drives after iTime.
//
//////////////////////////////////////////////////////////////////////////
void CLogicPort::GetOutputEventsAfter(int64 iTime, CArrayEventPtr* papcOutputEvents)
{
	size		i;
	CPortEvent*	pcEvent;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		pcEvent = mapcEvents.GetPtr(i);
		if (pcEvent->IsOutputEvent() && (pcEvent->GetTime() > iTime))
		{
			papcOutputEvents->Add(pcEvent);
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

