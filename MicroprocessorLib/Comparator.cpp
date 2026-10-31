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
#include "Comparator.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CComparator::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a comparator's pins (see EComparatorRole), into asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CComparator::GetRoles(SGroupPinRole* asRoles, size uiBits)
{
	asRoles[CR_P].szName = "P";
	asRoles[CR_P].uiBits = uiBits;
	asRoles[CR_P].eType = PT_Input;
	asRoles[CR_P].bShared = false;
	asRoles[CR_Q].szName = "Q";
	asRoles[CR_Q].uiBits = uiBits;
	asRoles[CR_Q].eType = PT_Input;
	asRoles[CR_Q].bShared = false;
	asRoles[CR_Enable].szName = "OE";
	asRoles[CR_Enable].uiBits = 1;
	asRoles[CR_Enable].eType = PT_Input;
	asRoles[CR_Enable].bShared = false;
	asRoles[CR_Equal].szName = "EQ";
	asRoles[CR_Equal].uiBits = 1;
	asRoles[CR_Equal].eType = PT_Output;
	asRoles[CR_Equal].bShared = false;
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComparator::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComparator::WriteOutputs(CTimeline* pcTimeline)
{
	CGroupPins*		pcPins;
	CLogicPort*		pcEqual;
	uint32			uiP;
	uint32			uiQ;
	bool			bPDriven;
	bool			bQDriven;

	pcPins = GetGroupPins();
	pcEqual = pcPins->GetPin(CR_Equal, 0);
	if (!pcPins->IsLow(pcTimeline, CR_Enable, 0))
	{
		pcEqual->WriteBool(pcTimeline, true);
		return;
	}
	uiP = pcPins->ReadBits(pcTimeline, CR_P, 0, &bPDriven);
	uiQ = pcPins->ReadBits(pcTimeline, CR_Q, 0, &bQDriven);
	if (!bPDriven || !bQDriven)
	{
		pcEqual->WriteImpedance(pcTimeline);
		return;
	}
	pcEqual->WriteBool(pcTimeline, uiP != uiQ);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComparator::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	size	uiRole;
	size	uiGroup;
	size	uiBit;

	if (IsPowered(pcTimeline->GetTime()) && GetGroupPins()->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		if (uiRole != CR_Equal)
		{
			WriteOutputs(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CComparator::GetType(void) { return COMPARATOR_TYPE; }
CGroupPins* CComparator::GetGroupPins(void) { return (CGroupPins*)&mpPins; }

