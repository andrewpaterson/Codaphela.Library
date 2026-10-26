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
#ifndef __COUNTER_H__
#define __COUNTER_H__
#include "IntegratedCircuit.h"
#include "CounterPins.h"


#define COUNTER_TYPE	"Counter"


//Logicim's Counter, like a 74161: the active low reset clears it and the active low load takes the data, both at once
//rather than on the clock; otherwise, with both enables high, a rising clock counts, wrapping to zero after the
//terminal value.  Terminal count is high at the terminal value while enable terminal is high.  The count starts at
//zero and the outputs are driven from it when the chip starts (see CIntegratedCircuit::WriteOutputs), and again
//whenever they change; Logicim's only drives them when they change, so they're undriven until the count first
//changes.  The count and whether terminal count is high are what it remembers; ResetState sets them back to zero and
//low.
class CCounter : public CIntegratedCircuit
{
CONSTRUCTABLE(CCounter);
protected:
	size	muiBitWidth;
	uint32	muiTerminalValue;
	uint32	muiCount;
	bool	mbTerminal;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CCounterPins* pcPins, size uiBitWidth, uint32 uiTerminalValue);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
	virtual void			ResetState(void);
			void			AppendCounterValueHex(CChars* psz);
			CCounterPins*	GetCounterPins(void);
			uint32			GetCount(void);
			void			SetCount(uint32 uiCount);
			bool			IsTerminal(void);
			void			SetTerminal(bool bTerminal);
};


#endif // __COUNTER_H__

