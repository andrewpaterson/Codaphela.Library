/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __TIMER_LISTENER_H__
#define __TIMER_LISTENER_H__
#include "Listener.h"
#include "Unknown.h"


struct STimerData
{
	char*		szTimerName;
	float		fOversleep;
};


class CTimerListener : public CListener
{
CONSTRUCTABLE(CTimerListener); 
public:
	virtual void	TimerAlarm(CListener* pcSource, void* pvContext) {};
};


#endif // __TIMER_LISTENER_H__

