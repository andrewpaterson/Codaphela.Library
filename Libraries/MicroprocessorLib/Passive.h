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
#ifndef __PASSIVE_H__
#define __PASSIVE_H__
#include "CircuitComponent.h"
#include "PortHolder.h"


//Logicim's Passive: a component that holds its own ports and does nothing when the simulation runs, as a power
//source, a pin or a splitter.  Logicim joins the traces a passive's trace ports connect when it builds the circuit;
//the passive itself only names them.  Made with OMalloc; Init adds it to its circuit, see CCircuitComponent.
class CPassive : public CCircuitComponent, public CPortHolder
{
CONSTRUCTABLE(CPassive);
public:
			void	Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName);
			void	Class(void) override;
			void	Free(void) override;

	virtual void	Disconnect(CTimeline* pcTimeline);
	virtual void	SimulationStarted(CTimeline* pcTimeline);
	virtual void	Reset(CTimeline* pcTimeline);
			void	ResetPorts(void);
	virtual void	TraceConnected(CTimeline* pcTimeline, CPort* pcPort);

	virtual size	NumPorts(void);
	virtual CPort*	GetPort(size uiIndex);
	virtual CPort*	GetPort(const char* szName);
	virtual char*	GetComponentName(void);
	virtual void	AppendDescription(CChars* psz);
	virtual CCircuitComponent*	GetComponent(void);
};


typedef CArrayTemplatePtr<CPassive>	CArrayPassivePtr;


#endif // __PASSIVE_H__

