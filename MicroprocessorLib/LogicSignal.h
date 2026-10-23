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
#ifndef __LOGIC_SIGNAL_H__
#define __LOGIC_SIGNAL_H__
#include "TraceValue.h"


//One signal a chip's pins drive and read, whatever it is connected to.  CMetaTrace is an ideal trace: a write is seen
//at once and the timeline is ignored.  CLogicPort is Logicim's LogicPort on an analog CTrace: a write slews the
//voltage after the family's hold time and a read compares the trace's voltage with the family's input thresholds.
class CTimeline;
class CLogicSignal
{
public:
	virtual void			Write(CTimeline* pcTimeline, bool bValue) =0;
	virtual void			WriteImpedance(CTimeline* pcTimeline) =0;
	virtual STraceValue		Read(CTimeline* pcTimeline) =0;
};


//For pins that may not be connected; an unconnected pin reads undriven.
#define SafeWriteSignal(p, t, b) if (p) { p->Write(t, b); }
#define SafeReadSignal(p, t) if (p) { return p->Read(t); } else { STraceValue sUndriven; sUndriven.Init(TV_Undriven); return sUndriven; }


#endif // __LOGIC_SIGNAL_H__

