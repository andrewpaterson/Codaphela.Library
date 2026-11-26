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
#include "LongTime.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "StateXMLFields.h"
#include "ResetSupervisor.h"


//////////////////////////////////////////////////////////////////////////
//
// A DS1813-10's trip voltage and reset time.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CResetSupervisorPins> pPins)
{
	PreInit();
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	mfTripVoltage = RESET_SUPERVISOR_TRIP_VOLTAGE;
	miResetTime = NanosecondsToTime((int64)RESET_SUPERVISOR_RESET_TIME);
	ResetState();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::Class(void)
{
	CIntegratedCircuit::Class();
	U_Float32(mfTripVoltage);
	U_Int64(miResetTime);
	U_Bool(mbAsserting);
	U_Bool(mbPulledLow);
	U_Bool(mbPowerOnReset);
}


//////////////////////////////////////////////////////////////////////////
//
// It comes up holding reset, from power on.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::ResetState(void)
{
	mbAsserting = true;
	mbPulledLow = false;
	mbPowerOnReset = true;
}


//////////////////////////////////////////////////////////////////////////
//
// If VCC is already good, reset goes after the reset time.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::SimulationStarted(CTimeline* pcTimeline)
{
	if (GetVCC(pcTimeline->GetTime()) >= mfTripVoltage)
	{
		ScheduleRelease(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Low while it holds reset; otherwise the pull-up holds the line high.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::WriteOutputs(CTimeline* pcTimeline)
{
	if (mbAsserting)
	{
		GetResetSupervisorPins()->GetReset()->WriteBool(pcTimeline, false);
	}
	else
	{
		GetResetSupervisorPins()->GetReset()->WriteImpedance(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Under the trip voltage it holds reset at once; rising past it, reset
// goes after the reset time.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::VccChanged(CTimeline* pcTimeline, float32 fOldVcc)
{
	float32	fVcc;

	CIntegratedCircuit::VccChanged(pcTimeline, fOldVcc);
	fVcc = GetVCC(pcTimeline->GetTime());
	if (fVcc < mfTripVoltage)
	{
		Disconnect(pcTimeline);
		if (!mbAsserting)
		{
			Assert(pcTimeline, true);
		}
	}
	else if (fOldVcc < mfTripVoltage)
	{
		ScheduleRelease(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Its reset line pulled low from outside while it isn't holding it: when
// the pull ends it holds reset for the reset time.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	STraceValue	sValue;

	if (mbAsserting)
	{
		return;
	}
	sValue = pcPort->Read(pcTimeline);
	if (sValue.IsLow())
	{
		mbPulledLow = true;
	}
	else if (sValue.IsHigh() && mbPulledLow)
	{
		mbPulledLow = false;
		Assert(pcTimeline, false);
		ScheduleRelease(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The reset time is up: reset goes if VCC is still good.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::ExecuteTick(CTimeline* pcTimeline)
{
	if (mbAsserting && (GetVCC(pcTimeline->GetTime()) >= mfTripVoltage))
	{
		mbAsserting = false;
		mbPowerOnReset = false;
		WriteOutputs(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::Assert(CTimeline* pcTimeline, bool bPowerOn)
{
	mbAsserting = true;
	mbPowerOnReset = bPowerOn;
	WriteOutputs(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// The release, the reset time from now, replacing any pending.
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::ScheduleRelease(CTimeline* pcTimeline)
{
	Disconnect(pcTimeline);
	CTickEvent::Schedule(miResetTime, this, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// While it holds the reset from power on.
//
//////////////////////////////////////////////////////////////////////////
bool CResetSupervisor::IsPoweringUp(void)
{
	return mbPowered && mbAsserting && mbPowerOnReset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisor::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("asserting", mbAsserting);
	pcWriter->Attribute("pulledLow", mbPulledLow);
	pcWriter->Attribute("powerOnReset", mbPowerOnReset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CResetSupervisor::LoadXML(CMarkupTag* pcTag)
{
	return LoadStateBool(pcTag, "asserting", &mbAsserting) && LoadStateBool(pcTag, "pulledLow", &mbPulledLow) && LoadStateBool(pcTag, "powerOnReset", &mbPowerOnReset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CResetSupervisor::GetType(void) { return RESET_SUPERVISOR_TYPE; }
const char* CResetSupervisor::GetStateXMLName(void) const { return "ResetSupervisorState"; }
void CResetSupervisor::SetTripVoltage(float32 fTripVoltage) { mfTripVoltage = fTripVoltage; }
float32 CResetSupervisor::GetTripVoltage(void) const { return mfTripVoltage; }
void CResetSupervisor::SetResetTime(int64 iResetTime) { miResetTime = iResetTime; }
int64 CResetSupervisor::GetResetTime(void) const { return miResetTime; }
bool CResetSupervisor::IsAsserting(void) const { return mbAsserting; }
CResetSupervisorPins* CResetSupervisor::GetResetSupervisorPins(void) { return (CResetSupervisorPins*)&mpPins; }

