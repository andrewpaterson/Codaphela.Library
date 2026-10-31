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
#include "BusBuffer.h"


//The pins of a bus buffer, in order; its enables come after its outputs.
#define BUS_BUFFER_INPUT		0
#define BUS_BUFFER_OUTPUT		1
#define BUS_BUFFER_ENABLE		2


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bInverting)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	mbInverting = bInverting;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::Class(void)
{
	CIntegratedCircuit::Class();

	U_Bool(mbInverting);
}


//////////////////////////////////////////////////////////////////////////
//
// "A" in, "Y" out and "OE" (or "OE1" and "OE2"), into asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CBusBuffer::GetRoles(SGroupPinRole* asRoles, size uiBits, size uiEnables)
{
	const char*		aszEnables[BUS_BUFFER_MAX_ENABLES] = { "OE1", "OE2" };
	size			i;

	if (uiEnables > BUS_BUFFER_MAX_ENABLES)
	{
		uiEnables = BUS_BUFFER_MAX_ENABLES;
	}
	asRoles[BUS_BUFFER_INPUT].szName = "A";
	asRoles[BUS_BUFFER_INPUT].uiBits = uiBits;
	asRoles[BUS_BUFFER_INPUT].eType = PT_Input;
	asRoles[BUS_BUFFER_INPUT].bShared = false;
	asRoles[BUS_BUFFER_OUTPUT].szName = "Y";
	asRoles[BUS_BUFFER_OUTPUT].uiBits = uiBits;
	asRoles[BUS_BUFFER_OUTPUT].eType = PT_Output;
	asRoles[BUS_BUFFER_OUTPUT].bShared = false;
	for (i = 0; i < uiEnables; i++)
	{
		asRoles[BUS_BUFFER_ENABLE + i].szName = (uiEnables == 1) ? "OE" : aszEnables[i];
		asRoles[BUS_BUFFER_ENABLE + i].uiBits = 1;
		asRoles[BUS_BUFFER_ENABLE + i].eType = PT_Input;
		asRoles[BUS_BUFFER_ENABLE + i].bShared = false;
	}
	return BUS_BUFFER_ENABLE + uiEnables;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The outputs are driven from the start, so a disabled group's don't
// float until an input changes.
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::WriteOutputs(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		WriteGroupOutputs(pcTimeline, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	size	uiRole;
	size	uiGroup;
	size	uiBit;

	if (IsPowered(pcTimeline->GetTime()) && GetGroupPins()->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		if (uiRole != BUS_BUFFER_OUTPUT)
		{
			WriteGroupOutputs(pcTimeline, uiGroup);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every one of the group's enables is low.
//
//////////////////////////////////////////////////////////////////////////
bool CBusBuffer::IsEnabled(CTimeline* pcTimeline, size uiGroup)
{
	CGroupPins*		pcPins;
	size			i;

	pcPins = GetGroupPins();
	for (i = BUS_BUFFER_ENABLE; pcPins->GetPin(i, uiGroup) != NULL; i++)
	{
		if (!pcPins->IsLow(pcTimeline, i, uiGroup))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusBuffer::WriteGroupOutputs(CTimeline* pcTimeline, size uiGroup)
{
	CGroupPins*		pcPins;
	size			i;
	STraceValue		sValue;
	CLogicPort*		pcOutput;

	pcPins = GetGroupPins();
	if (!IsEnabled(pcTimeline, uiGroup))
	{
		pcPins->WriteImpedance(pcTimeline, BUS_BUFFER_OUTPUT, uiGroup);
		return;
	}
	for (i = 0; i < pcPins->GetBitCount(BUS_BUFFER_INPUT); i++)
	{
		sValue = pcPins->GetPin(BUS_BUFFER_INPUT, uiGroup, i)->Read(pcTimeline);
		pcOutput = pcPins->GetPin(BUS_BUFFER_OUTPUT, uiGroup, i);
		if (sValue.IsHigh())
		{
			pcOutput->WriteBool(pcTimeline, !mbInverting);
		}
		else if (sValue.IsLow())
		{
			pcOutput->WriteBool(pcTimeline, mbInverting);
		}
		else if (sValue.eValue == TV_Undriven)
		{
			pcOutput->WriteImpedance(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CBusBuffer::GetType(void) { return BUS_BUFFER_TYPE; }
CGroupPins* CBusBuffer::GetGroupPins(void) { return (CGroupPins*)&mpPins; }
bool CBusBuffer::IsInverting(void) { return mbInverting; }

