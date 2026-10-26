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
	ResetState();
}

//////////////////////////////////////////////////////////////////////////
//
// Low.
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::ResetState(void)
{
	mbState = false;
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
// Q is the state and QB its inverse.
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::WriteOutputs(CTimeline* pcTimeline)
{
	CDTypeFlipFlopPins*		pcPins;

	pcPins = GetDTypeFlipFlopPins();
	pcPins->GetOutputQ()->WriteBool(pcTimeline, mbState);
	pcPins->GetOutputQB()->WriteBool(pcTimeline, !mbState);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlop::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CDTypeFlipFlopPins*		pcPins;
	bool					bStateChanged;
	bool					bSetLow;
	bool					bResetLow;
	STraceValue				sDataValue;

	pcPins = GetDTypeFlipFlopPins();
	bStateChanged = false;
	bSetLow = pcPins->GetSet()->Read(pcTimeline).IsLow();
	bResetLow = pcPins->GetReset()->Read(pcTimeline).IsLow();
	if (bSetLow && bResetLow)
	{
		//Most likely for LVC.  Could be metastable also.
		bStateChanged = ChangeState(true);
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
					bStateChanged = ChangeState(true);
				}
				else if (sDataValue.IsLow())
				{
					bStateChanged = ChangeState(false);
				}
			}
		}
	}
	else if (bSetLow)
	{
		bStateChanged = ChangeState(true);
	}
	else
	{
		bStateChanged = ChangeState(false);
	}

	if (bStateChanged)
	{
		WriteOutputs(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the state changed.
//
//////////////////////////////////////////////////////////////////////////
bool CDTypeFlipFlop::ChangeState(bool bState)
{
	if (bState != mbState)
	{
		mbState = bState;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlop::GetType(void) { return D_TYPE_FLIP_FLOP_TYPE; }
CDTypeFlipFlopPins* CDTypeFlipFlop::GetDTypeFlipFlopPins(void) { return (CDTypeFlipFlopPins*)mpcPins; }
bool CDTypeFlipFlop::GetState(void) { return mbState; }
void CDTypeFlipFlop::SetState(bool bState) { mbState = bState; }

