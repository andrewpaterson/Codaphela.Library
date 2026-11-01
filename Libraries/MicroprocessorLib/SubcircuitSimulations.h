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
#ifndef __SUBCIRCUIT_SIMULATIONS_H__
#define __SUBCIRCUIT_SIMULATIONS_H__
#include "SubcircuitSimulation.h"


struct SCircuitSubcircuitSimulations
{
	CCircuitSimulation*				pcCircuitSimulation;
	CArraySubcircuitSimulationPtr	apcSubcircuitSimulations;
};


//Allocated one by one so the arrays handed out by GetSubcircuitSimulations don't move.
typedef CArrayTemplatePtr<SCircuitSubcircuitSimulations>	CArrayCircuitSubcircuitSimulationsPtr;


//Logicim's SubcircuitSimulations: the subcircuit simulations of each circuit simulation, in the order they were
//added.  It doesn't own them; their circuit simulations do.  Logicim's validate checks them against the editor's
//circuit instance views and is not ported.
class CSubcircuitSimulations
{
protected:
	CArrayCircuitSubcircuitSimulationsPtr	mapsCircuitSimulations;

public:
	void							Init(void);
	void							Kill(void);

	bool							Add(CSubcircuitSimulation* pcSubcircuitSimulation);
	bool							Remove(CSubcircuitSimulation* pcSubcircuitSimulation);
	void							Clear(void);

	CArraySubcircuitSimulationPtr*	GetSubcircuitSimulations(CCircuitSimulation* pcCircuitSimulation);
	void							GetSubcircuitTopSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations);
	void							GetSubcircuitInstanceSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations);
	void							GetSubcircuitSimulations(CArraySubcircuitSimulationPtr* papcSubcircuitSimulations);
	size							NumCircuitSimulations(void);
	CCircuitSimulation*				GetCircuitSimulation(size uiIndex);

protected:
	SCircuitSubcircuitSimulations*	Find(CCircuitSimulation* pcCircuitSimulation, size* piIndex);
};


#endif // __SUBCIRCUIT_SIMULATIONS_H__

