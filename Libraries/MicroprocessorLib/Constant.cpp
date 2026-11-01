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
#include "Constant.h"


//////////////////////////////////////////////////////////////////////////
//
// Keeps pPins.
//
//////////////////////////////////////////////////////////////////////////
void CConstant::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CConstantPins> pPins, int64 iDefaultValue)
{
	PreInit();

	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pPins);
	miDefaultValue = iDefaultValue;
	ResetState();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstant::Class(void)
{
	CIntegratedCircuit::Class();

	U_Int64(miDefaultValue);
	U_Int64(miConstantValue);
}

//////////////////////////////////////////////////////////////////////////
//
// The default value.
//
//////////////////////////////////////////////////////////////////////////
void CConstant::ResetState(void)
{
	miConstantValue = miDefaultValue;
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim notes that placing a constant disconnects and reconnects it,
// which takes these slews off the timeline again.
//
//////////////////////////////////////////////////////////////////////////
void CConstant::SimulationStarted(CTimeline* pcTimeline)
{
	CArrayLogicPortPtr*	papcOutputs;
	uint64				uiConstantValue;
	size				i;

	papcOutputs = GetConstantPins()->GetOutputs();
	uiConstantValue = (uint64)miConstantValue;
	for (i = 0; i < papcOutputs->NumElements(); i++)
	{
		papcOutputs->GetPtr(i)->WriteBool(pcTimeline, (uiConstantValue & 1) == 1);
		uiConstantValue >>= 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstant::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstant::GetType(void) { return CONSTANT_TYPE; }
CConstantPins* CConstant::GetConstantPins(void) { return (CConstantPins*)&mpPins; }
int64 CConstant::GetConstantValue(void) { return miConstantValue; }
void CConstant::SetConstantValue(int64 iConstantValue) { miConstantValue = iConstantValue; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstant::GetStateXMLName(void) { return "ConstantState"; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstant::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("value", GetConstantValue());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConstant::LoadXML(CMarkupTag* pcTag)
{
	int64	iValue;

	if (!pcTag->GetInt64Attribute("value", &iValue))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number value.", NULL);
	}
	SetConstantValue(iValue);
	return true;
}

