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
#ifndef __CLOCK_OSCILLATOR_H__
#define __CLOCK_OSCILLATOR_H__
#include "IntegratedCircuit.h"
#include "ClockOscillatorPins.h"


#define CLOCK_OSCILLATOR_TYPE	"Clock"


//Logicim's ClockOscillator: while powered its output toggles every half cycle, starting low, so the first tick, the
//initialisation time after the simulation starts, drives it high.  The initialisation time is half a cycle unless
//given.  A full tick is counted each time the output goes high.  Connecting its power starts it ticking, or stops it
//driving when there is no power.  The output's level and the count of full ticks are what it remembers; they're set
//back by ResetState.
class CClockOscillator : public CIntegratedCircuit
{
CONSTRUCTABLE(CClockOscillator);
protected:
	int64	miHalfCycleTime;
	int64	miInitialisationTime;
	int64	miFullTicks;
	bool	mbHigh;

public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CClockOscillatorPins* pcPins, float32 fFrequency);
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CClockOscillatorPins* pcPins, float32 fFrequency, int64 iInitialisationTime);

	virtual void					SimulationStarted(CTimeline* pcTimeline);
	virtual void					InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual void					ExecuteTick(CTimeline* pcTimeline);
	virtual void					TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual const char*				GetType(void);
	virtual void					ResetState(void);

			float32					GetInternalVoltage(int64 iTime);
			int64					GetFullTicks(void);
			void					SetFullTicks(int64 iFullTicks);
			bool					IsHigh(void);
			void					SetHigh(bool bHigh);
			int64					GetHalfCycleTime(void);
			CClockOscillatorPins*	GetClockOscillatorPins(void);

protected:
			void					CreateTickEvent(CTimeline* pcTimeline, int64 iDelay);
			bool					HasTickEvent(void);
};


#endif // __CLOCK_OSCILLATOR_H__

