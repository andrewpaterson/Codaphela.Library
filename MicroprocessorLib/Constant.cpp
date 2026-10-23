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
#include "Constant.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins.
//
//////////////////////////////////////////////////////////////////////////
void CConstant::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CConstantPins* pcPins, int64 iDefaultValue)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);
	miDefaultValue = iDefaultValue;
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
	uiConstantValue = (uint64)GetConstantState()->GetConstantValue();
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
CState* CConstant::CreateState(void)
{
	CConstantState*	pcState;

	pcState = NewMalloc<CConstantState>();
	pcState->Init(miDefaultValue);
	return pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstant::GetType(void) { return CONSTANT_TYPE; }
CConstantPins* CConstant::GetConstantPins(void) { return (CConstantPins*)mpcPins; }
CConstantState* CConstant::GetConstantState(void) { return (CConstantState*)mpcState; }

