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
#include "BaseLib/Logger.h"
#include "StateXMLFields.h"
#include "Counter.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CCounter::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CCounterPins> pPins, size uiBitWidth, uint32 uiTerminalValue)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	muiBitWidth = uiBitWidth;
	muiTerminalValue = uiTerminalValue;
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounter::Class(void)
{
	CIntegratedCircuit::Class();

	U_Size(muiBitWidth);
	U_UInt32(muiTerminalValue);
	U_UInt32(muiCount);
	U_Bool(mbTerminal);
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
// The count and whether it's at the terminal count.
//
//////////////////////////////////////////////////////////////////////////
void CCounter::WriteOutputs(CTimeline* pcTimeline)
{
	CCounterPins*	pcPins;

	pcPins = GetCounterPins();
	pcPins->WriteOutput(pcTimeline, muiCount, muiBitWidth);
	pcPins->GetTerminalOutput()->WriteBool(pcTimeline, mbTerminal);
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
	bOldTerminal = mbTerminal;
	bTerminal = (uiState == muiTerminalValue) && bCountEnableTerminal;
	mbTerminal = bTerminal;
	if ((uiOldState != uiState) || (bOldTerminal != bTerminal))
	{
		WriteOutputs(pcTimeline);
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
CCounterPins* CCounter::GetCounterPins(void) { return (CCounterPins*)&mpPins; }
uint32 CCounter::GetCount(void) { return muiCount; }
void CCounter::SetCount(uint32 uiCount) { muiCount = uiCount; }
bool CCounter::IsTerminal(void) { return mbTerminal; }
void CCounter::SetTerminal(bool bTerminal) { mbTerminal = bTerminal; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounter::GetStateXMLName(void) { return "CounterState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounter::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("count", (int64)GetCount());
	pcWriter->Attribute("terminal", IsTerminal());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCounter::LoadXML(CMarkupTag* pcTag)
{
	int64	iCount;
	bool	bTerminal;

	if (!pcTag->GetInt64Attribute("count", &iCount) || (iCount < 0) || (iCount > 0xffffffffLL) || !pcTag->GetBoolAttribute("terminal", &bTerminal))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a count from 0 to 4294967295 and a true or false terminal.", NULL);
	}
	SetCount((uint32)iCount);
	SetTerminal(bTerminal);
	return true;
}

