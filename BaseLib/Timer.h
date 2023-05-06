/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __TIMER_H__
#define __TIMER_H__
#include "Chars.h"


class CTimer
{
private:
	int		miStart;
	int		miStart2;

public:
	int		miTotalTime;  //Time in milliseconds since the timer was intialised/reset.
	int		miUpdateTime;  //Time in milliseconds since the last update function.
	float	mfTotalTime;
	float	mfUpdateTime;
	float	mfUpdatesPerSecond;

	void Init(void);
	void Update(void);
	void Reset(void);
	void Kill(void);

	float GetUpdateTimeInSeconds(void) { return mfUpdateTime; }
	float GetUpdatesPerSecond(void) { return mfUpdatesPerSecond; }
	float GetTotalTimeInSeconds(void) { return mfTotalTime; }

	void HumanReadable(CChars* psz);
};

void DebugTimerTick(void);

extern CTimer	gcTimer;


#endif //__TIMER_H__

