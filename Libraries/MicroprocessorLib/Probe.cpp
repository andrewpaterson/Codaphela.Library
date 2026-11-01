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
#include "Probe.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CProbe::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CProbePins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProbe::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProbe::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The inputs as a number, "Input 0" the least significant bit.  A bit is
// 1 only when its input reads high.  Logicim's view reads the ports
// itself.
//
//////////////////////////////////////////////////////////////////////////
uint64 CProbe::ReadValue(CTimeline* pcTimeline)
{
	CArrayLogicPortPtr*	papcInputs;
	size				i;
	uint64				uiValue;

	papcInputs = GetProbePins()->GetInputs();
	uiValue = 0;
	for (i = 0; i < papcInputs->NumElements(); i++)
	{
		if (papcInputs->GetPtr(i)->Read(pcTimeline).IsHigh())
		{
			uiValue |= (uint64)1 << i;
		}
	}
	return uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CProbe::GetType(void) { return PROBE_TYPE; }
CProbePins* CProbe::GetProbePins(void) { return (CProbePins*)&mpPins; }

