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
#ifndef __XOR_GATE_H__
#define __XOR_GATE_H__
#include "IntegratedCircuit.h"
#include "XorGatePins.h"


#define XOR_GATE_TYPE	"XOR Gate"
#define XNOR_GATE_TYPE	"XNOR Gate"


//Logicim's XorGate: high when an odd number of inputs are high.  An undriven input makes the output unknown, so it stops
//driving, see CAndLogic.  Unsettled inputs count as neither; with only them the output is left alone.
class CXorGate : public CIntegratedCircuit
{
CONSTRUCTABLE(CXorGate);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CXorGatePins* pcPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
			CXorGatePins*	GetXorGatePins(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


//Logicim's XnorGate.
class CXnorGate : public CXorGate
{
CONSTRUCTABLE(CXnorGate);
public:
	virtual const char*		GetType(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


#endif // __XOR_GATE_H__

