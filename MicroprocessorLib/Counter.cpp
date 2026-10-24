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
	ResetState();
}

//////////////////////////////////////////////////////////////////////////
//
// Zero, not terminal.
//
//////////////////////////////////////////////////////////////////////////
void CCounter::ResetState(void)
{
	muiCount = 0;
	mbTerminal = false;
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
	bool			bResetLow;
	bool			bLoadLow;
	bool			bCountEnable;
	bool			bCountEnableTerminal;
	uint32			uiOldState;
	uint32			uiState;
	bool			bOldTerminal;
	bool			bTerminal;

	pcPins = GetCounterPins();
	bResetLow = pcPins->GetReset()->Read(pcTimeline).IsLow();
	bLoadLow = pcPins->GetLoad()->Read(pcTimeline).IsLow();
	bCountEnable = pcPins->GetCountEnable()->Read(pcTimeline).IsHigh();
	bCountEnableTerminal = pcPins->GetCountEnableTerminal()->Read(pcTimeline).IsHigh();
	uiOldState = muiCount;
	if (bResetLow)
	{
		muiCount = 0;
	}
	else if (bLoadLow)
	{
		muiCount = pcPins->ReadData(pcTimeline, muiBitWidth);
	}
	else if (pcPins->IsClock(pcPort))
	{
		if (bCountEnable && bCountEnableTerminal)
		{
			if (pcPort->Read(pcTimeline).IsHigh())
			{
				if (muiCount >= muiTerminalValue)
				{
					muiCount = 0;
				}
				else
				{
					muiCount++;
				}
			}
		}
	}

	uiState = muiCount;
	if (uiOldState != uiState)
	{
		pcPins->WriteOutput(pcTimeline, uiState, muiBitWidth);
	}

	bOldTerminal = mbTerminal;
	bTerminal = (uiState == muiTerminalValue) && bCountEnableTerminal;
	mbTerminal = bTerminal;
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

	uiByte = (uint8)muiCount;
	psz->Append("0x");
	psz->AppendHexHiLo(&uiByte, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounter::GetType(void) { return COUNTER_TYPE; }
CCounterPins* CCounter::GetCounterPins(void) { return (CCounterPins*)mpcPins; }
uint32 CCounter::GetCount(void) { return muiCount; }
void CCounter::SetCount(uint32 uiCount) { muiCount = uiCount; }
bool CCounter::IsTerminal(void) { return mbTerminal; }
void CCounter::SetTerminal(bool bTerminal) { mbTerminal = bTerminal; }

