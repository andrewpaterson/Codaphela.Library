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
#ifndef __PROBE_H__
#define __PROBE_H__
#include "IntegratedCircuit.h"
#include "ProbePins.h"


#define PROBE_TYPE	"Probe"


//Logicim's Probe: inputs that something outside the simulation reads, as Logicim's view shows them.
class CProbe : public CIntegratedCircuit
{
CONSTRUCTABLE(CProbe);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CProbePins* pcPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
			CProbePins*		GetProbePins(void);
			uint64			ReadValue(CTimeline* pcTimeline);
};


#endif // __PROBE_H__

