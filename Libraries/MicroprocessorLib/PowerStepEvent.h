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
#ifndef __POWER_STEP_EVENT_H__
#define __POWER_STEP_EVENT_H__
#include "Event.h"


//A supply's next step as it ramps on or off: the supply's port (CPowerOutPort::Step) tells its trace's other ports
//that its voltage has moved.  The port keeps a pointer to the event until it executes or is removed.
class CPowerOutPort;
class CPowerStepEvent : public CEvent
{
CONSTRUCTABLE(CPowerStepEvent);
protected:
	CPowerOutPort*	mpcPort;

public:
	static	CPowerStepEvent*	Schedule(CPowerOutPort* pcPort, int64 iTime, CTimeline* pcTimeline);

			void				Init(CPowerOutPort* pcPort, int64 iTime);
	virtual void				Execute(CTimeline* pcTimeline);
	virtual void				RemoveFromOwner(void);
};


#endif // __POWER_STEP_EVENT_H__

