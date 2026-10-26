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
#include "SubcircuitSimulation.h"
#include "Port.h"
#include "CircuitComponent.h"


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add themselves to the circuit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitComponent::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName)
{
	PreInit();

	mpcContainingSubcircuitSimulation = pcContainingSubcircuitSimulation;
	mszName.Init(szName);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitComponent::Class(void)
{
	U_Pointer(mpcContainingSubcircuitSimulation);
	U_String(mszName);
}


//////////////////////////////////////////////////////////////////////////
//
// The containing subcircuit simulation is kept, see the class comment.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitComponent::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitComponent::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitComponent::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Disconnects every port, telling the ports on traces it leaves
// undriven, as Logicim's Ports.disconnect.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitComponent::DisconnectPorts(CTimeline* pcTimeline)
{
	size	i;

	for (i = 0; i < NumPorts(); i++)
	{
		GetPort(i)->Disconnect(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like "AND \"U1\"", or just the type when there is no name.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitComponent::AppendDescription(CChars* psz)
{
	psz->Append(GetType());
	if (!mszName.Empty())
	{
		psz->Append(" \"");
		psz->Append(mszName);
		psz->Append("\"");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuit* CCircuitComponent::GetCircuit(void)
{
	return mpcContainingSubcircuitSimulation->GetCircuit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTimeline* CCircuitComponent::GetTimeline(void)
{
	return mpcContainingSubcircuitSimulation->GetTimeline();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCircuitComponent::GetComponentName(void) { return mszName.Text(); }
CSubcircuitSimulation* CCircuitComponent::GetContainingSubcircuitSimulation(void) { return mpcContainingSubcircuitSimulation; }

