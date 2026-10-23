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
#ifndef __TICK_EVENT_H__
#define __TICK_EVENT_H__
#include "IntegratedCircuitEvent.h"


//Logicim's TickEvent: calls the chip's ExecuteTick, as a clock does.
class CTimeline;
class CTickEvent : public CIntegratedCircuitEvent
{
CONSTRUCTABLE(CTickEvent);
public:
	static	CTickEvent*		Schedule(int64 iDelay, CIntegratedCircuit* pcIntegratedCircuit, CTimeline* pcTimeline);

	virtual void			Execute(CTimeline* pcTimeline);
};


#endif // __TICK_EVENT_H__

