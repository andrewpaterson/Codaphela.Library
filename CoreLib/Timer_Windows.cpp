/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Timer.h"
#ifdef WIN32
#include <windows.h>


CTimer	gcTimer;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int GetDebugTickCount()
{
	//return giTime;
	return GetTickCount();
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
	int		iTime;

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
	//Does nothing.
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

