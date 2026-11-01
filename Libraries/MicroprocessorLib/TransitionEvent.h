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
#ifndef __TRANSITION_EVENT_H__
#define __TRANSITION_EVENT_H__
#include "PortInputEvent.h"


//Logicim's TransitionEvent: the voltage on a port's trace crosses one of its input thresholds.  When it executes the
//port's chip is told an input has changed.
class CTimeline;
class CTransitionEvent : public CPortInputEvent
{
CONSTRUCTABLE(CTransitionEvent);
protected:
	float32		mfVoltage;

public:
	static	CTransitionEvent*	Schedule(CLogicPort* pcPort, float32 fVoltage, int64 iDelay, CTimeline* pcTimeline);

			void				Init(CLogicPort* pcPort, int64 iTime, float32 fVoltage);
			void				Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fVoltage);

	virtual void				Execute(CTimeline* pcTimeline);
			float32				GetVoltage(void);
	virtual void				PrintShort(CChars* psz);
};


#endif // __TRANSITION_EVENT_H__

