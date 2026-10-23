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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "CircuitSimulation.h"


//The id the next circuit simulation is given.
int64	giNextCircuitSimulationID = 1;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::Init(const char* szName)
{
	Init(giNextCircuitSimulationID, szName);
}


//////////////////////////////////////////////////////////////////////////
//
// Later circuit simulations are given ids after iID.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::Init(int64 iID, const char* szName)
{
	mszName.Init(szName);
	mcTimeline.Init(this);
	mcCircuit.Init();

	miID = iID;
	if (iID >= giNextCircuitSimulationID)
	{
		giNextCircuitSimulationID = iID + 1;
	}

	mpcSubcircuitTopSimulation = NewMalloc<CSubcircuitTopSimulation>();
	mpcSubcircuitTopSimulation->Init(this);
	mapcInstanceSimulations.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit, so its components can take their events off the
// timeline, then the subcircuit simulations and the timeline.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::Kill(void)
{
	size					i;
	CSubcircuitSimulation*	pcSubcircuitSimulation;

	mcCircuit.Kill();

	for (i = 0; i < mapcInstanceSimulations.NumElements(); i++)
	{
		pcSubcircuitSimulation = mapcInstanceSimulations.GetPtr(i);
		SafeKill(pcSubcircuitSimulation);
	}
	mapcInstanceSimulations.Kill();
	SafeKill(mpcSubcircuitTopSimulation);

	mcTimeline.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new timeline.  Fails if the circuit still has components; they are
// made again against the new timeline.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSimulation::Reset(void)
{
	CChars	sz;

	if (!mcCircuit.IsEmpty())
	{
		sz.Init(" Cannot reset Circuit Simulation [");
		AppendDescription(&sz);
		sz.Append("].  Its Circuit still contains components.");
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
		return false;
	}
	mcTimeline.Kill();
	mcTimeline.Init(this);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Resets every component, as Logicim does when a simulation starts.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::SimulationStarted(void)
{
	mcCircuit.SimulationStarted(&mcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation of an instance of a subcircuit in this circuit; it
// shares this circuit and timeline.
//
//////////////////////////////////////////////////////////////////////////
CSubcircuitInstanceSimulation* CCircuitSimulation::AddInstanceSimulation(CSubcircuitInstance* pcSubcircuitInstance)
{
	CSubcircuitInstanceSimulation*	pcSimulation;

	pcSimulation = NewMalloc<CSubcircuitInstanceSimulation>();
	pcSimulation->Init(this, pcSubcircuitInstance);
	mapcInstanceSimulations.Add(pcSimulation);
	return pcSimulation;
}


//////////////////////////////////////////////////////////////////////////
//
// For a loaded simulation.
//
//////////////////////////////////////////////////////////////////////////
CSubcircuitInstanceSimulation* CCircuitSimulation::AddInstanceSimulation(int64 iID)
{
	CSubcircuitInstanceSimulation*	pcSimulation;

	pcSimulation = NewMalloc<CSubcircuitInstanceSimulation>();
	pcSimulation->Init(this, iID);
	mapcInstanceSimulations.Add(pcSimulation);
	return pcSimulation;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Main (1)", or just the id when there is no name.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::AppendDescription(CChars* psz)
{
	if (mszName.Empty())
	{
		psz->Append(miID);
	}
	else
	{
		psz->Append(mszName);
		psz->Append(" (");
		psz->Append(miID);
		psz->Append(")");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSimulation::ResetNextID(void)
{
	giNextCircuitSimulationID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuit* CCircuitSimulation::GetCircuit(void) { return &mcCircuit; }
CTimeline* CCircuitSimulation::GetTimeline(void) { return &mcTimeline; }
int64 CCircuitSimulation::GetTime(void) { return mcTimeline.GetTime(); }
bool CCircuitSimulation::RunSimultaneous(void) { return mcTimeline.RunSimultaneous(); }
bool CCircuitSimulation::RunToTime(int64 iTimeForward) { return mcTimeline.RunToTime(iTimeForward); }
int64 CCircuitSimulation::GetID(void) { return miID; }
char* CCircuitSimulation::GetName(void) { return mszName.Text(); }
CSubcircuitTopSimulation* CCircuitSimulation::GetSubcircuitTopSimulation(void) { return mpcSubcircuitTopSimulation; }
size CCircuitSimulation::NumInstanceSimulations(void) { return mapcInstanceSimulations.NumElements(); }
CSubcircuitInstanceSimulation* CCircuitSimulation::GetInstanceSimulation(size uiIndex) { return (CSubcircuitInstanceSimulation*)mapcInstanceSimulations.SafeGetPtr(uiIndex); }

