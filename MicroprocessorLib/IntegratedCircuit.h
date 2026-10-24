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
#ifndef __INTEGRATED_CIRCUIT_H__
#define __INTEGRATED_CIRCUIT_H__
#include "CircuitComponent.h"
#include "Event.h"
#include "Pins.h"


//Logicim's IntegratedCircuit: a chip with CPins.  Subclasses create their state, start it when the simulation starts
//and respond when an input transitions; a clocked chip can schedule CTickEvents.
//
//A chip is made with NewMalloc; Init adds it to its circuit, which kills and frees it, see CCircuitComponent.  The chip
//owns its pins, which are made with NewMalloc too.  Killing a chip takes its events, and its ports' events, off the
//timeline and disconnects its ports.
//
//A chip that remembers anything (a clock's level, a flip-flop's bit) keeps it in its own fields, set up by its Init
//and set back by ResetState.  Logicim keeps those in a separate State object; here they are part of the chip.
//
//Logicim's IntegratedCircuit is generic over its pins and state; here a subclass casts GetPins.
class CIntegratedCircuitEvent;
class CIntegratedCircuit : public CCircuitComponent
{
CONSTRUCTABLE(CIntegratedCircuit);
protected:
	CPins*			mpcPins;
	CArrayEventPtr	mapcEvents;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CPins* pcPins);
	virtual void			Kill(void);

	virtual void			ExecuteTick(CTimeline* pcTimeline);
	virtual void			TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual void			Disconnect(CTimeline* pcTimeline);
	virtual void			Reset(CTimeline* pcTimeline);
	virtual void			ResetState(void);

	virtual void			SimulationStarted(CTimeline* pcTimeline) =0;
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort) =0;
	virtual const char*		GetType(void) =0;

	virtual size			NumPorts(void);
	virtual CPort*			GetPort(size uiIndex);
	virtual CPort*			GetPort(const char* szName);

			CPins*			GetPins(void);

			float32			GetVCC(int64 iTime);
			float32			GetGND(int64 iTime);
			bool			IsPowered(int64 iTime);

			void			Add(CIntegratedCircuitEvent* pcEvent);
			bool			Remove(CIntegratedCircuitEvent* pcEvent);
			size			NumEvents(void);
			CEvent*			GetEvent(size uiIndex);

protected:
			void			ResetEventsAndPorts(void);
};


typedef CArrayTemplatePtr<CIntegratedCircuit>	CArrayIntegratedCircuitPtr;


#endif // __INTEGRATED_CIRCUIT_H__

