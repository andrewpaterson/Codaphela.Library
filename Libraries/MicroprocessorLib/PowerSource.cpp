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
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "Voltage.h"
#include "Timeline.h"
#include "PowerPinNames.h"
#include "PowerSource.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fVoltage)
{
	PreInit();
	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mpcPowerOutPort = NewMalloc<CPowerOutPort>();
	mpcPowerOutPort->Init(POWER_PIN_POWER, this, fVoltage);
	mbOnAtStart = true;
	mpcFeedPort = NULL;
	mpcGroundPort = NULL;
	mfDropout = 0.0f;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::Class(void)
{
	CPassive::Class();

	U_Pointer(mpcPowerOutPort);
	U_Bool(mbOnAtStart);
	U_Pointer(mpcFeedPort);
	U_Pointer(mpcGroundPort);
	U_Float32(mfDropout);
}


//////////////////////////////////////////////////////////////////////////
//
// On (rising over its rise time) or off, as it is at start.  A fed supply
// starts off and follows its feed from the next tick, once every supply
// has started.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::SimulationStarted(CTimeline* pcTimeline)
{
	if (mpcFeedPort == NULL)
	{
		mpcPowerOutPort->Start(pcTimeline, mbOnAtStart);
	}
	else
	{
		mpcPowerOutPort->Start(pcTimeline, false);
		mpcPowerOutPort->StepNextTick(pcTimeline);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Its feed or ground has stepped, or been connected or cut, or its own
// port has stepped: a fed supply follows its feed.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	FollowFeed(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// While it has steps to take: ramping, or a fed supply about to look at
// its feed.
//
//////////////////////////////////////////////////////////////////////////
bool CPowerSource::IsPoweringUp(void)
{
	return mpcPowerOutPort->HasStep();
}


//////////////////////////////////////////////////////////////////////////
//
// On while the ground is at 0V and the feed is at the supply's voltage
// plus its dropout or above.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::FollowFeed(CTimeline* pcTimeline)
{
	float32	fFeed;
	float32	fGround;
	bool	bOn;

	if (mpcFeedPort == NULL)
	{
		return;
	}
	fFeed = mpcFeedPort->GetVoltageIn(pcTimeline->GetTime());
	fGround = mpcGroundPort->GetVoltageIn(pcTimeline->GetTime());
	bOn = IsVoltage(fFeed) && IsVoltage(fGround) && (fabsf(fGround) <= POWER_SOURCE_GROUND_TOLERANCE) && (fFeed >= mpcPowerOutPort->GetVoltage() + mfDropout);
	if (bOn != mpcPowerOutPort->IsOn())
	{
		mpcPowerOutPort->Switch(pcTimeline, bOn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Switches it on (rising over its rise time) or off (falling).
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::Switch(CTimeline* pcTimeline, bool bOn)
{
	mpcPowerOutPort->Switch(pcTimeline, bOn);
}


//////////////////////////////////////////////////////////////////////////
//
// Makes it a regulator, fed through its In port and grounded through its
// GND port, which are then connected to its feed's and ground's traces.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::SetFeed(float32 fDropout)
{
	if (mpcFeedPort == NULL)
	{
		mpcFeedPort = NewMalloc<CPowerInPort>();
		mpcFeedPort->Init(PT_PowerIn, POWER_PIN_FEED, this);
		mpcGroundPort = NewMalloc<CPowerInPort>();
		mpcGroundPort->Init(PT_PowerIn, POWER_PIN_GND, this);
	}
	mfDropout = fDropout;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPowerSource::GetType(void) { return POWER_SOURCE_TYPE; }
CPowerOutPort* CPowerSource::GetPowerOutPort(void) const { return mpcPowerOutPort; }
bool CPowerSource::IsOn(void) const { return mpcPowerOutPort->IsOn(); }
void CPowerSource::SetOnAtStart(bool bOnAtStart) { mbOnAtStart = bOnAtStart; }
bool CPowerSource::IsOnAtStart(void) const { return mbOnAtStart; }
void CPowerSource::SetRiseTime(int64 iRiseTime) { mpcPowerOutPort->SetRiseTime(iRiseTime); }
int64 CPowerSource::GetRiseTime(void) const { return mpcPowerOutPort->GetRiseTime(); }
CPowerInPort* CPowerSource::GetFeedPort(void) const { return mpcFeedPort; }
CPowerInPort* CPowerSource::GetGroundPort(void) const { return mpcGroundPort; }
float32 CPowerSource::GetDropout(void) const { return mfDropout; }

