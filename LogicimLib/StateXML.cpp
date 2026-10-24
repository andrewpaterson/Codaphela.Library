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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/ClockOscillatorState.h"
#include "MicroprocessorLib/ConstantState.h"
#include "MicroprocessorLib/CounterState.h"
#include "MicroprocessorLib/DTypeFlipFlopState.h"
#include "MicroprocessorLib/OscilloscopeState.h"
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
// NULL for a state no helper handles (stateless ICs have nothing to save).
//
//////////////////////////////////////////////////////////////////////////
CStateXML* FindStateXMLForState(CState* pcState)
{
	size	i;

	for (i = 0; i < sizeof(gapcStateXMLs) / sizeof(CStateXML*); i++)
	{
		if (StringCompare(gapcStateXMLs[i]->GetStateClassName(), pcState->ClassName()) == 0)
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
const char* CClockOscillatorStateXML::GetStateClassName(void) { return "CClockOscillatorState"; }
const char* CClockOscillatorStateXML::GetXMLName(void) { return "ClockOscillatorState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockOscillatorStateXML::Save(CXMLWriter* pcWriter, CState* pcState)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("state", ((CClockOscillatorState*)pcState)->GetState());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CState* CClockOscillatorStateXML::Load(CMarkupTag* pcTag)
{
	CClockOscillatorState*	pcState;
	bool					bState;

	if (!pcTag->GetBoolAttribute("state", &bState))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false state.", NULL);
		return NULL;
	}
	pcState = NewMalloc<CClockOscillatorState>();
	pcState->Init(bState);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstantStateXML::GetStateClassName(void) { return "CConstantState"; }
const char* CConstantStateXML::GetXMLName(void) { return "ConstantState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstantStateXML::Save(CXMLWriter* pcWriter, CState* pcState)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("value", ((CConstantState*)pcState)->GetConstantValue());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CState* CConstantStateXML::Load(CMarkupTag* pcTag)
{
	CConstantState*	pcState;
	int64			iValue;

	if (!pcTag->GetInt64Attribute("value", &iValue))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number value.", NULL);
		return NULL;
	}
	pcState = NewMalloc<CConstantState>();
	pcState->Init(iValue);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounterStateXML::GetStateClassName(void) { return "CCounterState"; }
const char* CCounterStateXML::GetXMLName(void) { return "CounterState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterStateXML::Save(CXMLWriter* pcWriter, CState* pcState)
{
	CCounterState*	pcCounter;

	pcCounter = (CCounterState*)pcState;
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("count", (int64)pcCounter->GetState());
	pcWriter->Attribute("terminal", pcCounter->GetTerminal());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CState* CCounterStateXML::Load(CMarkupTag* pcTag)
{
	CCounterState*	pcState;
	int64			iCount;
	bool			bTerminal;

	if (!pcTag->GetInt64Attribute("count", &iCount) || (iCount < 0) || (iCount > 0xffffffffLL) || !pcTag->GetBoolAttribute("terminal", &bTerminal))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a count from 0 to 4294967295 and a true or false terminal.", NULL);
		return NULL;
	}
	pcState = NewMalloc<CCounterState>();
	pcState->Init((uint32)iCount);
	pcState->SetTerminal(bTerminal);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlopStateXML::GetStateClassName(void) { return "CDTypeFlipFlopState"; }
const char* CDTypeFlipFlopStateXML::GetXMLName(void) { return "DTypeFlipFlopState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopStateXML::Save(CXMLWriter* pcWriter, CState* pcState)
{
	pcWriter->Begin(GetXMLName());
	pcWriter->Attribute("state", ((CDTypeFlipFlopState*)pcState)->GetState());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CState* CDTypeFlipFlopStateXML::Load(CMarkupTag* pcTag)
{
	CDTypeFlipFlopState*	pcState;
	bool					bState;

	if (!pcTag->GetBoolAttribute("state", &bState))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a true or false state.", NULL);
		return NULL;
	}
	pcState = NewMalloc<CDTypeFlipFlopState>();
	pcState->Init(bState);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscopeStateXML::GetStateClassName(void) { return "COscilloscopeState"; }
const char* COscilloscopeStateXML::GetXMLName(void) { return "OscilloscopeState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeStateXML::Save(CXMLWriter* pcWriter, CState* pcState)
{
	COscilloscopeState*	pcScope;
	size				uiInput;
	size				uiSample;
	CChars				szMinimum;
	CChars				szMaximum;
	char				sz[48];

	pcScope = (COscilloscopeState*)pcState;
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
// Reads up to uiCount space separated floats.  Fails if there are fewer or
// anything else.
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
//
//////////////////////////////////////////////////////////////////////////
CState* COscilloscopeStateXML::Load(CMarkupTag* pcTag)
{
	COscilloscopeState*	pcState;
	int64				iInputCount;
	int64				iSampleCount;
	int64				iTickPosition;
	int64				iInput;
	STagIterator		sIter;
	CMarkupTag*			pcSamples;
	float32*			afMinimum;
	float32*			afMaximum;
	size				uiSample;
	bool				bValid;

	if (!pcTag->GetInt64Attribute("inputCount", &iInputCount) || !pcTag->GetInt64Attribute("sampleCount", &iSampleCount) || !pcTag->GetInt64Attribute("tickPosition", &iTickPosition) ||
		(iInputCount < 1) || (iSampleCount < 1) || (iTickPosition < 0) || (iTickPosition >= iSampleCount))
	{
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs inputCount and sampleCount of at least one and a tickPosition inside the samples.", NULL);
		return NULL;
	}

	pcState = NewMalloc<COscilloscopeState>();
	pcState->Init((size)iInputCount, (size)iSampleCount);
	pcState->SetTickPosition((size)iTickPosition);

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
				pcState->SetVoltages((size)iInput, uiSample, afMinimum[uiSample], afMaximum[uiSample]);
			}
		}
		pcSamples = pcTag->GetNextTag(&sIter);
	}
	free(afMaximum);
	free(afMinimum);

	if (!bValid)
	{
		SafeKill(pcState);
		gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a Samples element without an input in range and sampleCount min and max voltages.", NULL);
		return NULL;
	}
	return pcState;
}

