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
#include "DriveEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// Makes a drive iDelay from now and adds it to the timeline, which owns
// it.  NULL if it couldn't be added.
//
//////////////////////////////////////////////////////////////////////////
CDriveEvent* CDriveEvent::Schedule(CLogicPort* pcPort, int64 iDelay, float32 fVoltage, CTimeline* pcTimeline)
{
	CDriveEvent*	pcEvent;

	pcEvent = NewMalloc<CDriveEvent>();
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
void CDriveEvent::Init(CLogicPort* pcPort, int64 iTime, float32 fVoltage)
{
	CPortOutputEvent::Init(pcPort, iTime);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDriveEvent::Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fVoltage)
{
	CPortOutputEvent::Init(pcPort, iTime, iID);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDriveEvent::Execute(CTimeline* pcTimeline)
{
	CPortOutputEvent::Execute(pcTimeline);
	mpcPort->DriveEvent(pcTimeline, this);
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage from the drive's time on.
//
//////////////////////////////////////////////////////////////////////////
float32 CDriveEvent::GetVoltage(int64 iTime)
{
	if (iTime >= miTime)
	{
		return mfVoltage;
	}
	else
	{
		return NoVoltage();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDriveEvent::CopyOutput(CPortOutput* pcOutput)
{
	pcOutput->InitDrive(miTime, mfVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDriveEvent::IsDriveEvent(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Drive: Time [1024] 3.3V".
//
//////////////////////////////////////////////////////////////////////////
void CDriveEvent::PrintShort(CChars* psz)
{
	CPortOutputEvent::PrintShort(psz);
	psz->Append(' ');
	AppendVoltage(psz, mfVoltage);
}

