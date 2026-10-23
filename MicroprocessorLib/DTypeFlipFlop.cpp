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
#include "DTypeFlipFlop.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins.
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CDTypeFlipFlopPins* pcPins)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CDTypeFlipFlopPins*		pcPins;
	CDTypeFlipFlopState*	pcState;
	bool					bStateChanged;
	bool					bSetLow;
	bool					bResetLow;
	STraceValue				sDataValue;

	pcPins = GetDTypeFlipFlopPins();
	pcState = GetDTypeFlipFlopState();
	bStateChanged = false;
	bSetLow = pcPins->GetSet()->Read(pcTimeline).IsLow();
	bResetLow = pcPins->GetReset()->Read(pcTimeline).IsLow();
	if (bSetLow && bResetLow)
	{
		//Most likely for LVC.  Could be metastable also.
		bStateChanged = pcState->SetState(true);
	}
	else if (!bSetLow && !bResetLow)
	{
		if (pcPins->IsClock(pcPort))
		{
			if (pcPort->Read(pcTimeline).IsHigh())
			{
				sDataValue = pcPins->GetData()->Read(pcTimeline);
				if (sDataValue.IsHigh())
				{
					bStateChanged = pcState->SetState(true);
				}
				else if (sDataValue.IsLow())
				{
					bStateChanged = pcState->SetState(false);
				}
			}
		}
	}
	else if (bSetLow)
	{
		bStateChanged = pcState->SetState(true);
	}
	else
	{
		bStateChanged = pcState->SetState(false);
	}

	if (bStateChanged)
	{
		pcPins->GetOutputQ()->WriteBool(pcTimeline, pcState->GetState());
		pcPins->GetOutputQB()->WriteBool(pcTimeline, !pcState->GetState());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Starting low.
//
//////////////////////////////////////////////////////////////////////////
CState* CDTypeFlipFlop::CreateState(void)
{
	CDTypeFlipFlopState*	pcState;

	pcState = NewMalloc<CDTypeFlipFlopState>();
	pcState->Init(false);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlop::GetType(void) { return D_TYPE_FLIP_FLOP_TYPE; }
CDTypeFlipFlopPins* CDTypeFlipFlop::GetDTypeFlipFlopPins(void) { return (CDTypeFlipFlopPins*)mpcPins; }
CDTypeFlipFlopState* CDTypeFlipFlop::GetDTypeFlipFlopState(void) { return (CDTypeFlipFlopState*)mpcState; }

