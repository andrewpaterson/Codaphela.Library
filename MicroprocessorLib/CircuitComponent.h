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
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Objects.h"


//Logicim's Component, with CircuitElement and Described: something on a CCircuit with ports, either a
//CIntegratedCircuit or a CPassive.  It is made in a CSubcircuitSimulation, through which it reaches its circuit and
//timeline.  A component is a CObject made with OMalloc; Init adds it to the circuit, whose pointer to it keeps it
//alive.  Removing it from the circuit frees it, unless something else still points to it.
//
//Components are freed in batches (a circuit and everything in it) in no particular order, so Free only lets go of
//what the component itself holds.  mpcContainingSubcircuitSimulation is left set by Free so the component's pins,
//freed in the same batch, can still reach the timeline.
//
//To take a component out of a running simulation, as Logicim's editor does, call Disconnect and DisconnectPorts, so
//the ports on its traces see them go undriven, then remove it from the circuit.
class CTimeline;
class CPort;
class CCircuit;
class CSubcircuitSimulation;
class CCircuitComponent : public CObject
{
CONSTRUCTABLE(CCircuitComponent);
protected:
	CSubcircuitSimulation*	mpcContainingSubcircuitSimulation;
	CChars					mszName;

public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName);
			void					Class(void) override;
			void					Free(void) override;

			bool					Save(CObjectWriter* pcFile) override;
			bool					Load(CObjectReader* pcFile) override;

	virtual const char*				GetType(void) =0;
	virtual void					Disconnect(CTimeline* pcTimeline) =0;
	virtual void					SimulationStarted(CTimeline* pcTimeline) =0;
	virtual void					Reset(CTimeline* pcTimeline) =0;
	virtual size					NumPorts(void) =0;
	virtual CPort*					GetPort(size uiIndex) =0;
	virtual CPort*					GetPort(const char* szName) =0;

			void					DisconnectPorts(CTimeline* pcTimeline);

	virtual char*					GetComponentName(void);  //Not GetName, that is the object's name.
	virtual void					AppendDescription(CChars* psz);
			CSubcircuitSimulation*	GetContainingSubcircuitSimulation(void);
			CCircuit*				GetCircuit(void);
			CTimeline*				GetTimeline(void);
};


typedef CArrayTemplatePtr<CCircuitComponent>	CArrayCircuitComponentPtr;


#endif // __CIRCUIT_COMPONENT_H__

