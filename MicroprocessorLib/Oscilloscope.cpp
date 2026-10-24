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
#include "BaseLib/PointerFunctions.h"
#include "LongTime.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "Trace.h"
#include "Voltage.h"
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

	muiInputCount = pcPins->GetInputs()->NumElements();
	muiSampleCount = uiNumberOfDivsWide * uiSamplesPerDiv;
	mafMinVoltage = (float32*)malloc(muiInputCount * muiSampleCount * sizeof(float32));
	mafMaxVoltage = (float32*)malloc(muiInputCount * muiSampleCount * sizeof(float32));
	ResetState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Kill(void)
{
	SafeFree(mafMinVoltage);
	SafeFree(mafMaxVoltage);
	CIntegratedCircuit::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every sample 0 and the ring at its start.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::ResetState(void)
{
	memset(mafMinVoltage, 0, muiInputCount * muiSampleCount * sizeof(float32));
	memset(mafMaxVoltage, 0, muiInputCount * muiSampleCount * sizeof(float32));
	muiTickPosition = 0;
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
	size				i;

	CTickEvent::Schedule(miSampleTime, this, pcTimeline);
	papcInputs = GetOscilloscopePins()->GetInputs();
	muiTickPosition++;
	if (muiTickPosition >= muiSampleCount)
	{
		muiTickPosition = 0;
	}
	for (i = 0; i < papcInputs->NumElements(); i++)
	{
		Sample(i, papcInputs->GetPtr(i)->GetTrace(), pcTimeline->GetTime());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscope::GetType(void) { return OSCILLOSCOPE_TYPE; }
int64 COscilloscope::GetSampleTime(void) { return miSampleTime; }
COscilloscopePins* COscilloscope::GetOscilloscopePins(void) { return (COscilloscopePins*)mpcPins; }
size COscilloscope::GetInputCount(void) { return muiInputCount; }
size COscilloscope::GetSampleCount(void) { return muiSampleCount; }
size COscilloscope::GetTickPosition(void) { return muiTickPosition; }
float32 COscilloscope::GetMinVoltage(size uiInput, size uiSample) { return mafMinVoltage[uiInput * muiSampleCount + uiSample]; }
float32 COscilloscope::GetMaxVoltage(size uiInput, size uiSample) { return mafMaxVoltage[uiInput * muiSampleCount + uiSample]; }


//////////////////////////////////////////////////////////////////////////
//
// The trace's minimum and maximum voltage now, 0 if it's undriven or there
// isn't one, at the ring's position.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Sample(size uiInput, CTrace* pcTrace, int64 iTime)
{
	float32	fMinimum;
	float32	fMaximum;
	size	uiIndex;

	fMinimum = 0;
	fMaximum = 0;
	if (pcTrace != NULL)
	{
		fMinimum = pcTrace->GetMinimumVoltage(iTime);
		fMaximum = pcTrace->GetMaximumVoltage(iTime);
		if (!IsVoltage(fMinimum))
		{
			fMinimum = 0;
		}
		if (!IsVoltage(fMaximum))
		{
			fMaximum = 0;
		}
	}

	uiIndex = uiInput * muiSampleCount + muiTickPosition;
	mafMinVoltage[uiIndex] = fMinimum;
	mafMaxVoltage[uiIndex] = fMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
// For restoring a saved oscilloscope.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::SetVoltages(size uiInput, size uiSample, float32 fMinVoltage, float32 fMaxVoltage)
{
	if ((uiInput < muiInputCount) && (uiSample < muiSampleCount))
	{
		mafMinVoltage[uiInput * muiSampleCount + uiSample] = fMinVoltage;
		mafMaxVoltage[uiInput * muiSampleCount + uiSample] = fMaxVoltage;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::SetTickPosition(size uiTickPosition)
{
	if (uiTickPosition < muiSampleCount)
	{
		muiTickPosition = uiTickPosition;
	}
}

