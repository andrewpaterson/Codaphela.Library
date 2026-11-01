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
#include "CircuitSimulation.h"
#include "SubcircuitInstance.h"
#include "SubcircuitSimulation.h"


//The id the next subcircuit simulation is given.
int64	giNextSubcircuitSimulationID = 1;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::Init(CCircuitSimulation* pcCircuitSimulation)
{
	Init(pcCircuitSimulation, giNextSubcircuitSimulationID);
}


//////////////////////////////////////////////////////////////////////////
//
// Later subcircuit simulations are given ids after iID.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::Init(CCircuitSimulation* pcCircuitSimulation, int64 iID)
{
	mpcCircuitSimulation = pcCircuitSimulation;
	SetID(iID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::Kill(void)
{
	mpcCircuitSimulation = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::SetID(int64 iID)
{
	miID = iID;
	if (iID >= giNextSubcircuitSimulationID)
	{
		giNextSubcircuitSimulationID = iID + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Type[Top] ID[1] Simulation[Main (1)] ".
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::AppendDescription(CChars* psz)
{
	psz->Append("Type[");
	psz->Append(GetType());
	psz->Append("] ID[");
	psz->Append(miID);
	psz->Append("] Simulation[");
	if (mpcCircuitSimulation != NULL)
	{
		mpcCircuitSimulation->AppendDescription(psz);
	}
	psz->Append("] ");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitSimulation::ResetNextID(void)
{
	giNextSubcircuitSimulationID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSubcircuitSimulation::IsTop(void) { return false; }
bool CSubcircuitSimulation::IsInstance(void) { return false; }
int64 CSubcircuitSimulation::GetID(void) { return miID; }
CCircuitSimulation* CSubcircuitSimulation::GetCircuitSimulation(void) { return mpcCircuitSimulation; }
CCircuit* CSubcircuitSimulation::GetCircuit(void) { return mpcCircuitSimulation->GetCircuit(); }
CTimeline* CSubcircuitSimulation::GetTimeline(void) { return mpcCircuitSimulation->GetTimeline(); }
int64 CSubcircuitSimulation::GetTime(void) { return mpcCircuitSimulation->GetTime(); }
char* CSubcircuitSimulation::GetName(void) { return mpcCircuitSimulation->GetName(); }
bool CSubcircuitSimulation::RunSimultaneous(void) { return mpcCircuitSimulation->RunSimultaneous(); }
bool CSubcircuitSimulation::RunToTime(int64 iTimeForward) { return mpcCircuitSimulation->RunToTime(iTimeForward); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSubcircuitTopSimulation::GetType(void) { return "Top"; }
bool CSubcircuitTopSimulation::IsTop(void) { return true; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceSimulation::Init(CCircuitSimulation* pcCircuitSimulation, CSubcircuitInstance* pcSubcircuitInstance)
{
	CSubcircuitSimulation::Init(pcCircuitSimulation);
	mpcSubcircuitInstance = pcSubcircuitInstance;
}


//////////////////////////////////////////////////////////////////////////
//
// For a loaded simulation; its instance is set later.
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceSimulation::Init(CCircuitSimulation* pcCircuitSimulation, int64 iID)
{
	CSubcircuitSimulation::Init(pcCircuitSimulation, iID);
	mpcSubcircuitInstance = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The instance's name is added when it has one, like
// "Type[Instance] ID[2] Simulation[Main (1)]  Name[ALU]".
//
//////////////////////////////////////////////////////////////////////////
void CSubcircuitInstanceSimulation::AppendDescription(CChars* psz)
{
	char*	szName;

	CSubcircuitSimulation::AppendDescription(psz);
	if (mpcSubcircuitInstance != NULL)
	{
		szName = mpcSubcircuitInstance->GetComponentName();
		if ((szName != NULL) && (szName[0] != '\0'))
		{
			psz->Append(" Name[");
			psz->Append(szName);
			psz->Append("]");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSubcircuitInstanceSimulation::GetType(void) { return "Instance"; }
bool CSubcircuitInstanceSimulation::IsInstance(void) { return true; }
void CSubcircuitInstanceSimulation::SetSubcircuitInstance(CSubcircuitInstance* pcSubcircuitInstance) { mpcSubcircuitInstance = pcSubcircuitInstance; }
CSubcircuitInstance* CSubcircuitInstanceSimulation::GetSubcircuitInstance(void) { return mpcSubcircuitInstance; }

