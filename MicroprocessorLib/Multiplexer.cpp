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
#include "Multiplexer.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bDisabledHighImpedance)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	mbDisabledHighImpedance = bDisabledHighImpedance;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::Class(void)
{
	CIntegratedCircuit::Class();

	U_Bool(mbDisabledHighImpedance);
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a multiplexer's pins (see EMultiplexerRole), into asRoles:
// two inputs a channel for each select bit.
//
//////////////////////////////////////////////////////////////////////////
size CMultiplexer::GetRoles(SGroupPinRole* asRoles, size uiSelectBits, bool bChannelEnables)
{
	if (uiSelectBits < 1)
	{
		uiSelectBits = 1;
	}
	else if (uiSelectBits > MULTIPLEXER_MAX_SELECT_BITS)
	{
		uiSelectBits = MULTIPLEXER_MAX_SELECT_BITS;
	}
	asRoles[MR_Input].szName = "I";
	asRoles[MR_Input].uiBits = (size)1 << uiSelectBits;
	asRoles[MR_Input].eType = PT_Input;
	asRoles[MR_Input].bShared = false;
	asRoles[MR_Output].szName = "Y";
	asRoles[MR_Output].uiBits = 1;
	asRoles[MR_Output].eType = PT_Output;
	asRoles[MR_Output].bShared = false;
	asRoles[MR_Select].szName = "S";
	asRoles[MR_Select].uiBits = uiSelectBits;
	asRoles[MR_Select].eType = PT_Input;
	asRoles[MR_Select].bShared = true;
	asRoles[MR_Enable].szName = "E";
	asRoles[MR_Enable].uiBits = 1;
	asRoles[MR_Enable].eType = PT_Input;
	asRoles[MR_Enable].bShared = !bChannelEnables;
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::WriteOutputs(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		WriteChannel(pcTimeline, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A shared select or enable changes every channel; anything else only
// its own.
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CGroupPins*		pcPins;
	size			uiRole;
	size			uiGroup;
	size			uiBit;

	pcPins = GetGroupPins();
	if (!IsPowered(pcTimeline->GetTime()) || !pcPins->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		return;
	}
	if (uiRole == MR_Output)
	{
		return;
	}
	if ((uiRole == MR_Select) || ((uiRole == MR_Enable) && (pcPins->GetPin(MR_Enable, 0) == pcPins->GetPin(MR_Enable, pcPins->GetGroupCount() - 1))))
	{
		WriteOutputs(pcTimeline);
	}
	else
	{
		WriteChannel(pcTimeline, uiGroup);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiplexer::WriteChannel(CTimeline* pcTimeline, size uiChannel)
{
	CGroupPins*		pcPins;
	CLogicPort*		pcOutput;
	uint32			uiSelect;
	bool			bAllDriven;
	STraceValue		sValue;

	pcPins = GetGroupPins();
	pcOutput = pcPins->GetPin(MR_Output, uiChannel);
	if (!pcPins->IsLow(pcTimeline, MR_Enable, uiChannel))
	{
		if (mbDisabledHighImpedance)
		{
			pcOutput->WriteImpedance(pcTimeline);
		}
		else
		{
			pcOutput->WriteBool(pcTimeline, false);
		}
		return;
	}

	uiSelect = pcPins->ReadBits(pcTimeline, MR_Select, 0, &bAllDriven);
	if (!bAllDriven)
	{
		pcOutput->WriteImpedance(pcTimeline);
		return;
	}
	sValue = pcPins->GetPin(MR_Input, uiChannel, uiSelect)->Read(pcTimeline);
	if (sValue.IsHigh())
	{
		pcOutput->WriteBool(pcTimeline, true);
	}
	else if (sValue.IsLow())
	{
		pcOutput->WriteBool(pcTimeline, false);
	}
	else if (sValue.eValue == TV_Undriven)
	{
		pcOutput->WriteImpedance(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CMultiplexer::GetType(void) { return MULTIPLEXER_TYPE; }
CGroupPins* CMultiplexer::GetGroupPins(void) { return (CGroupPins*)&mpPins; }
bool CMultiplexer::IsDisabledHighImpedance(void) { return mbDisabledHighImpedance; }

