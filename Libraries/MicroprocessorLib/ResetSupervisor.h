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
#ifndef __RESET_SUPERVISOR_H__
#define __RESET_SUPERVISOR_H__
#include "IntegratedCircuit.h"
#include "ResetSupervisorPins.h"


#define RESET_SUPERVISOR_TYPE			"Reset Supervisor"
#define RESET_SUPERVISOR_TRIP_VOLTAGE	4.35f  //A DS1813-10's, typical.
#define RESET_SUPERVISOR_RESET_TIME		150000000  //Nanoseconds: 150ms, a DS1813's typical.


//A power-on reset supervisor, as a DS1813: it holds its active low reset (an open drain output with a pull-up, see
//CResetSupervisorPins) low while its VCC is under its trip voltage and for its reset time after VCC passes it, so the
//chips it resets start once their supply is good.  Pulled low from outside, as by a reset button or another chip, it
//holds reset low for its reset time after the pull ends.  The reset time can be set shorter than the datasheet's,
//since simulating 150ms of a computer's clocks is millions of edges.  While it holds the reset from power on it is
//powering up (IsPoweringUp).  Whether it is holding reset, and why, is what it remembers.
class CResetSupervisor : public CIntegratedCircuit
{
CONSTRUCTABLE(CResetSupervisor);
protected:
	float32		mfTripVoltage;
	int64		miResetTime;
	bool		mbAsserting;
	bool		mbPulledLow;  //Pulled low from outside, waiting for the pull to end.
	bool		mbPowerOnReset;  //The reset it is holding is the one from power on.

public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CResetSupervisorPins> pPins);
			void					Class(void) override;

	virtual void					SimulationStarted(CTimeline* pcTimeline);
	virtual void					WriteOutputs(CTimeline* pcTimeline);
	virtual void					VccChanged(CTimeline* pcTimeline, float32 fOldVcc);
	virtual void					InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual void					ExecuteTick(CTimeline* pcTimeline);
	virtual void					ResetState(void);
	virtual bool					IsPoweringUp(void);
	virtual const char*				GetType(void);
	virtual const char*				GetStateXMLName(void) const;
	virtual void					SaveXML(CXMLWriter* pcWriter);
	virtual bool					LoadXML(CMarkupTag* pcTag);

			void					SetTripVoltage(float32 fTripVoltage);
			float32					GetTripVoltage(void) const;
			void					SetResetTime(int64 iResetTime);
			int64					GetResetTime(void) const;
			bool					IsAsserting(void) const;
			CResetSupervisorPins*	GetResetSupervisorPins(void);

protected:
			void					Assert(CTimeline* pcTimeline, bool bPowerOn);
			void					ScheduleRelease(CTimeline* pcTimeline);
};


#endif // __RESET_SUPERVISOR_H__

