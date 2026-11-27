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
#ifndef __POWER_SOURCE_H__
#define __POWER_SOURCE_H__
#include "Passive.h"
#include "PowerOutPort.h"
#include "PowerInPort.h"
#include "PowerPinNames.h"


#define POWER_PIN_FEED		"In"

//How far from 0V a regulator's ground may be and still be its ground.
#define POWER_SOURCE_GROUND_TOLERANCE	0.1f
#define POWER_SOURCE_TYPE	"Power Source"


//Logicim's PowerSource: drives its Power port's trace with a supply's voltage.  It is switched on and off, and ramps
//over its rise time, see CPowerOutPort.  When the simulation starts it is on, or off if it isn't on at start; on, it
//rises from 0V over its rise time (at once with none, as Logicim's).
//
//A supply fed by another, a regulator, has two power in ports: In, on its feed's trace, and GND, on its ground's.  It
//is on while its ground is at 0V and its feed is at its voltage plus its dropout or above it, and off otherwise (a
//regulator with nothing on its In or GND is off), whatever it was told at start.
class CPowerSource : public CPassive
{
CONSTRUCTABLE(CPowerSource);
protected:
	CPowerOutPort*	mpcPowerOutPort;
	bool			mbOnAtStart;
	CPowerInPort*	mpcFeedPort;  //NULL unless it is fed.
	CPowerInPort*	mpcGroundPort;  //NULL unless it is fed.
	float32			mfDropout;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fVoltage);
			void			Class(void) override;

	virtual const char*		GetType(void);
	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual bool			IsPoweringUp(void);

			void			Switch(CTimeline* pcTimeline, bool bOn);
			bool			IsOn(void) const;
			void			SetOnAtStart(bool bOnAtStart);
			bool			IsOnAtStart(void) const;
			void			SetRiseTime(int64 iRiseTime);
			int64			GetRiseTime(void) const;
			void			SetFeed(float32 fDropout);
			CPowerInPort*	GetFeedPort(void) const;
			CPowerInPort*	GetGroundPort(void) const;
			float32			GetDropout(void) const;
			CPowerOutPort*	GetPowerOutPort(void) const;

protected:
			void			FollowFeed(CTimeline* pcTimeline);
};


typedef CArrayTemplatePtr<CPowerSource>	CArrayPowerSourcePtr;


#endif // __POWER_SOURCE_H__

