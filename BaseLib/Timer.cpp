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
#include "Timer.h"


//The clock (GetDebugTickCount) is each platform's: Timer_Windows.cpp, Timer_Linux.cpp and Timer_65C64.cpp.
CTimer	gcTimer;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Init(void)
{
	miStart = GetDebugTickCount();
	miStart2 = miStart;
	miTotalTime = 0;
	miUpdateTime = 0;
	mfTotalTime = 0.0f;
	mfUpdateTime = 0.0f;
	mfUpdatesPerSecond = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Update(void)
{
	int64	iTime;

	iTime = GetDebugTickCount();

	//The milliseconds since the start and since the last update.
	miTotalTime = iTime - miStart;
	miUpdateTime = iTime - miStart2;
	miStart2 = iTime;

	//And in seconds.
	mfTotalTime = (float)miTotalTime * (1.0f / 1000.0f);
	mfUpdateTime = (float)miUpdateTime * (1.0f / 1000.0f);
	if (mfUpdateTime != 0.0f)
	{
		mfUpdatesPerSecond = 1.0f / mfUpdateTime;
	}
	else
	{
		mfUpdatesPerSecond = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Reset(void)
{
	miStart = GetDebugTickCount();
	miTotalTime = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::HumanReadable(CChars* psz)
{
	float	fSeconds;

	if (miTotalTime < 1000)
	{
		psz->Append(miTotalTime);
		psz->Append("ms");
	}
	else
	{
		fSeconds = (float)miTotalTime / 1000.0f;
		psz->Append(fSeconds, 3);
		psz->Append("s");
	}
}

