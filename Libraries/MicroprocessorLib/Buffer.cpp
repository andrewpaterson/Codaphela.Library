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
#include "Buffer.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CBuffer::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CBufferPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBuffer::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBuffer::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CArrayLogicPortPtr*	papcInputs;
	CArrayLogicPortPtr*	papcOutputs;
	size				i;
	STraceValue			sValue;

	if (IsPowered(pcTimeline->GetTime()))
	{
		papcInputs = GetBufferPins()->GetInputs();
		papcOutputs = GetBufferPins()->GetOutputs();
		for (i = 0; i < papcInputs->NumElements(); i++)
		{
			sValue = papcInputs->GetPtr(i)->Read(pcTimeline);
			if (sValue.IsHigh())
			{
				papcOutputs->GetPtr(i)->WriteBool(pcTimeline, TransformOutput(true));
			}
			else if (sValue.IsLow())
			{
				papcOutputs->GetPtr(i)->WriteBool(pcTimeline, TransformOutput(false));
			}
			else if (sValue.eValue == TV_Undriven)
			{
				papcOutputs->GetPtr(i)->WriteImpedance(pcTimeline);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBuffer::TransformOutput(bool bValue) { return bValue; }
const char* CBuffer::GetType(void) { return BUFFER_TYPE; }
CBufferPins* CBuffer::GetBufferPins(void) { return (CBufferPins*)&mpPins; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverter::TransformOutput(bool bValue) { return !bValue; }
const char* CInverter::GetType(void) { return INVERTER_TYPE; }

