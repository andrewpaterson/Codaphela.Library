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
#ifndef __CIRCUIT_H__
#define __CIRCUIT_H__
#include "IntegratedCircuit.h"
#include "Trace.h"


//Logicim's Circuit, with the Simulation's timeline: the chips and traces of a circuit.  It owns both; killing it
//kills the chips first, which takes their events off the timeline and disconnects their ports, then the traces.  So
//the circuit is killed before its timeline.  Logicim's passives are not ported.
class CTimeline;
class CCircuit
{
protected:
	CTimeline*					mpcTimeline;
	CArrayIntegratedCircuitPtr	mapcIntegratedCircuits;
	CArrayTracePtr				mapcTraces;

public:
	void				Init(CTimeline* pcTimeline);
	void				Kill(void);

	void				Add(CIntegratedCircuit* pcIntegratedCircuit);
	bool				Remove(CIntegratedCircuit* pcIntegratedCircuit);
	CTrace*				AddTrace(void);
	bool				IsEmpty(void);

	void				SimulationStarted(void);
	void				GetIntegratedCircuits(const char* szType, CArrayIntegratedCircuitPtr* papcIntegratedCircuits);
	size				NumIntegratedCircuits(void);
	CIntegratedCircuit*	GetIntegratedCircuit(size uiIndex);
	size				NumTraces(void);
	CTrace*				GetTrace(size uiIndex);
	CTimeline*			GetTimeline(void);
};


#endif // __CIRCUIT_H__

