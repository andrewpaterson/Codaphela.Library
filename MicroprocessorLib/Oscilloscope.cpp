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
#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "LongTime.h"
#include "Timeline.h"
#include "TickEvent.h"
#include "Trace.h"
#include "Voltage.h"
#include "StateXMLFields.h"
#include "Oscilloscope.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.  fSampleFrequency is in hertz.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<COscilloscopePins> pPins, float32 fSampleFrequency, size uiNumberOfDivsWide, size uiSamplesPerDiv)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	mfSampleFrequency = fSampleFrequency;
	miSampleTime = FrequencyToTime(fSampleFrequency);
	muiNumberOfDivsWide = uiNumberOfDivsWide;
	muiSamplesPerDiv = uiSamplesPerDiv;

	muiInputCount = pPins->GetInputs()->NumElements();
	muiSampleCount = uiNumberOfDivsWide * uiSamplesPerDiv;
	mafMinVoltage = (float32*)malloc(muiInputCount * muiSampleCount * sizeof(float32));
	mafMaxVoltage = (float32*)malloc(muiInputCount * muiSampleCount * sizeof(float32));
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Class(void)
{
	CIntegratedCircuit::Class();

	U_Float32(mfSampleFrequency);
	U_Int64(miSampleTime);
	U_Size(muiNumberOfDivsWide);
	U_Size(muiSamplesPerDiv);
	U_Size(muiInputCount);
	U_Size(muiSampleCount);
	U_Pointer(mafMinVoltage);
	U_Pointer(mafMaxVoltage);
	U_Size(muiTickPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::Free(void)
{
	SafeFree(mafMinVoltage);
	SafeFree(mafMaxVoltage);
	CIntegratedCircuit::Free();
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
COscilloscopePins* COscilloscope::GetOscilloscopePins(void) { return (COscilloscopePins*)&mpPins; }
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscope::GetStateXMLName(void) { return "OscilloscopeState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscope::SaveXML(CXMLWriter* pcWriter)
{
	size			uiInput;
	size			uiSample;
	CChars			szMinimum;
	CChars			szMaximum;
	char			sz[48];

	pcWriter->Attribute("inputCount", (int64)GetInputCount());
	pcWriter->Attribute("sampleCount", (int64)GetSampleCount());
	pcWriter->Attribute("tickPosition", (int64)GetTickPosition());
	for (uiInput = 0; uiInput < GetInputCount(); uiInput++)
	{
		szMinimum.Init();
		szMaximum.Init();
		for (uiSample = 0; uiSample < GetSampleCount(); uiSample++)
		{
			if (uiSample != 0)
			{
				szMinimum.Append(' ');
				szMaximum.Append(' ');
			}
			snprintf(sz, sizeof(sz), "%.9g", (double)GetMinVoltage(uiInput, uiSample));
			szMinimum.Append(sz);
			snprintf(sz, sizeof(sz), "%.9g", (double)GetMaxVoltage(uiInput, uiSample));
			szMaximum.Append(sz);
		}
		pcWriter->Begin("Samples");
		pcWriter->Attribute("input", (int64)uiInput);
		pcWriter->Attribute("min", szMinimum.Text());
		pcWriter->Attribute("max", szMaximum.Text());
		pcWriter->End();
		szMaximum.Kill();
		szMinimum.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Reads exactly uiCount space separated floats.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadOscilloscopeVoltages(char* szText, float32* afVoltages, size uiCount)
{
	char*	szEnd;
	size	i;

	if (szText == NULL)
	{
		return false;
	}
	for (i = 0; i < uiCount; i++)
	{
		afVoltages[i] = (float32)strtod(szText, &szEnd);
		if (szEnd == szText)
		{
			return false;
		}
		szText = szEnd;
	}
	while (*szText == ' ')
	{
		szText++;
	}
	return *szText == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// Inputs without a <Samples> keep the samples they have.
//
//////////////////////////////////////////////////////////////////////////
bool COscilloscope::LoadXML(CMarkupTag* pcTag)
{
	int64			iInputCount;
	int64			iSampleCount;
	int64			iTickPosition;
	int64			iInput;
	STagIterator	sIter;
	CMarkupTag*		pcSamples;
	float32*		afMinimum;
	float32*		afMaximum;
	size			uiSample;
	bool			bValid;

	if (!pcTag->GetInt64Attribute("inputCount", &iInputCount) || !pcTag->GetInt64Attribute("sampleCount", &iSampleCount) || !pcTag->GetInt64Attribute("tickPosition", &iTickPosition) ||
		(iInputCount != (int64)GetInputCount()) || (iSampleCount != (int64)GetSampleCount()) || (iTickPosition < 0) || (iTickPosition >= iSampleCount))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs the oscilloscope's inputCount and sampleCount and a tickPosition inside the samples.", NULL);
	}
	SetTickPosition((size)iTickPosition);

	afMinimum = (float32*)malloc(sizeof(float32) * (size)iSampleCount);
	afMaximum = (float32*)malloc(sizeof(float32) * (size)iSampleCount);
	bValid = true;
	pcSamples = pcTag->GetTag("Samples", &sIter);
	while ((pcSamples != NULL) && bValid)
	{
		bValid = pcSamples->GetInt64Attribute("input", &iInput) && (iInput >= 0) && (iInput < iInputCount) &&
				 ReadOscilloscopeVoltages(pcSamples->GetStringAttribute("min"), afMinimum, (size)iSampleCount) &&
				 ReadOscilloscopeVoltages(pcSamples->GetStringAttribute("max"), afMaximum, (size)iSampleCount);
		if (bValid)
		{
			for (uiSample = 0; uiSample < (size)iSampleCount; uiSample++)
			{
				SetVoltages((size)iInput, uiSample, afMinimum[uiSample], afMaximum[uiSample]);
			}
		}
		pcSamples = pcTag->GetNextTag(&sIter);
	}
	free(afMaximum);
	free(afMinimum);

	if (!bValid)
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a Samples element without an input in range and sampleCount min and max voltages.", NULL);
	}
	return true;
}

