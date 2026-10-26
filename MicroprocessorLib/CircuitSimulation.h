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
#ifndef __CIRCUIT_SIMULATION_H__
#define __CIRCUIT_SIMULATION_H__
#include "Timeline.h"
#include "Circuit.h"
#include "SubcircuitSimulation.h"


//Logicim's CircuitSimulation, with its Simulation folded in: a circuit and the timeline it runs on.  Logicim's
//Simulation only wraps the timeline; its GetTime, RunSimultaneous, RunToTime and RemoveEvent are here, and the events
//that Logicim passes the Simulation are passed the timeline, whose context is the circuit simulation.  It owns both, its CSubcircuitTopSimulation, made in Init as Logicim's editor does, and the
//CSubcircuitInstanceSimulations added to it.  The circuit is a CObject made in Init and kept by mpCircuit; Kill lets
//go of it, which frees it and its components, before the timeline, so its components can take their events off it.  The timeline's context is the circuit simulation.  Every one gets the next id unless it is
//given one.
class CCircuitSimulation
{
protected:
	Ptr<CCircuit>					mpCircuit;
	CTimeline						mcTimeline;
	int64							miID;
	CChars							mszName;
	CSubcircuitTopSimulation*		mpcSubcircuitTopSimulation;
	CArraySubcircuitSimulationPtr	mapcInstanceSimulations;

public:
	void							Init(const char* szName);
	void							Init(int64 iID, const char* szName);
	void							Kill(void);

	bool							Reset(void);
	void							SimulationStarted(void);
	CSubcircuitInstanceSimulation*	AddInstanceSimulation(CSubcircuitInstance* pcSubcircuitInstance);
	CSubcircuitInstanceSimulation*	AddInstanceSimulation(int64 iID);
	bool							RemoveInstanceSimulation(CSubcircuitInstanceSimulation* pcSimulation);

	CCircuit*						GetCircuit(void);
	CTimeline*						GetTimeline(void);
	int64							GetTime(void);
	bool							RunSimultaneous(void);
	bool							RunToTime(int64 iTimeForward);
	bool							RemoveEvent(CEvent* pcEvent);
	int64							GetID(void);
	char*							GetName(void);
	void							AppendDescription(CChars* psz);
	CSubcircuitTopSimulation*		GetSubcircuitTopSimulation(void);
	size							NumInstanceSimulations(void);
	CSubcircuitInstanceSimulation*	GetInstanceSimulation(size uiIndex);

	static void						ResetNextID(void);
};


#endif // __CIRCUIT_SIMULATION_H__

