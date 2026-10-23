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
#include "BaseLib/Logger.h"
#include "Voltage.h"
#include "Timeline.h"
#include "LogicPort.h"
#include "Trace.h"
#include "DriveEvent.h"
#include "SlewEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// Makes a slew iDelay from now and adds it to the timeline, which owns
// it.  NULL if it couldn't be added.
//
//////////////////////////////////////////////////////////////////////////
CSlewEvent* CSlewEvent::Schedule(CLogicPort* pcPort, float32 fEndVoltage, int64 iDelay, CTimeline* pcTimeline)
{
	CSlewEvent*	pcEvent;

	pcEvent = NewMalloc<CSlewEvent>();
	pcEvent->Init(pcPort, pcTimeline->GetTime() + iDelay, fEndVoltage);
	if (pcTimeline->AddNewFutureEvent(pcEvent))
	{
		return pcEvent;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The start voltage and slew time are worked out when it executes.
//
//////////////////////////////////////////////////////////////////////////
void CSlewEvent::Init(CLogicPort* pcPort, int64 iTime, float32 fEndVoltage)
{
	CPortOutputEvent::Init(pcPort, iTime);
	mfStartVoltage = NoVoltage();
	mfEndVoltage = fEndVoltage;
	miSlewTime = -1;
}


//////////////////////////////////////////////////////////////////////////
//
// For a loaded event.
//
//////////////////////////////////////////////////////////////////////////
void CSlewEvent::Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime)
{
	CPortOutputEvent::Init(pcPort, iTime, iID);
	mfStartVoltage = fStartVoltage;
	mfEndVoltage = fEndVoltage;
	miSlewTime = iSlewTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSlewEvent::Execute(CTimeline* pcTimeline)
{
	CPortOutputEvent::Execute(pcTimeline);
	mpcPort->SlewEvent(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
// Starts from the voltage the port is driving now, or the family's low
// voltage out when it drives nothing, and works out the slew time.  The
// port's later drives are replaced by one at the end of the slew, which
// is returned.  NULL when the chip has no power.
//
// An undriven trace takes the new level at once: the slew starts at its
// end voltage.  Logicim slews it up from the low voltage out, so an
// undriven trace being driven high read low for a while, see
// CLogicPort::TraceSlew.
//
//////////////////////////////////////////////////////////////////////////
CDriveEvent* CSlewEvent::Update(CTimeline* pcTimeline)
{
	CVoltageConfigurationSource*	pcVoltageConfiguration;
	float32							fVcc;
	int64							iNowTime;
	float32							fStartVoltage;
	float32							fPortVoltage;
	CArrayEventPtr					apcDriveEvents;

	pcVoltageConfiguration = mpcPort->GetVoltageConfigurationSource();
	fVcc = mpcPort->GetVCC(miTime);

	iNowTime = pcTimeline->GetTime();
	if ((mpcPort->GetTrace() != NULL) && !IsVoltage(mpcPort->GetTrace()->GetVoltage(iNowTime)))
	{
		mfStartVoltage = mfEndVoltage;
	}
	else
	{
		fStartVoltage = CalculateStartVoltage(iNowTime, pcVoltageConfiguration, fVcc);
		fPortVoltage = CalculateVoltageAtTime(iNowTime, fStartVoltage);
		mfStartVoltage = pcVoltageConfiguration->CalculateStartVoltage(fPortVoltage, fVcc);
	}

	miSlewTime = CalculateSlewTime(pcVoltageConfiguration, fVcc);
	if (miSlewTime == MAX_INT64)
	{
		return NULL;
	}

	if (miSlewTime < 1)
	{
		gcLogger.Error2(__METHOD__, " Slew time must be in the future.", NULL);
		miSlewTime = MAX_INT64;
		return NULL;
	}

	apcDriveEvents.Init();
	mpcPort->GetFutureDriveEvents(miTime, &apcDriveEvents);
	if (apcDriveEvents.NumElements() > 0)
	{
		pcTimeline->RemoveAll(&apcDriveEvents);
	}
	apcDriveEvents.Kill();

	return CDriveEvent::Schedule(mpcPort, miSlewTime, mfEndVoltage, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// The time to slew from the start to the end voltage at the family's
// rate, truncated; 1 when they are the same and MAX_INT64 with no power.
//
//////////////////////////////////////////////////////////////////////////
int64 CSlewEvent::CalculateSlewTime(CVoltageConfigurationSource* pcVoltageConfiguration, float32 fVcc)
{
	float32	fVoltageDiff;
	float32	fVoltsPerTime;

	fVoltageDiff = mfEndVoltage - mfStartVoltage;
	if (fVoltageDiff > 0)
	{
		fVoltsPerTime = pcVoltageConfiguration->GetVoltsPerTimeLowToHigh(fVcc);
		if (!IsInfinite(fVoltsPerTime))
		{
			return (int64)(fVoltageDiff / fVoltsPerTime);
		}
		return MAX_INT64;
	}
	else if (fVoltageDiff < 0)
	{
		fVoltsPerTime = pcVoltageConfiguration->GetVoltsPerTimeHighToLow(fVcc);
		if (!IsInfinite(fVoltsPerTime))
		{
			return -(int64)(fVoltageDiff / fVoltsPerTime);
		}
		return MAX_INT64;
	}
	else
	{
		return 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSlewEvent::CalculateStartVoltage(int64 iNowTime, CVoltageConfigurationSource* pcVoltageConfiguration, float32 fVcc)
{
	float32	fVoltage;

	fVoltage = mpcPort->GetVoltageOut(iNowTime);
	if (IsVoltage(fVoltage))
	{
		return fVoltage;
	}
	return pcVoltageConfiguration->GetVoltageOut(false, fVcc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSlewEvent::CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage)
{
	CPortOutput	cOutput;

	CopyOutput(&cOutput);
	return cOutput.CalculateVoltageAtTime(iTime, fStartVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSlewEvent::GetVoltage(int64 iTime)
{
	CPortOutput	cOutput;

	CopyOutput(&cOutput);
	return cOutput.GetVoltage(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSlewEvent::ForTime(int64 iTime)
{
	CPortOutput	cOutput;

	CopyOutput(&cOutput);
	return cOutput.ForTime(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CSlewEvent::GetEndTime(void)
{
	CPortOutput	cOutput;

	CopyOutput(&cOutput);
	return cOutput.GetEndTime();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSlewEvent::CopyOutput(CPortOutput* pcOutput)
{
	pcOutput->InitSlew(miTime, mfStartVoltage, mfEndVoltage, miSlewTime);
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Slew: Time [1024] 0.2V -> 3.1V".
//
//////////////////////////////////////////////////////////////////////////
void CSlewEvent::PrintShort(CChars* psz)
{
	CPortOutputEvent::PrintShort(psz);
	psz->Append(' ');
	AppendVoltage(psz, mfStartVoltage);
	psz->Append(" -> ");
	AppendVoltage(psz, mfEndVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSlewEvent::GetStartVoltage(void) { return mfStartVoltage; }
float32 CSlewEvent::GetEndVoltage(void) { return mfEndVoltage; }
int64 CSlewEvent::GetSlewTime(void) { return miSlewTime; }

