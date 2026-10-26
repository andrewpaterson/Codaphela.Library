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
#include "Circuit.h"
#include "Passive.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the passive to its circuit, which owns it.  Subclasses add their
// ports after calling this.
//
//////////////////////////////////////////////////////////////////////////
void CPassive::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName)
{
	PreInit();

	CCircuitComponent::Init(pcContainingSubcircuitSimulation, szName);
	CPortHolder::InitPorts();
	GetCircuit()->Add(this);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPassive::Class(void)
{
	CCircuitComponent::Class();

	U_Data(CArrayPortPtr, mapcPorts);
}


//////////////////////////////////////////////////////////////////////////
//
// Kills the ports, which disconnects them from their traces.
//
//////////////////////////////////////////////////////////////////////////
void CPassive::Free(void)
{
	CPortHolder::KillPorts();
	CCircuitComponent::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A passive has no events.
//
//////////////////////////////////////////////////////////////////////////
void CPassive::Disconnect(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPassive::SimulationStarted(CTimeline* pcTimeline)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPassive::Reset(CTimeline* pcTimeline)
{
	ResetPorts();
	SimulationStarted(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPassive::ResetPorts(void)
{
	size	i;

	for (i = 0; i < mapcPorts.NumElements(); i++)
	{
		mapcPorts.GetPtr(i)->Reset();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every Logicim passive ignores it.
//
//////////////////////////////////////////////////////////////////////////
void CPassive::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPassive::NumPorts(void) { return CPortHolder::NumPorts(); }
CPort* CPassive::GetPort(size uiIndex) { return CPortHolder::GetPort(uiIndex); }
CPort* CPassive::GetPort(const char* szName) { return CPortHolder::GetPort(szName); }
char* CPassive::GetComponentName(void) { return CCircuitComponent::GetComponentName(); }
void CPassive::AppendDescription(CChars* psz) { CCircuitComponent::AppendDescription(psz); }
CCircuitComponent* CPassive::GetComponent(void) { return this; }

