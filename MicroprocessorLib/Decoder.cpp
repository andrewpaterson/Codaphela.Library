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
#include "Decoder.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.  The first uiLowEnables enables are active low.
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, size uiLowEnables)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	muiLowEnables = uiLowEnables;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::Class(void)
{
	CIntegratedCircuit::Class();

	U_Size(muiLowEnables);
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a decoder's pins (see EDecoderRole), into asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CDecoder::GetRoles(SGroupPinRole* asRoles, size uiAddressBits, size uiEnables)
{
	const char*		aszEnables[DECODER_MAX_ENABLES] = { "E1", "E2", "E3" };
	size			i;

	if (uiAddressBits < 1)
	{
		uiAddressBits = 1;
	}
	else if (uiAddressBits > DECODER_MAX_ADDRESS_BITS)
	{
		uiAddressBits = DECODER_MAX_ADDRESS_BITS;
	}
	if (uiEnables > DECODER_MAX_ENABLES)
	{
		uiEnables = DECODER_MAX_ENABLES;
	}
	asRoles[DR_Address].szName = "A";
	asRoles[DR_Address].uiBits = uiAddressBits;
	asRoles[DR_Address].eType = PT_Input;
	asRoles[DR_Address].bShared = false;
	asRoles[DR_Output].szName = "Y";
	asRoles[DR_Output].uiBits = (size)1 << uiAddressBits;
	asRoles[DR_Output].eType = PT_Output;
	asRoles[DR_Output].bShared = false;
	for (i = 0; i < uiEnables; i++)
	{
		asRoles[DR_Enable + i].szName = (uiEnables == 1) ? "E" : aszEnables[i];
		asRoles[DR_Enable + i].uiBits = 1;
		asRoles[DR_Enable + i].eType = PT_Input;
		asRoles[DR_Enable + i].bShared = false;
	}
	return DR_Enable + uiEnables;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::WriteOutputs(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		WriteGroup(pcTimeline, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	size	uiRole;
	size	uiGroup;
	size	uiBit;

	if (IsPowered(pcTimeline->GetTime()) && GetGroupPins()->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		if (uiRole != DR_Output)
		{
			WriteGroup(pcTimeline, uiGroup);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDecoder::WriteGroup(CTimeline* pcTimeline, size uiGroup)
{
	CGroupPins*		pcPins;
	size			i;
	bool			bEnabled;
	uint32			uiAddress;
	bool			bAllDriven;

	pcPins = GetGroupPins();
	bEnabled = true;
	for (i = 0; pcPins->GetPin(DR_Enable + i, uiGroup) != NULL; i++)
	{
		if (i < muiLowEnables)
		{
			bEnabled &= pcPins->IsLow(pcTimeline, DR_Enable + i, uiGroup);
		}
		else
		{
			bEnabled &= pcPins->IsHigh(pcTimeline, DR_Enable + i, uiGroup);
		}
	}

	if (!bEnabled)
	{
		pcPins->WriteBits(pcTimeline, DR_Output, uiGroup, 0xFFFFFFFF);
		return;
	}
	uiAddress = pcPins->ReadBits(pcTimeline, DR_Address, uiGroup, &bAllDriven);
	if (!bAllDriven)
	{
		pcPins->WriteImpedance(pcTimeline, DR_Output, uiGroup);
		return;
	}
	pcPins->WriteBits(pcTimeline, DR_Output, uiGroup, ~((uint32)1 << uiAddress));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDecoder::GetType(void) { return DECODER_TYPE; }
CGroupPins* CDecoder::GetGroupPins(void) { return (CGroupPins*)&mpPins; }

