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
#include "BusTransceiver.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CBusTransceiver::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a transceiver's pins (see EBusTransceiverRole), into
// asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CBusTransceiver::GetRoles(SGroupPinRole* asRoles, size uiBits)
{
	asRoles[BTR_A].szName = "A";
	asRoles[BTR_A].uiBits = uiBits;
	asRoles[BTR_A].eType = PT_Bidirectional;
	asRoles[BTR_A].bShared = false;
	asRoles[BTR_B].szName = "B";
	asRoles[BTR_B].uiBits = uiBits;
	asRoles[BTR_B].eType = PT_Bidirectional;
	asRoles[BTR_B].bShared = false;
	asRoles[BTR_Direction].szName = "DIR";
	asRoles[BTR_Direction].uiBits = 1;
	asRoles[BTR_Direction].eType = PT_Input;
	asRoles[BTR_Direction].bShared = false;
	asRoles[BTR_OutputEnable].szName = "OE";
	asRoles[BTR_OutputEnable].uiBits = 1;
	asRoles[BTR_OutputEnable].eType = PT_Input;
	asRoles[BTR_OutputEnable].bShared = false;
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusTransceiver::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The side being driven follows the other; the other doesn't drive.
//
//////////////////////////////////////////////////////////////////////////
void CBusTransceiver::WriteOutputs(CTimeline* pcTimeline)
{
	CGroupPins*		pcPins;
	size			i;
	bool			bEnabled;
	bool			bAToB;

	pcPins = GetGroupPins();
	bEnabled = pcPins->IsLow(pcTimeline, BTR_OutputEnable, 0);
	bAToB = pcPins->IsHigh(pcTimeline, BTR_Direction, 0);
	if (!bEnabled)
	{
		pcPins->WriteImpedance(pcTimeline, BTR_A, 0);
		pcPins->WriteImpedance(pcTimeline, BTR_B, 0);
	}
	else if (bAToB)
	{
		pcPins->WriteImpedance(pcTimeline, BTR_A, 0);
		for (i = 0; i < pcPins->GetBitCount(BTR_A); i++)
		{
			Drive(pcTimeline, BTR_A, BTR_B, i);
		}
	}
	else
	{
		pcPins->WriteImpedance(pcTimeline, BTR_B, 0);
		for (i = 0; i < pcPins->GetBitCount(BTR_B); i++)
		{
			Drive(pcTimeline, BTR_B, BTR_A, i);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A change of enable or direction turns everything round; a change of a
// bit on the side being read drives its bit on the other.
//
//////////////////////////////////////////////////////////////////////////
void CBusTransceiver::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CGroupPins*		pcPins;
	size			uiRole;
	size			uiGroup;
	size			uiBit;
	bool			bAToB;

	pcPins = GetGroupPins();
	if (!IsPowered(pcTimeline->GetTime()) || !pcPins->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		return;
	}

	if ((uiRole == BTR_Direction) || (uiRole == BTR_OutputEnable))
	{
		WriteOutputs(pcTimeline);
	}
	else if (pcPins->IsLow(pcTimeline, BTR_OutputEnable, 0))
	{
		bAToB = pcPins->IsHigh(pcTimeline, BTR_Direction, 0);
		if ((uiRole == BTR_A) && bAToB)
		{
			Drive(pcTimeline, BTR_A, BTR_B, uiBit);
		}
		else if ((uiRole == BTR_B) && !bAToB)
		{
			Drive(pcTimeline, BTR_B, BTR_A, uiBit);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBusTransceiver::Drive(CTimeline* pcTimeline, size uiFrom, size uiTo, size uiBit)
{
	CGroupPins*		pcPins;
	STraceValue		sValue;
	CLogicPort*		pcTo;

	pcPins = GetGroupPins();
	sValue = pcPins->GetPin(uiFrom, 0, uiBit)->Read(pcTimeline);
	pcTo = pcPins->GetPin(uiTo, 0, uiBit);
	if (sValue.IsHigh())
	{
		pcTo->WriteBool(pcTimeline, true);
	}
	else if (sValue.IsLow())
	{
		pcTo->WriteBool(pcTimeline, false);
	}
	else if (sValue.eValue == TV_Undriven)
	{
		pcTo->WriteImpedance(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CBusTransceiver::GetType(void) { return BUS_TRANSCEIVER_TYPE; }
CGroupPins* CBusTransceiver::GetGroupPins(void) { return (CGroupPins*)&mpPins; }

