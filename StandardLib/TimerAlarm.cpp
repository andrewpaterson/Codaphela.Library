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
#include "TimerAlarm.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Init(CAction* pcAction, char* szTimerName, int iTime, BOOL bRepeat)
{
	mpcAction = pcAction;
	miTime = iTime;
	mbRepeat = bRepeat;
	miId = 0;
	mszName.Init(szTimerName);  //Not to be confused with the action name.
	Reset();
	Start();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Reset(void)
{
	miRemainingTime = miTime;
	mbWaitForFirstUpdate = TRUE;
	mbRunning = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Kill(void)
{
	Stop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Start(void)
{
	mbRunning = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Stop(void)
{
	mbRunning = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CTimerAlarm::Update(void)
{
	int				iTimeOver;
	float			fOver;
	STimerData		sTimerData;

	if (!mbRunning)
	{
		return -1.0f;
	}

	if (mbWaitForFirstUpdate)
	{
		mbWaitForFirstUpdate = FALSE;
		return -1.0f;
	}

	miRemainingTime -= gcTimer.miUpdateTime;;
	iTimeOver = miRemainingTime;
	if (miRemainingTime <= 0)
	{
		if (mbRepeat)
		{
			miRemainingTime = miTime + miRemainingTime;
		}
		else
		{
			mbRunning = FALSE;
		}
	}

	fOver = (((float)(-iTimeOver)) / ((float)miTime)); 
	if (fOver >= 0.0f)
	{
		sTimerData.szTimerName = mszName.Text();
		sTimerData.fOversleep = fOver;
		mpcAction->Call(&sTimerData);
	}
	return fOver;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimerAlarm::Pause(int iPauseTime)
{
	miRemainingTime += iPauseTime;
}

