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
#ifndef __OSCILLOSCOPE_H__
#define __OSCILLOSCOPE_H__
#include "IntegratedCircuit.h"
#include "OscilloscopePins.h"
#include "OscilloscopeState.h"


#define OSCILLOSCOPE_TYPE	"Oscilloscope"


//Logicim's Oscilloscope: samples its inputs' traces at the sample frequency, numberOfDivsWide * samplesPerDiv samples
//per input in a ring.  Each tick moves on a sample then takes it.  Logicim's view colours are left to a view.
class COscilloscope : public CIntegratedCircuit
{
CONSTRUCTABLE(COscilloscope);
protected:
	float32		mfSampleFrequency;
	int64		miSampleTime;
	size		muiNumberOfDivsWide;
	size		muiSamplesPerDiv;

public:
			void				Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, COscilloscopePins* pcPins, float32 fSampleFrequency, size uiNumberOfDivsWide, size uiSamplesPerDiv);

	virtual void				SimulationStarted(CTimeline* pcTimeline);
	virtual void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual void				ExecuteTick(CTimeline* pcTimeline);
	virtual const char*			GetType(void);
	virtual CState*				CreateState(void);

			int64				GetSampleTime(void);
			COscilloscopePins*	GetOscilloscopePins(void);
			COscilloscopeState*	GetOscilloscopeState(void);
};


#endif // __OSCILLOSCOPE_H__

