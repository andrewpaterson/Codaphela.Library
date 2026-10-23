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
#include "Counter.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins.
//
//////////////////////////////////////////////////////////////////////////
void CCounter::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CCounterPins* pcPins, size uiBitWidth, uint32 uiTerminalValue)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);
	muiBitWidth = uiBitWidth;
	muiTerminalValue = uiTerminalValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounter::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounter::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CCounterPins*	pcPins;
	CCounterState*	pcState;
	bool			bResetLow;
	bool			bLoadLow;
	bool			bCountEnable;
	bool			bCountEnableTerminal;
	uint32			uiOldState;
	uint32			uiState;
	bool			bOldTerminal;
	bool			bTerminal;

	pcPins = GetCounterPins();
	pcState = GetCounterState();
	bResetLow = pcPins->GetReset()->Read(pcTimeline).IsLow();
	bLoadLow = pcPins->GetLoad()->Read(pcTimeline).IsLow();
	bCountEnable = pcPins->GetCountEnable()->Read(pcTimeline).IsHigh();
	bCountEnableTerminal = pcPins->GetCountEnableTerminal()->Read(pcTimeline).IsHigh();
	uiOldState = pcState->GetState();
	if (bResetLow)
	{
		pcState->SetState(0);
	}
	else if (bLoadLow)
	{
		pcState->SetState(pcPins->ReadData(pcTimeline, muiBitWidth));
	}
	else if (pcPins->IsClock(pcPort))
	{
		if (bCountEnable && bCountEnableTerminal)
		{
			if (pcPort->Read(pcTimeline).IsHigh())
			{
				pcState->Count(muiTerminalValue);
			}
		}
	}

	uiState = pcState->GetState();
	if (uiOldState != uiState)
	{
		pcPins->WriteOutput(pcTimeline, uiState, muiBitWidth);
	}

	bOldTerminal = pcState->GetTerminal();
	bTerminal = (uiState == muiTerminalValue) && bCountEnableTerminal;
	pcState->SetTerminal(bTerminal);
	if (bOldTerminal != bTerminal)
	{
		pcPins->GetTerminalOutput()->WriteBool(pcTimeline, bTerminal);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like "0x0A", the low 8 bits, as Logicim's getByteStringHex.
//
//////////////////////////////////////////////////////////////////////////
void CCounter::AppendCounterValueHex(CChars* psz)
{
	uint8	uiByte;

	uiByte = (uint8)GetCounterState()->GetState();
	psz->Append("0x");
	psz->AppendHexHiLo(&uiByte, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Starting at zero.
//
//////////////////////////////////////////////////////////////////////////
CState* CCounter::CreateState(void)
{
	CCounterState*	pcState;

	pcState = NewMalloc<CCounterState>();
	pcState->Init(0);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounter::GetType(void) { return COUNTER_TYPE; }
CCounterPins* CCounter::GetCounterPins(void) { return (CCounterPins*)mpcPins; }
CCounterState* CCounter::GetCounterState(void) { return (CCounterState*)mpcState; }

