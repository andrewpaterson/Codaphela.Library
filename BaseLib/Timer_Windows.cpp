/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifdef _WIN32
#include "WindowsHeaders.h"


CTimer	gcTimer;


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
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Init(void)
{
	miStart = GetDebugTickCount();
	miStart2 = GetDebugTickCount();
	miTotalTime = 0;
	miUpdateTime = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimer::Update(void)
{
	int64	iTime;

	iTime = GetDebugTickCount();

	//Get the milliseconds passed since start.
	miTotalTime = iTime - miStart;

	//Get the milliseconds passed since last update
	miUpdateTime = iTime - miStart2;
	miStart2 = iTime;

	//Give time in seconds also.
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
		fSeconds = (float)miTotalTime/1000.0f;

		psz->Append(fSeconds, 3);
		psz->Append("s");
	}
}


#endif // WIN32

