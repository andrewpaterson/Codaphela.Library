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
#include "BaseLib/StringHelper.h"
#include "LongTime.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "StateXMLFields.h"
#include "ClockOscillator.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.  fFrequency is in hertz.
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CClockOscillatorPins> pPins, float32 fFrequency)
{
	Init(pcContainingSubcircuitSimulation, szName, pPins, fFrequency, FrequencyToTime(fFrequency) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CClockOscillatorPins> pPins, float32 fFrequency, int64 iInitialisationTime)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	miHalfCycleTime = FrequencyToTime(fFrequency) / 2;
	miInitialisationTime = iInitialisationTime;
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::Class(void)
{
	CIntegratedCircuit::Class();

	U_Int64(miHalfCycleTime);
	U_Int64(miInitialisationTime);
	U_Int64(miFullTicks);
	U_Bool(mbHigh);
}


//////////////////////////////////////////////////////////////////////////
//
// Low, with no full ticks.
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::ResetState(void)
{
	mbHigh = false;
	miFullTicks = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::SimulationStarted(CTimeline* pcTimeline)
{
	CreateTickEvent(pcTimeline, miInitialisationTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Toggles the output and schedules the next tick.  An unpowered clock
// stops ticking.
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::ExecuteTick(CTimeline* pcTimeline)
{
	CClockOscillatorPins*	pcPins;

	if (IsPowered(pcTimeline->GetTime()))
	{
		pcPins = GetClockOscillatorPins();
		mbHigh = !mbHigh;
		if (mbHigh)
		{
			miFullTicks++;
		}

		CreateTickEvent(pcTimeline, miHalfCycleTime);
		pcPins->GetOutput()->WriteBool(pcTimeline, mbHigh);
		if (pcPins->GetOutputInverse() != NULL)
		{
			pcPins->GetOutputInverse()->WriteBool(pcTimeline, !mbHigh);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Connecting a power port with power starts the clock ticking if it has
// stopped; without power the outputs stop driving.
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	CClockOscillatorPins*	pcPins;

	CIntegratedCircuit::TraceConnected(pcTimeline, pcPort);

	if (pcPort->IsPowerIn())
	{
		if (IsPowered(pcTimeline->GetTime()))
		{
			if (!HasTickEvent())
			{
				CreateTickEvent(pcTimeline, miHalfCycleTime);
			}
		}
		else
		{
			pcPins = GetClockOscillatorPins();
			pcPins->GetOutput()->WriteImpedance(pcTimeline);
			if (pcPins->GetOutputInverse() != NULL)
			{
				pcPins->GetOutputInverse()->WriteImpedance(pcTimeline);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::CreateTickEvent(CTimeline* pcTimeline, int64 iDelay)
{
	CTickEvent::Schedule(iDelay, this, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClockOscillator::HasTickEvent(void)
{
	size	i;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (StringCompare(mapcEvents.GetPtr(i)->ClassName(), "CTickEvent") == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage the clock is driving towards; low until its first tick.
//
//////////////////////////////////////////////////////////////////////////
float32 CClockOscillator::GetInternalVoltage(int64 iTime)
{
	return GetClockOscillatorPins()->GetOutput()->GetVoltageConfigurationSource()->GetVoltageOut(mbHigh, GetVCC(iTime));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockOscillator::GetType(void) { return CLOCK_OSCILLATOR_TYPE; }
int64 CClockOscillator::GetFullTicks(void) { return miFullTicks; }
int64 CClockOscillator::GetHalfCycleTime(void) { return miHalfCycleTime; }
CClockOscillatorPins* CClockOscillator::GetClockOscillatorPins(void) { return (CClockOscillatorPins*)&mpPins; }
void CClockOscillator::SetFullTicks(int64 iFullTicks) { miFullTicks = iFullTicks; }
bool CClockOscillator::IsHigh(void) { return mbHigh; }
void CClockOscillator::SetHigh(bool bHigh) { mbHigh = bHigh; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockOscillator::GetStateXMLName(void) { return "ClockOscillatorState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("high", IsHigh());
	pcWriter->Attribute("fullTicks", GetFullTicks());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClockOscillator::LoadXML(CMarkupTag* pcTag)
{
	bool	bHigh;
	int64	iFullTicks;

	if (!pcTag->GetBoolAttribute("high", &bHigh) || !pcTag->GetInt64Attribute("fullTicks", &iFullTicks))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false high and a whole number fullTicks.", NULL);
	}
	SetHigh(bHigh);
	SetFullTicks(iFullTicks);
	return true;
}

