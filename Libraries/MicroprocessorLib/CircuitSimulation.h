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
#include "BaseLib/StdRandom.h"
#include "Timeline.h"
#include "Circuit.h"
#include "SubcircuitSimulation.h"


//What a chip remembers (a flip-flop's bit, a counter's count, a register) when its power comes.
enum EPowerUpState
{
	PUS_Zero,  //Every bit low, as Logicim's.
	PUS_Random,  //Each bit high or low from the simulation's seed, as a real part comes up.
	PUS_HalfSupply,  //Not known: the chip's outputs at VCC/2 until it is reset or clocked.
};


//Logicim's CircuitSimulation, with its Simulation folded in: a circuit and the timeline it runs on.  Logicim's
//Simulation only wraps the timeline; its GetTime, RunSimultaneous, RunToTime and RemoveEvent are here, and the events
//that Logicim passes the Simulation are passed the timeline, whose context is the circuit simulation.  It owns both, its CSubcircuitTopSimulation, made in Init as Logicim's editor does, and the
//CSubcircuitInstanceSimulations added to it.  It is a CObject: make it with OMalloc and keep it under the root.
//
//The circuit is a CObject made in Init and kept by mpCircuit.  The circuit needs (Nes) the simulation, so the
//simulation is freed after the circuit and its components, which take their events off its timeline as they are
//freed.  The timeline's context is the circuit simulation.  Every one gets the next id unless it is given one.
//
//It keeps the power-up state, what chips remember when their power comes (EPowerUpState), Zero unless it is set, and the
//seed its random bits come from.  The generator starts again from the seed each time the simulation starts, and the
//chips draw from it in the circuit's order, so a run repeats.
class CCircuitSimulation : public CObject
{
CONSTRUCTABLE(CCircuitSimulation);
DESTRUCTABLE(CCircuitSimulation);
protected:
	Ptr<CCircuit>					mpCircuit;
	CTimeline						mcTimeline;
	int64							miID;
	CChars							mszName;
	CSubcircuitTopSimulation*		mpcSubcircuitTopSimulation;
	CArraySubcircuitSimulationPtr	mapcInstanceSimulations;
	EPowerUpState					mePowerUpState;
	uint32							muiPowerUpSeed;
	CRandom							mcPowerUpRandom;

public:
	void							Init(const char* szName);
	void							Init(int64 iID, const char* szName);
	void							Class(void) override;
	void							Free(void) override;

	bool							Save(CObjectWriter* pcFile) override;
	bool							Load(CObjectReader* pcFile) override;

	bool							Reset(void);
	void							SimulationStarted(void);
	CSubcircuitInstanceSimulation*	AddInstanceSimulation(CSubcircuitInstance* pcSubcircuitInstance);
	CSubcircuitInstanceSimulation*	AddInstanceSimulation(int64 iID);
	bool							RemoveInstanceSimulation(CSubcircuitInstanceSimulation* pcSimulation);

	CCircuit*						GetCircuit(void);
	CTimeline*						GetTimeline(void);
	int64							GetTime(void) const;
	bool							RunSimultaneous(void);
	bool							RunToTime(int64 iTimeForward);
	bool							RemoveEvent(CEvent* pcEvent);
	int64							GetID(void) const;
	const char*						GetName(void) const override;
	void							AppendDescription(CChars* psz);
	CSubcircuitTopSimulation*		GetSubcircuitTopSimulation(void) const;
	size							NumInstanceSimulations(void) const;
	CSubcircuitInstanceSimulation*	GetInstanceSimulation(size uiIndex);

	void							SetPowerUpState(EPowerUpState ePowerUpState);
	EPowerUpState					GetPowerUpState(void) const;
	void							SetPowerUpSeed(uint32 uiSeed);
	uint32							GetPowerUpSeed(void) const;
	bool							NextPowerUpBit(void);
	bool							IsPoweringUp(void);

	static void						ResetNextID(void);
};


#endif // __CIRCUIT_SIMULATION_H__

