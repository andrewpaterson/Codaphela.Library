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
// Its feed has stepped, or been connected or cut, or its own port has
// stepped: a fed supply follows its feed.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::TraceConnected(CTimeline* pcTimeline, CPort* pcPort)
{
	FollowFeed(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// On while the feed is at the supply's voltage plus its dropout or above.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::FollowFeed(CTimeline* pcTimeline)
{
	float32	fFeed;
	bool	bOn;

	if (mpcFeedPort == NULL)
	{
		return;
	}
	fFeed = mpcFeedPort->GetVoltageIn(pcTimeline->GetTime());
	bOn = IsVoltage(fFeed) && (fFeed >= mpcPowerOutPort->GetVoltage() + mfDropout);
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
// Makes it a regulator, fed through its In port, which is then connected
// to its feed's trace.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::SetFeed(float32 fDropout)
{
	if (mpcFeedPort == NULL)
	{
		mpcFeedPort = NewMalloc<CPowerInPort>();
		mpcFeedPort->Init(PT_PowerIn, POWER_PIN_FEED, this);
	}
	mfDropout = fDropout;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPowerSource::GetType(void) { return "Power Source"; }
CPowerOutPort* CPowerSource::GetPowerOutPort(void) const { return mpcPowerOutPort; }
bool CPowerSource::IsOn(void) const { return mpcPowerOutPort->IsOn(); }
void CPowerSource::SetOnAtStart(bool bOnAtStart) { mbOnAtStart = bOnAtStart; }
bool CPowerSource::IsOnAtStart(void) const { return mbOnAtStart; }
void CPowerSource::SetRiseTime(int64 iRiseTime) { mpcPowerOutPort->SetRiseTime(iRiseTime); }
int64 CPowerSource::GetRiseTime(void) const { return mpcPowerOutPort->GetRiseTime(); }
CPowerInPort* CPowerSource::GetFeedPort(void) const { return mpcFeedPort; }
float32 CPowerSource::GetDropout(void) const { return mfDropout; }

