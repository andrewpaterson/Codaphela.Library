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
#ifndef __CIRCUIT_COMPONENT_H__
#define __CIRCUIT_COMPONENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"


//Logicim's Component, with CircuitElement and Described: something on a CCircuit with ports, either a
//CIntegratedCircuit or a CPassive.  It is made in a CSubcircuitSimulation, through which it reaches its circuit and
//timeline.  Init adds it to the circuit, which owns it.
//
//To take a component out of a running simulation, as Logicim's editor does, call Disconnect and DisconnectPorts, so
//the ports on its traces see them go undriven, then remove it from the circuit and kill it.
class CTimeline;
class CPort;
class CCircuit;
class CSubcircuitSimulation;
class CCircuitComponent : public CConstructable
{
CONSTRUCTABLE(CCircuitComponent);
protected:
	CSubcircuitSimulation*	mpcContainingSubcircuitSimulation;
	CChars					mszName;

public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName);
	virtual void					Kill(void);

	virtual const char*				GetType(void) =0;
	virtual void					Disconnect(CTimeline* pcTimeline) =0;
	virtual void					SimulationStarted(CTimeline* pcTimeline) =0;
	virtual void					Reset(CTimeline* pcTimeline) =0;
	virtual size					NumPorts(void) =0;
	virtual CPort*					GetPort(size uiIndex) =0;
	virtual CPort*					GetPort(const char* szName) =0;

			void					DisconnectPorts(CTimeline* pcTimeline);

	virtual char*					GetName(void);
	virtual void					AppendDescription(CChars* psz);
			CSubcircuitSimulation*	GetContainingSubcircuitSimulation(void);
			CCircuit*				GetCircuit(void);
			CTimeline*				GetTimeline(void);
};


#endif // __CIRCUIT_COMPONENT_H__

