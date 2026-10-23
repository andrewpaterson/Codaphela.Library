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
#include "LongTime.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "Oscilloscope.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins.  fSampleFrequency is in hertz.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, COscilloscopePins* pcPins, float32 fSampleFrequency, size uiNumberOfDivsWide, size uiSamplesPerDiv)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);
	mfSampleFrequency = fSampleFrequency;
	miSampleTime = FrequencyToTime(fSampleFrequency);
	muiNumberOfDivsWide = uiNumberOfDivsWide;
	muiSamplesPerDiv = uiSamplesPerDiv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::SimulationStarted(CTimeline* pcTimeline)
{
	CTickEvent::Schedule(miSampleTime, this, pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Schedules the next sample, moves on and samples every input.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::ExecuteTick(CTimeline* pcTimeline)
{
	CArrayLogicPortPtr*	papcInputs;
	COscilloscopeState*	pcState;
	size				i;

	CTickEvent::Schedule(miSampleTime, this, pcTimeline);
	papcInputs = GetOscilloscopePins()->GetInputs();
	pcState = GetOscilloscopeState();
	pcState->Tick();
	for (i = 0; i < papcInputs->NumElements(); i++)
	{
		pcState->Sample(i, papcInputs->GetPtr(i)->GetTrace(), pcTimeline->GetTime());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CState* COscilloscope::CreateState(void)
{
	COscilloscopeState*	pcState;

	pcState = NewMalloc<COscilloscopeState>();
	pcState->Init(GetOscilloscopePins()->GetInputs()->NumElements(), muiNumberOfDivsWide * muiSamplesPerDiv);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscope::GetType(void) { return OSCILLOSCOPE_TYPE; }
int64 COscilloscope::GetSampleTime(void) { return miSampleTime; }
COscilloscopePins* COscilloscope::GetOscilloscopePins(void) { return (COscilloscopePins*)mpcPins; }
COscilloscopeState* COscilloscope::GetOscilloscopeState(void) { return (COscilloscopeState*)mpcState; }

