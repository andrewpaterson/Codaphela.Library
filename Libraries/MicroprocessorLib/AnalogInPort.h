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
#ifndef __ANALOG_IN_PORT_H__
#define __ANALOG_IN_PORT_H__
#include "LogicPort.h"


//An input that follows its trace's voltage, not its level: an RC element's (CRCFilter) input.  Its chip is told
//(InputTransition) whenever the voltage its trace is going to changes, and GetTarget is that voltage, NaN when the
//trace is undriven.  A slew on the trace is told halfway along it (a linear slew's middle stands for the whole of it to
//a curve much slower than the slew), at its end voltage; a supply or pull moving the trace, its start, or the trace
//let go, next tick.
class CAnalogInPort : public CLogicPort
{
CONSTRUCTABLE(CAnalogInPort);
protected:
	float32		mfTarget;

public:
			void		Init(CPins* pcPins, const char* szName);
	virtual void		ReadTraceLevel(CTimeline* pcTimeline);
	virtual bool		TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew, bool bTraceWasUndriven = false);
	virtual void		TraceUndriven(CTimeline* pcTimeline);
	virtual void		TraceVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage);
	virtual void		TransitionEvent(CTimeline* pcTimeline, CTransitionEvent* pcEvent);
	virtual void		Reset(void);
			float32		GetTarget(void) const;
};


#endif // __ANALOG_IN_PORT_H__

