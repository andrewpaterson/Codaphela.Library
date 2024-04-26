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
#ifndef __TIMER_ALARM_H__
#define __TIMER_ALARM_H__
#include "BaseLib/Timer.h"
#include "BaseLib/Define.h"
#include "Action.h"
#include "TimerListener.h"


class CTimerAlarm
{
public:
	bool		mbRepeat;
	int			miTime;  //How long before the action occurs. All times in milliseconds.
	int			miRemainingTime;
	bool		mbWaitForFirstUpdate;
	bool		mbRunning;
	int			miId;
	CChars		mszName;
	CAction*	mpcAction;

	void	Init(CAction* pcAction, char* szTimerName, int iTime, bool bRepeat);
	void	Kill(void);
	float	Update(void);
	void	Reset(void);
	void	Start(void);
	void	Stop(void);
	void	Pause(int iPauseTime);
};


#endif // __TIMER_ALARM_H__

