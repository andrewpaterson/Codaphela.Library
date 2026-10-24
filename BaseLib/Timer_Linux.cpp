/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifdef LINUX_GNU
#include <time.h>
#include "Timer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 GetDebugTickCount()
{
	struct timespec	sTime;
	int64			uiMilliseconds;

	//CLOCK_MONOTONIC is not affected by changes to the system clock, like QueryPerformanceCounter.
	clock_gettime(CLOCK_MONOTONIC, &sTime);
	uiMilliseconds = (int64)sTime.tv_sec * 1000 + sTime.tv_nsec / 1000000;
	return uiMilliseconds;
}


#endif // LINUX_GNU
