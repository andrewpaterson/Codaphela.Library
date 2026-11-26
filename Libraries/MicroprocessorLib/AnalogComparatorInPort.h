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
#ifndef __ANALOG_COMPARATOR_IN_PORT_H__
#define __ANALOG_COMPARATOR_IN_PORT_H__
#include "LogicPort.h"


//A comparator's offset either side of the other input's voltage (a MAX9010's, typical): an input is above the other
//once it is this much over it, below once this much under, and between the two the output doesn't change.
#define ANALOG_COMPARATOR_OFFSET	0.001f


//An input of an analog comparator (CAnalogComparator): it reads its trace's voltage against the other input's
//(SetOther), not against a family's thresholds.  A slew on its trace is a transition where it crosses the other's
//voltage, as it is when the slew starts, by ANALOG_COMPARATOR_OFFSET; a supply or pull moving the trace, or the trace let go,
//next tick.  The other input moving at the same time isn't followed.
class CAnalogComparatorInPort : public CLogicPort
{
CONSTRUCTABLE(CAnalogComparatorInPort);
protected:
	CLogicPort*		mpcOther;

public:
			void		Init(CPins* pcPins, const char* szName);
			void		SetOther(CLogicPort* pcOther);
	virtual void		ReadTraceLevel(CTimeline* pcTimeline);
	virtual bool		TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew, bool bTraceWasUndriven = false);
	virtual void		TraceUndriven(CTimeline* pcTimeline);
	virtual void		TraceVoltageChanged(CTimeline* pcTimeline, float32 fOldVoltage);
			float32		GetVoltage(int64 iTime);
};


#endif // __ANALOG_COMPARATOR_IN_PORT_H__

