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
#ifndef __D_TYPE_FLIP_FLOP_H__
#define __D_TYPE_FLIP_FLOP_H__
#include "IntegratedCircuit.h"
#include "DTypeFlipFlopPins.h"


#define D_TYPE_FLIP_FLOP_TYPE	"D-type flip flop"


//Logicim's DTypeFlipFlop: on a rising clock it takes D, unless the active low set or reset is asserted.  Set and reset
//together set it, as an LVC part most likely does.  The state starts low and Q and QB are driven from it when the
//chip starts (see CIntegratedCircuit::WriteOutputs), and again whenever it changes; Logicim's only drives them when it
//changes, so they're undriven until it first goes high.  Undriven set and reset inputs are not asserted.  The state
//is what it remembers; ResetState sets it back to low.
class CDTypeFlipFlop : public CIntegratedCircuit
{
CONSTRUCTABLE(CDTypeFlipFlop);
protected:
	bool	mbState;

public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CDTypeFlipFlopPins* pcPins);

	virtual void					SimulationStarted(CTimeline* pcTimeline);
	virtual void					WriteOutputs(CTimeline* pcTimeline);
	virtual void					InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*				GetType(void);
	virtual void					ResetState(void);
			CDTypeFlipFlopPins*		GetDTypeFlipFlopPins(void);
			bool					GetState(void);
			void					SetState(bool bState);

protected:
			bool					ChangeState(bool bState);
};


#endif // __D_TYPE_FLIP_FLOP_H__

