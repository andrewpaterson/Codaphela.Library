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
#include <stdlib.h>
#include <string.h>
#include "Voltage.h"
#include "Trace.h"
#include "OscilloscopeState.h"


//////////////////////////////////////////////////////////////////////////
//
// Every sample starts at 0V.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeState::Init(size uiInputCount, size uiSampleCount)
{
	size	uiBytes;

	muiInputCount = uiInputCount;
	muiSampleCount = uiSampleCount;
	uiBytes = uiInputCount * uiSampleCount * sizeof(float32);
	mafMinVoltage = (float32*)malloc(uiBytes);
	mafMaxVoltage = (float32*)malloc(uiBytes);
	memset(mafMinVoltage, 0, uiBytes);
	memset(mafMaxVoltage, 0, uiBytes);
	muiTickPosition = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeState::Kill(void)
{
	free(mafMinVoltage);
	free(mafMaxVoltage);
	mafMinVoltage = NULL;
	mafMaxVoltage = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves on to the next sample, wrapping to the first.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeState::Tick(void)
{
	muiTickPosition++;
	if (muiTickPosition >= muiSampleCount)
	{
		muiTickPosition = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The current sample of uiInput.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeState::Sample(size uiInput, CTrace* pcTrace, int64 iTime)
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
//
//////////////////////////////////////////////////////////////////////////
float32 COscilloscopeState::GetMinVoltage(size uiInput, size uiSample) { return mafMinVoltage[uiInput * muiSampleCount + uiSample]; }
float32 COscilloscopeState::GetMaxVoltage(size uiInput, size uiSample) { return mafMaxVoltage[uiInput * muiSampleCount + uiSample]; }
size COscilloscopeState::GetTickPosition(void) { return muiTickPosition; }
size COscilloscopeState::GetSampleCount(void) { return muiSampleCount; }
size COscilloscopeState::GetInputCount(void) { return muiInputCount; }


//////////////////////////////////////////////////////////////////////////
//
// For restoring a saved oscilloscope.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeState::SetVoltages(size uiInput, size uiSample, float32 fMinVoltage, float32 fMaxVoltage)
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
void COscilloscopeState::SetTickPosition(size uiTickPosition)
{
	if (uiTickPosition < muiSampleCount)
	{
		muiTickPosition = uiTickPosition;
	}
}

