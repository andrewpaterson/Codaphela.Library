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
#include "Timeline.h"
#include "GateArray.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CGateArray::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGateArrayPins> pPins, EGateFunction eFunction)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	meFunction = eFunction;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGateArray::Class(void)
{
	CIntegratedCircuit::Class();

	U_Enum(meFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGateArray::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Only the gate the input is of changes.
//
//////////////////////////////////////////////////////////////////////////
void CGateArray::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	size	uiGate;

	if (IsPowered(pcTimeline->GetTime()))
	{
		uiGate = GetGateArrayPins()->FindGate(pcPort);
		if (uiGate != ARRAY_ELEMENT_NOT_FOUND)
		{
			UpdateGate(pcTimeline, uiGate);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An AND's low input decides it and an OR's high one does, whatever the
// others are; otherwise an undriven input leaves the output undriven.  An
// exclusive OR needs every input.
//
//////////////////////////////////////////////////////////////////////////
void CGateArray::UpdateGate(CTimeline* pcTimeline, size uiGate)
{
	CGateArrayPins*		pcPins;
	CLogicPort*			pcOutput;
	size				i;
	size				uiHighs;
	size				uiLows;
	size				uiUndriven;
	STraceValue			sValue;
	bool				bNegated;

	pcPins = GetGateArrayPins();
	uiHighs = 0;
	uiLows = 0;
	uiUndriven = 0;
	for (i = 0; i < pcPins->GetInputCount(); i++)
	{
		sValue = pcPins->GetInput(uiGate, i)->Read(pcTimeline);
		if (sValue.IsHigh())
		{
			uiHighs++;
		}
		else if (sValue.IsLow())
		{
			uiLows++;
		}
		else if (sValue.eValue == TV_Undriven)
		{
			uiUndriven++;
		}
	}

	pcOutput = pcPins->GetOutput(uiGate);
	bNegated = IsNegated(meFunction);
	switch (meFunction)
	{
	case GF_And:
	case GF_Nand:
		if (uiLows > 0)
		{
			pcOutput->WriteBool(pcTimeline, bNegated);
		}
		else if (uiUndriven > 0)
		{
			pcOutput->WriteImpedance(pcTimeline);
		}
		else if (uiHighs > 0)
		{
			pcOutput->WriteBool(pcTimeline, !bNegated);
		}
		break;

	case GF_Or:
	case GF_Nor:
	case GF_Buffer:
	case GF_Inverter:
		if (uiHighs > 0)
		{
			pcOutput->WriteBool(pcTimeline, !bNegated);
		}
		else if (uiUndriven > 0)
		{
			pcOutput->WriteImpedance(pcTimeline);
		}
		else if (uiLows > 0)
		{
			pcOutput->WriteBool(pcTimeline, bNegated);
		}
		break;

	case GF_Xor:
	case GF_Xnor:
		if (uiUndriven > 0)
		{
			pcOutput->WriteImpedance(pcTimeline);
		}
		else if ((uiHighs > 0) || (uiLows > 0))
		{
			pcOutput->WriteBool(pcTimeline, ((uiHighs % 2) == 1) != bNegated);
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGateArray::IsNegated(EGateFunction eFunction)
{
	return (eFunction == GF_Nand) || (eFunction == GF_Nor) || (eFunction == GF_Xnor) || (eFunction == GF_Inverter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGateArray::IsSingleInput(EGateFunction eFunction)
{
	return (eFunction == GF_Buffer) || (eFunction == GF_Inverter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CGateArray::GetType(void) { return GATE_ARRAY_TYPE; }
CGateArrayPins* CGateArray::GetGateArrayPins(void) { return (CGateArrayPins*)&mpPins; }
EGateFunction CGateArray::GetFunction(void) { return meFunction; }

