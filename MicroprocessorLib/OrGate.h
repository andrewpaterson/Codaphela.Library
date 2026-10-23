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
#ifndef __OR_GATE_H__
#define __OR_GATE_H__
#include "IntegratedCircuit.h"
#include "OrGatePins.h"


#define OR_GATE_TYPE	"OR Gate"
#define NOR_GATE_TYPE	"NOR Gate"


//Logicim's OrGate: any high input drives the output high.  Otherwise an undriven input makes the output unknown, so it
//stops driving, see CAndLogic.  Otherwise any low input drives it low.  Unsettled inputs are ignored; with only them
//the output is left alone.
class COrGate : public CIntegratedCircuit
{
CONSTRUCTABLE(COrGate);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, COrGatePins* pcPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
	virtual CState*			CreateState(void);
			COrGatePins*	GetOrGatePins(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


//Logicim's NorGate.
class CNorGate : public COrGate
{
CONSTRUCTABLE(CNorGate);
public:
	virtual const char*		GetType(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


#endif // __OR_GATE_H__

