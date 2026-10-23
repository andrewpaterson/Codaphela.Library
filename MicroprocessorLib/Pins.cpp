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
#include "PowerInPort.h"
#include "PowerPinNames.h"
#include "LogicPort.h"
#include "IntegratedCircuit.h"
#include "Pins.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the VCC and GND ports.  Subclasses add their logic ports after
// calling this.
//
//////////////////////////////////////////////////////////////////////////
void CPins::Init(void)
{
	CPortHolder::Init();
	mpcIntegratedCircuit = NULL;

	mpcVCC = NewMalloc<CPowerInPort>();
	mpcVCC->Init(PT_PowerIn, POWER_PIN_VCC, this);
	mpcGND = NewMalloc<CPowerInPort>();
	mpcGND->Init(PT_PowerIn, POWER_PIN_GND, this);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and frees the ports.
//
//////////////////////////////////////////////////////////////////////////
void CPins::Kill(void)
{
	mpcVCC = NULL;
	mpcGND = NULL;
	mpcIntegratedCircuit = NULL;
	CPortHolder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPins::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	mpcIntegratedCircuit->InputTransition(pcTimeline, pcPort);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPins::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	mpcIntegratedCircuit->TraceConnected(pcTimeline, pcPort);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPins::GetName(void)
{
	return mpcIntegratedCircuit->GetName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPins::AppendDescription(CChars* psz)
{
	mpcIntegratedCircuit->AppendDescription(psz);
}


//////////////////////////////////////////////////////////////////////////
//
// szName, then " i" if there is more than one buffer, like "In 2".
//
//////////////////////////////////////////////////////////////////////////
void CPins::CalculatePortName(CChars* psz, const char* szName, size i, size uiBufferCount)
{
	psz->Append(szName);
	if (uiBufferCount > 1)
	{
		psz->Append(' ');
		psz->Append(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPins::SetIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit) { mpcIntegratedCircuit = pcIntegratedCircuit; }
CIntegratedCircuit* CPins::GetIntegratedCircuit(void) { return mpcIntegratedCircuit; }
CCircuitComponent* CPins::GetComponent(void) { return mpcIntegratedCircuit; }
const char* CPins::GetType(void) { return mpcIntegratedCircuit->GetType(); }
CPowerInPort* CPins::GetVoltageCommon(void) { return mpcVCC; }
CPowerInPort* CPins::GetVoltageGround(void) { return mpcGND; }

