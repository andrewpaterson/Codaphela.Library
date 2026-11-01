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
#include "OrGate.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void COrGate::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<COrGatePins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrGate::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrGate::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CArrayLogicPortPtr*	papcInputs;
	size				i;
	size				uiHighs;
	size				uiLows;
	size				uiUndriven;
	STraceValue			sValue;

	papcInputs = GetOrGatePins()->GetInputs();
	uiHighs = 0;
	uiLows = 0;
	uiUndriven = 0;
	for (i = 0; i < papcInputs->NumElements(); i++)
	{
		sValue = papcInputs->GetPtr(i)->Read(pcTimeline);
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

	if (uiHighs > 0)
	{
		GetOrGatePins()->GetOutput()->WriteBool(pcTimeline, TransformOutput(true));
	}
	else if (uiUndriven > 0)
	{
		GetOrGatePins()->GetOutput()->WriteImpedance(pcTimeline);
	}
	else if (uiLows > 0)
	{
		GetOrGatePins()->GetOutput()->WriteBool(pcTimeline, TransformOutput(false));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrGate::TransformOutput(bool bValue) { return bValue; }
const char* COrGate::GetType(void) { return OR_GATE_TYPE; }
COrGatePins* COrGate::GetOrGatePins(void) { return (COrGatePins*)&mpPins; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNorGate::TransformOutput(bool bValue) { return !bValue; }
const char* CNorGate::GetType(void) { return NOR_GATE_TYPE; }

