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
//A chip is made with OMalloc; Init adds it to its circuit, see CCircuitComponent.  Its pins are made with OMalloc too
//and given to Init; the chip's pointer to them keeps them alive.  Freeing a chip takes its own events off the timeline;
//freeing its pins takes their ports' events off and disconnects the ports.
//
//A chip that remembers anything (a clock's level, a flip-flop's bit) keeps it in its own fields, set up by its Init
//and set back by ResetState.  Logicim keeps those in a separate State object; here they are part of the chip.
//
//A chip whose outputs come from what it remembers drives them from it in WriteOutputs, which Reset calls once the
//chip has started, as a real part drives its outputs from power up, and which the chip calls itself whenever what it
//remembers changes.
//
//A chip starts when its power comes (IsPowered) and stops when it goes, however that happens: the simulation
//starting, a supply switched or ramping, a VCC or GND wire connected or cut while it runs.  Its power ports tell it,
//through TraceConnected, and PowerChanged starts it (Reset, then it reads its inputs), stops it (its events dropped
//and its outputs undriven) or, when only its VCC has moved, has its ports write again at the new VCC.  Logicim's chips only drive them when what they
//remember changes, so until then they're undriven, and a chip fed back to itself (a flip-flop's QB to its D) never
//starts.  Chips whose outputs follow their inputs (gates) don't override it; undriven inputs leave them undriven.
//
//Logicim's IntegratedCircuit is generic over its pins and state; here a subclass casts GetPins.
//
//A chip that remembers anything saves it with a saved simulation: GetStateXMLName names the element it's saved in
//(NULL, the default, for a chip that remembers nothing), SaveXML writes the element's attributes and child elements
//and LoadXML sets what the chip remembers from them.
class CIntegratedCircuitEvent;
class CXMLWriter;
class CMarkupTag;
class CVoltageConfigurationSource;
class CIntegratedCircuit : public CCircuitComponent
{
CONSTRUCTABLE(CIntegratedCircuit);
protected:
	Ptr<CPins>		mpPins;
	CArrayEventPtr	mapcEvents;
	bool			mbPowered;
	float32			mfPoweredVcc;  //The VCC when the chip last saw its power change.

public:
			void							Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CPins> pPins);
			void							Class(void) override;
			void							Free(void) override;

	virtual void							ExecuteTick(CTimeline* pcTimeline);
	virtual void							TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual void							Disconnect(CTimeline* pcTimeline);
	virtual void							Reset(CTimeline* pcTimeline);
	virtual void							ResetState(void);
	virtual void							WriteOutputs(CTimeline* pcTimeline);

	virtual void							SimulationStarted(CTimeline* pcTimeline) =0;
	virtual void							InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort) =0;
	virtual const char*						GetType(void) =0;

	virtual bool							NeedsPower(void) const;
	virtual const char*						GetStateXMLName(void) const;
	virtual void							SaveXML(CXMLWriter* pcWriter);
	virtual bool							LoadXML(CMarkupTag* pcTag);

	virtual size							NumPorts(void);
	virtual CPort*							GetPort(size uiIndex);
	virtual CPort*							GetPort(const char* szName);

			CPins*							GetPins(void);

			float32							GetVCC(int64 iTime);
			float32							GetGND(int64 iTime);
			bool							IsPowered(int64 iTime);
			CVoltageConfigurationSource*	GetVoltageConfigurationSource(void);
			void							PowerChanged(CTimeline* pcTimeline);

			void							Add(CIntegratedCircuitEvent* pcEvent);
			bool							Remove(CIntegratedCircuitEvent* pcEvent);
			size							NumEvents(void) const;
			CEvent*							GetEvent(size uiIndex);

protected:
			void							ResetEventsAndPorts(void);
			void							RemoveAllEvents(CTimeline* pcTimeline);
			void							Start(CTimeline* pcTimeline);
			void							Stop(CTimeline* pcTimeline);
};


typedef CArrayTemplatePtr<CIntegratedCircuit>	CArrayIntegratedCircuitPtr;


#endif // __INTEGRATED_CIRCUIT_H__

