/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <stdlib.h>
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/ClockOscillator.h"
#include "MicroprocessorLib/Constant.h"
#include "MicroprocessorLib/Counter.h"
#include "MicroprocessorLib/DTypeFlipFlop.h"
#include "MicroprocessorLib/Oscilloscope.h"
#include "StateXML.h"


CClockOscillatorStateXML	gcClockOscillatorStateXML;
CConstantStateXML			gcConstantStateXML;
CCounterStateXML			gcCounterStateXML;
CDTypeFlipFlopStateXML		gcDTypeFlipFlopStateXML;
COscilloscopeStateXML		gcOscilloscopeStateXML;

CStateXML* gapcStateXMLs[] =
{
	&gcClockOscillatorStateXML,
	&gcConstantStateXML,
	&gcCounterStateXML,
	&gcDTypeFlipFlopStateXML,
	&gcOscilloscopeStateXML,
};


//////////////////////////////////////////////////////////////////////////
//
// NULL for an IC that remembers nothing (gates, buffers).
//
//////////////////////////////////////////////////////////////////////////
CStateXML* FindStateXMLForIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit)
{
	size	i;

	for (i = 0; i < sizeof(gapcStateXMLs) / sizeof(CStateXML*); i++)
	{
		if (StringCompare(gapcStateXMLs[i]->GetIntegratedCircuitClassName(), pcIntegratedCircuit->ClassName()) == 0)
		{
			return gapcStateXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStateXML* FindStateXMLForElement(const char* szXMLName)
{
	size	i;

	for (i = 0; i < sizeof(gapcStateXMLs) / sizeof(CStateXML*); i++)
	{
		if (StringCompare(gapcStateXMLs[i]->GetXMLName(), szXMLName) == 0)
		{
			return gapcStateXMLs[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockOscillatorStateXML::GetIntegratedCircuitClassName(void) { return "CClockOscillator"; }
const char* CClockOscillatorStateXML::GetXMLName(void) { return "ClockOscillatorState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillatorStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	CClockOscillator*	pcClock;

	pcClock = (CClockOscillator*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("high", pcClock->IsHigh());
	pcWriter->Attribute("fullTicks", pcClock->GetFullTicks());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClockOscillatorStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	CClockOscillator*	pcClock;
	bool				bHigh;
	int64				iFullTicks;

	if (!pcTag->GetBoolAttribute("high", &bHigh) || !pcTag->GetInt64Attribute("fullTicks", &iFullTicks))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false high and a whole number fullTicks.", NULL);
	}
	pcClock = (CClockOscillator*)pcIntegratedCircuit;
	pcClock->SetHigh(bHigh);
	pcClock->SetFullTicks(iFullTicks);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstantStateXML::GetIntegratedCircuitClassName(void) { return "CConstant"; }
const char* CConstantStateXML::GetXMLName(void) { return "ConstantState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstantStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("value", ((CConstant*)pcIntegratedCircuit)->GetConstantValue());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConstantStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	int64	iValue;

	if (!pcTag->GetInt64Attribute("value", &iValue))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number value.", NULL);
	}
	((CConstant*)pcIntegratedCircuit)->SetConstantValue(iValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounterStateXML::GetIntegratedCircuitClassName(void) { return "CCounter"; }
const char* CCounterStateXML::GetXMLName(void) { return "CounterState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	CCounter*	pcCounter;

	pcCounter = (CCounter*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("count", (int64)pcCounter->GetCount());
	pcWriter->Attribute("terminal", pcCounter->IsTerminal());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCounterStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	CCounter*	pcCounter;
	int64		iCount;
	bool		bTerminal;

	if (!pcTag->GetInt64Attribute("count", &iCount) || (iCount < 0) || (iCount > 0xffffffffLL) || !pcTag->GetBoolAttribute("terminal", &bTerminal))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a count from 0 to 4294967295 and a true or false terminal.", NULL);
	}
	pcCounter = (CCounter*)pcIntegratedCircuit;
	pcCounter->SetCount((uint32)iCount);
	pcCounter->SetTerminal(bTerminal);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlopStateXML::GetIntegratedCircuitClassName(void) { return "CDTypeFlipFlop"; }
const char* CDTypeFlipFlopStateXML::GetXMLName(void) { return "DTypeFlipFlopState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("state", ((CDTypeFlipFlop*)pcIntegratedCircuit)->GetState());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDTypeFlipFlopStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	bool	bState;

	if (!pcTag->GetBoolAttribute("state", &bState))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false state.", NULL);
	}
	((CDTypeFlipFlop*)pcIntegratedCircuit)->SetState(bState);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscopeStateXML::GetIntegratedCircuitClassName(void) { return "COscilloscope"; }
const char* COscilloscopeStateXML::GetXMLName(void) { return "OscilloscopeState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeStateXML::Save(CXMLWriter* pcWriter, CIntegratedCircuit* pcIntegratedCircuit)
{
	COscilloscope*	pcScope;
	size			uiInput;
	size			uiSample;
	CChars			szMinimum;
	CChars			szMaximum;
	char			sz[48];

	pcScope = (COscilloscope*)pcIntegratedCircuit;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("inputCount", (int64)pcScope->GetInputCount());
	pcWriter->Attribute("sampleCount", (int64)pcScope->GetSampleCount());
	pcWriter->Attribute("tickPosition", (int64)pcScope->GetTickPosition());
	for (uiInput = 0; uiInput < pcScope->GetInputCount(); uiInput++)
	{
		szMinimum.Init();
		szMaximum.Init();
		for (uiSample = 0; uiSample < pcScope->GetSampleCount(); uiSample++)
		{
			if (uiSample != 0)
			{
				szMinimum.Append(' ');
				szMaximum.Append(' ');
			}
			snprintf(sz, sizeof(sz), "%.9g", (double)pcScope->GetMinVoltage(uiInput, uiSample));
			szMinimum.Append(sz);
			snprintf(sz, sizeof(sz), "%.9g", (double)pcScope->GetMaxVoltage(uiInput, uiSample));
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
	pcWriter->End();
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
bool COscilloscopeStateXML::Load(CMarkupTag* pcTag, CIntegratedCircuit* pcIntegratedCircuit)
{
	COscilloscope*	pcScope;
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

	pcScope = (COscilloscope*)pcIntegratedCircuit;
	if (!pcTag->GetInt64Attribute("inputCount", &iInputCount) || !pcTag->GetInt64Attribute("sampleCount", &iSampleCount) || !pcTag->GetInt64Attribute("tickPosition", &iTickPosition) ||
		(iInputCount != (int64)pcScope->GetInputCount()) || (iSampleCount != (int64)pcScope->GetSampleCount()) || (iTickPosition < 0) || (iTickPosition >= iSampleCount))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs the oscilloscope's inputCount and sampleCount and a tickPosition inside the samples.", NULL);
	}
	pcScope->SetTickPosition((size)iTickPosition);

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
				pcScope->SetVoltages((size)iInput, uiSample, afMinimum[uiSample], afMaximum[uiSample]);
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

