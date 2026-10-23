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
#include "Voltage.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "ClockOscillator.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins.  fFrequency is in hertz.
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CClockOscillatorPins* pcPins, float32 fFrequency)
{
	Init(pcContainingSubcircuitSimulation, szName, pcPins, fFrequency, FrequencyToTime(fFrequency) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CClockOscillatorPins* pcPins, float32 fFrequency, int64 iInitialisationTime)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);
	miHalfCycleTime = FrequencyToTime(fFrequency) / 2;
	miInitialisationTime = iInitialisationTime;
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
	CClockOscillatorState*	pcState;
	CClockOscillatorPins*	pcPins;

	if (IsPowered(pcTimeline->GetTime()))
	{
		pcState = GetClockOscillatorState();
		pcPins = GetClockOscillatorPins();
		pcState->Tick();
		if (pcState->GetState())
		{
			miFullTicks++;
		}

		CreateTickEvent(pcTimeline, miHalfCycleTime);
		pcPins->GetOutput()->WriteBool(pcTimeline, pcState->GetState());
		if (pcPins->GetOutputInverse() != NULL)
		{
			pcPins->GetOutputInverse()->WriteBool(pcTimeline, !pcState->GetState());
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
// The voltage the clock is driving towards.  NaN, with an error logged,
// before the simulation has started.
//
//////////////////////////////////////////////////////////////////////////
float32 CClockOscillator::GetInternalVoltage(int64 iTime)
{
	if (mpcState != NULL)
	{
		return GetClockOscillatorPins()->GetOutput()->GetVoltageConfigurationSource()->GetVoltageOut(GetClockOscillatorState()->GetState(), GetVCC(iTime));
	}
	gcLogger.Error2(__METHOD__, " Clock without state cannot get internal voltage.", NULL);
	return NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
// Starting low.
//
//////////////////////////////////////////////////////////////////////////
CState* CClockOscillator::CreateState(void)
{
	CClockOscillatorState*	pcState;

	pcState = NewMalloc<CClockOscillatorState>();
	pcState->Init(false);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockOscillator::GetType(void) { return CLOCK_OSCILLATOR_TYPE; }
int64 CClockOscillator::GetFullTicks(void) { return miFullTicks; }
int64 CClockOscillator::GetHalfCycleTime(void) { return miHalfCycleTime; }
CClockOscillatorPins* CClockOscillator::GetClockOscillatorPins(void) { return (CClockOscillatorPins*)mpcPins; }
CClockOscillatorState* CClockOscillator::GetClockOscillatorState(void) { return (CClockOscillatorState*)mpcState; }

