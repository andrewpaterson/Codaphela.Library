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
#ifdef _WIN32
#include "Timer.h"
#include "WindowsHeaders.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 GetDebugTickCount()
{
	LARGE_INTEGER	freq;
	LARGE_INTEGER	counter;
	int64			uiMilliseconds;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	uiMilliseconds = (counter.QuadPart * 1000) / freq.QuadPart;
	return uiMilliseconds;
}


//////////////////////////////////////////////////////////////////////////
//
// The counter's frequency doesn't change while the system runs, so it is
// asked for once.
//
//////////////////////////////////////////////////////////////////////////
int64 GetDebugMicroseconds()
{
	static int64	lliFrequency = 0;
	LARGE_INTEGER	freq;
	LARGE_INTEGER	counter;

	if (lliFrequency == 0)
	{
		QueryPerformanceFrequency(&freq);
		lliFrequency = freq.QuadPart;
	}
	QueryPerformanceCounter(&counter);
	return (counter.QuadPart / lliFrequency) * 1000000 + ((counter.QuadPart % lliFrequency) * 1000000) / lliFrequency;
}


#endif // WIN32

