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
#include "FlipFlopArray.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::Class(void)
{
	CIntegratedCircuit::Class();

	U_Data(bool[FLIP_FLOP_ARRAY_MAX], mabStates);
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a flip flop array's pins (see EFlipFlopArrayRole), into
// asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CFlipFlopArray::GetRoles(SGroupPinRole* asRoles)
{
	const char*		aszNames[] = { "CP", "D", "SD", "RD", "Q", "QB" };
	size			i;

	for (i = 0; i < 6; i++)
	{
		asRoles[i].szName = aszNames[i];
		asRoles[i].uiBits = 1;
		asRoles[i].eType = (i >= FFAR_Q) ? PT_Output : PT_Input;
		asRoles[i].bShared = false;
	}
	return 6;
}


//////////////////////////////////////////////////////////////////////////
//
// Low.
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::ResetState(void)
{
	size	i;

	for (i = 0; i < FLIP_FLOP_ARRAY_MAX; i++)
	{
		mabStates[i] = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::WriteOutputs(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		WriteFlipFlop(pcTimeline, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Q is the state and QB its inverse.
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::WriteFlipFlop(CTimeline* pcTimeline, size uiFlipFlop)
{
	GetGroupPins()->GetPin(FFAR_Q, uiFlipFlop)->WriteBool(pcTimeline, mabStates[uiFlipFlop]);
	GetGroupPins()->GetPin(FFAR_QB, uiFlipFlop)->WriteBool(pcTimeline, !mabStates[uiFlipFlop]);
}


//////////////////////////////////////////////////////////////////////////
//
// As CDTypeFlipFlop: set and reset together set it.
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CGroupPins*		pcPins;
	size			uiRole;
	size			uiFlipFlop;
	size			uiBit;
	bool			bSetLow;
	bool			bResetLow;
	bool			bState;
	STraceValue		sData;

	pcPins = GetGroupPins();
	if (!pcPins->FindPin(pcPort, &uiRole, &uiFlipFlop, &uiBit) || (uiRole >= FFAR_Q))
	{
		return;
	}

	bState = mabStates[uiFlipFlop];
	bSetLow = pcPins->IsLow(pcTimeline, FFAR_Set, uiFlipFlop);
	bResetLow = pcPins->IsLow(pcTimeline, FFAR_Reset, uiFlipFlop);
	if (bSetLow)
	{
		bState = true;
	}
	else if (bResetLow)
	{
		bState = false;
	}
	else if ((uiRole == FFAR_Clock) && pcPort->Read(pcTimeline).IsHigh())
	{
		sData = pcPins->GetPin(FFAR_Data, uiFlipFlop)->Read(pcTimeline);
		if (sData.IsHigh())
		{
			bState = true;
		}
		else if (sData.IsLow())
		{
			bState = false;
		}
	}

	if (bState != mabStates[uiFlipFlop])
	{
		mabStates[uiFlipFlop] = bState;
		WriteFlipFlop(pcTimeline, uiFlipFlop);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CFlipFlopArray::GetType(void) { return FLIP_FLOP_ARRAY_TYPE; }
CGroupPins* CFlipFlopArray::GetGroupPins(void) { return (CGroupPins*)&mpPins; }
bool CFlipFlopArray::GetState(size uiFlipFlop) { return (uiFlipFlop < FLIP_FLOP_ARRAY_MAX) ? mabStates[uiFlipFlop] : false; }
void CFlipFlopArray::SetState(size uiFlipFlop, bool bState) { if (uiFlipFlop < FLIP_FLOP_ARRAY_MAX) { mabStates[uiFlipFlop] = bState; } }
const char* CFlipFlopArray::GetStateXMLName(void) { return "FlipFlopArrayState"; }


//////////////////////////////////////////////////////////////////////////
//
// Each flip flop's state, "state1" up.
//
//////////////////////////////////////////////////////////////////////////
void CFlipFlopArray::SaveXML(CXMLWriter* pcWriter)
{
	size	i;
	CChars	szName;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		szName.Init("state");
		szName.Append((int32)(i + 1));
		pcWriter->Attribute(szName.Text(), mabStates[i]);
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFlipFlopArray::LoadXML(CMarkupTag* pcTag)
{
	size	i;
	CChars	szName;
	bool	bLoaded;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		szName.Init("state");
		szName.Append((int32)(i + 1));
		bLoaded = LoadStateBool(pcTag, szName.Text(), &mabStates[i]);
		szName.Kill();
		if (!bLoaded)
		{
			return false;
		}
	}
	return true;
}

