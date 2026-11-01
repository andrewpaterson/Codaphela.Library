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
#include "Adder.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CAdder::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of an adder's pins (see EAdderRole), into asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CAdder::GetRoles(SGroupPinRole* asRoles, size uiBits)
{
	if (uiBits > ADDER_MAX_BITS)
	{
		uiBits = ADDER_MAX_BITS;
	}
	asRoles[AR_A].szName = "A";
	asRoles[AR_A].uiBits = uiBits;
	asRoles[AR_A].eType = PT_Input;
	asRoles[AR_A].bShared = false;
	asRoles[AR_B].szName = "B";
	asRoles[AR_B].uiBits = uiBits;
	asRoles[AR_B].eType = PT_Input;
	asRoles[AR_B].bShared = false;
	asRoles[AR_CarryIn].szName = "CI";
	asRoles[AR_CarryIn].uiBits = 1;
	asRoles[AR_CarryIn].eType = PT_Input;
	asRoles[AR_CarryIn].bShared = false;
	asRoles[AR_Sum].szName = "S";
	asRoles[AR_Sum].uiBits = uiBits;
	asRoles[AR_Sum].eType = PT_Output;
	asRoles[AR_Sum].bShared = false;
	asRoles[AR_CarryOut].szName = "CO";
	asRoles[AR_CarryOut].uiBits = 1;
	asRoles[AR_CarryOut].eType = PT_Output;
	asRoles[AR_CarryOut].bShared = false;
	return 5;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAdder::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAdder::WriteOutputs(CTimeline* pcTimeline)
{
	CGroupPins*		pcPins;
	uint32			uiA;
	uint32			uiB;
	uint32			uiCarry;
	uint32			uiSum;
	bool			bADriven;
	bool			bBDriven;
	bool			bCarryDriven;
	size			uiBits;

	pcPins = GetGroupPins();
	uiA = pcPins->ReadBits(pcTimeline, AR_A, 0, &bADriven);
	uiB = pcPins->ReadBits(pcTimeline, AR_B, 0, &bBDriven);
	uiCarry = pcPins->ReadBits(pcTimeline, AR_CarryIn, 0, &bCarryDriven);
	if (!bADriven || !bBDriven || !bCarryDriven)
	{
		pcPins->WriteImpedance(pcTimeline, AR_Sum, 0);
		pcPins->WriteImpedance(pcTimeline, AR_CarryOut, 0);
		return;
	}
	uiBits = pcPins->GetBitCount(AR_A);
	uiSum = uiA + uiB + uiCarry;
	pcPins->WriteBits(pcTimeline, AR_Sum, 0, uiSum);
	pcPins->WriteBits(pcTimeline, AR_CarryOut, 0, (uiSum >> uiBits) & 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAdder::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	size	uiRole;
	size	uiGroup;
	size	uiBit;

	if (IsPowered(pcTimeline->GetTime()) && GetGroupPins()->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		if ((uiRole != AR_Sum) && (uiRole != AR_CarryOut))
		{
			WriteOutputs(pcTimeline);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CAdder::GetType(void) { return ADDER_TYPE; }
CGroupPins* CAdder::GetGroupPins(void) { return (CGroupPins*)&mpPins; }

