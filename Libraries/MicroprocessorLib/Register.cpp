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
#include "Register.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CRegister::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bTransparent)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	mbTransparent = bTransparent;
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegister::Class(void)
{
	CIntegratedCircuit::Class();

	U_Bool(mbTransparent);
	U_Data(uint32[REGISTER_MAX_GROUPS], mauiValues);
}


//////////////////////////////////////////////////////////////////////////
//
// The roles of a register's pins (see ERegisterRole), into asRoles.
//
//////////////////////////////////////////////////////////////////////////
size CRegister::GetRoles(SGroupPinRole* asRoles, size uiBits, bool bTransparent)
{
	asRoles[RR_Data].szName = "D";
	asRoles[RR_Data].uiBits = uiBits;
	asRoles[RR_Data].eType = PT_Input;
	asRoles[RR_Data].bShared = false;
	asRoles[RR_Clock].szName = bTransparent ? "LE" : "CP";
	asRoles[RR_Clock].uiBits = 1;
	asRoles[RR_Clock].eType = PT_Input;
	asRoles[RR_Clock].bShared = false;
	asRoles[RR_OutputEnable].szName = "OE";
	asRoles[RR_OutputEnable].uiBits = 1;
	asRoles[RR_OutputEnable].eType = PT_Input;
	asRoles[RR_OutputEnable].bShared = false;
	asRoles[RR_Output].szName = "Q";
	asRoles[RR_Output].uiBits = uiBits;
	asRoles[RR_Output].eType = PT_Output;
	asRoles[RR_Output].bShared = false;
	return 4;
}


//////////////////////////////////////////////////////////////////////////
//
// Zero.
//
//////////////////////////////////////////////////////////////////////////
void CRegister::ResetState(void)
{
	size	i;

	for (i = 0; i < REGISTER_MAX_GROUPS; i++)
	{
		mauiValues[i] = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegister::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegister::WriteOutputs(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		WriteGroupOutputs(pcTimeline, i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What's held while the output enable is low, otherwise nothing.
//
//////////////////////////////////////////////////////////////////////////
void CRegister::WriteGroupOutputs(CTimeline* pcTimeline, size uiGroup)
{
	CGroupPins*		pcPins;

	pcPins = GetGroupPins();
	if (pcPins->IsLow(pcTimeline, RR_OutputEnable, uiGroup))
	{
		pcPins->WriteBits(pcTimeline, RR_Output, uiGroup, mauiValues[uiGroup]);
	}
	else
	{
		pcPins->WriteImpedance(pcTimeline, RR_Output, uiGroup);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRegister::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	CGroupPins*		pcPins;
	size			uiRole;
	size			uiGroup;
	size			uiBit;

	pcPins = GetGroupPins();
	if (!pcPins->FindPin(pcPort, &uiRole, &uiGroup, &uiBit))
	{
		return;
	}

	if (uiRole == RR_Clock)
	{
		if (pcPort->Read(pcTimeline).IsHigh())
		{
			TakeData(pcTimeline, uiGroup);
		}
	}
	else if (uiRole == RR_Data)
	{
		if (mbTransparent && pcPins->IsHigh(pcTimeline, RR_Clock, uiGroup))
		{
			TakeData(pcTimeline, uiGroup);
		}
	}
	else if (uiRole == RR_OutputEnable)
	{
		WriteGroupOutputs(pcTimeline, uiGroup);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each data bit that's high or low, then the outputs if what's held
// changed.
//
//////////////////////////////////////////////////////////////////////////
void CRegister::TakeData(CTimeline* pcTimeline, size uiGroup)
{
	CGroupPins*		pcPins;
	size			i;
	uint32			uiValue;
	STraceValue		sValue;

	pcPins = GetGroupPins();
	uiValue = mauiValues[uiGroup];
	for (i = 0; i < pcPins->GetBitCount(RR_Data); i++)
	{
		sValue = pcPins->GetPin(RR_Data, uiGroup, i)->Read(pcTimeline);
		if (sValue.IsHigh())
		{
			uiValue |= 1 << i;
		}
		else if (sValue.IsLow())
		{
			uiValue &= ~(1 << i);
		}
	}
	if (uiValue != mauiValues[uiGroup])
	{
		mauiValues[uiGroup] = uiValue;
		WriteGroupOutputs(pcTimeline, uiGroup);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CRegister::GetType(void) { return mbTransparent ? LATCH_TYPE : REGISTER_TYPE; }
CGroupPins* CRegister::GetGroupPins(void) { return (CGroupPins*)&mpPins; }
bool CRegister::IsTransparent(void) { return mbTransparent; }
uint32 CRegister::GetValue(size uiGroup) { return (uiGroup < REGISTER_MAX_GROUPS) ? mauiValues[uiGroup] : 0; }
void CRegister::SetValue(size uiGroup, uint32 uiValue) { if (uiGroup < REGISTER_MAX_GROUPS) { mauiValues[uiGroup] = uiValue; } }
const char* CRegister::GetStateXMLName(void) { return "RegisterState"; }


//////////////////////////////////////////////////////////////////////////
//
// Each group's value, "value1" up.
//
//////////////////////////////////////////////////////////////////////////
void CRegister::SaveXML(CXMLWriter* pcWriter)
{
	size	i;
	CChars	szName;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		szName.Init("value");
		szName.Append((int32)(i + 1));
		pcWriter->Attribute(szName.Text(), (int64)mauiValues[i]);
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRegister::LoadXML(CMarkupTag* pcTag)
{
	size	i;
	CChars	szName;
	int64	iValue;
	bool	bLoaded;

	for (i = 0; i < GetGroupPins()->GetGroupCount(); i++)
	{
		szName.Init("value");
		szName.Append((int32)(i + 1));
		bLoaded = LoadStateInt64(pcTag, szName.Text(), 0, 0xFFFFFFFFLL, &iValue);
		szName.Kill();
		if (!bLoaded)
		{
			return false;
		}
		mauiValues[i] = (uint32)iValue;
	}
	return true;
}

