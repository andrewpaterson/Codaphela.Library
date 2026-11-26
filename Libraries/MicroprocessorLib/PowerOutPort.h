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
#ifndef __POWER_OUT_PORT_H__
#define __POWER_OUT_PORT_H__
#include "Port.h"


#define POWER_STEP_VOLTAGE	0.1f


//Logicim's PowerOutPort: drives its trace with a supply's voltage.  Logicim's is fixed and there from the moment it is
//connected.  Here a supply is switched on and off: switched on it rises from where it is to its voltage over its rise
//time, and switched off falls to 0V the same way, a regulator charging and its load discharging the rail's
//capacitors.  It moves in steps of POWER_STEP_VOLTAGE, each a CPowerStepEvent that tells the trace's other ports
//(CPort::SupplyChanged), so a chip powered from it starts, writes again at its new VCC or stops
//(CIntegratedCircuit::PowerChanged), and its holder (CPortHolder::TraceConnected).  With no rise time it switches at
//once, the next tick.
//
//It is on from before the simulation starts, at its voltage, until Start (as the simulation starts) or Switch says
//otherwise.  Its voltage at a time is worked out from when it was last switched and the voltage it had then.
class CPowerStepEvent;
class CPowerOutPort : public CPort
{
CONSTRUCTABLE(CPowerOutPort);
protected:
	float32				mfVoltage;  //When on.
	int64				miRiseTime;  //From 0V to mfVoltage; 0 switches at once.
	bool				mbOn;
	int64				miSwitchTime;
	float32				mfSwitchVoltage;  //The voltage when it was last switched.
	float32				mfToldVoltage;  //The voltage the trace's ports were last told of.
	CPowerStepEvent*	mpcStepEvent;

public:
			void		Init(const char* szName, CPortHolder* pcHolder, float32 fVoltage);

	virtual void		Reset(void);
	virtual float32		GetVoltageOut(int64 iTime);
			void		Start(CTimeline* pcTimeline, bool bOn);
			void		Switch(CTimeline* pcTimeline, bool bOn);
			void		SetSwitched(CTimeline* pcTimeline, bool bOn, int64 iSwitchTime, float32 fSwitchVoltage);
			void		Step(CTimeline* pcTimeline);
			void		StepNextTick(CTimeline* pcTimeline);
			void		StepRemoved(CPowerStepEvent* pcEvent);

			float32		GetVoltage(void) const;
			void		SetRiseTime(int64 iRiseTime);
			int64		GetRiseTime(void) const;
			bool		IsOn(void) const;
			int64		GetSwitchTime(void) const;
			float32		GetSwitchVoltage(void) const;
			int64		CalculateStepTime(void) const;
	virtual void		AppendDescription(CChars* psz);
	virtual void		AppendDebugString(CChars* psz);

protected:
			void		SetSwitch(bool bOn, int64 iSwitchTime, float32 fSwitchVoltage);
			void		RemoveStep(CTimeline* pcTimeline);
			void		ScheduleNextStep(CTimeline* pcTimeline);
			bool		HasArrived(float32 fVoltage) const;
};


#endif // __POWER_OUT_PORT_H__

