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
#ifndef __OSCILLOSCOPE_STATE_H__
#define __OSCILLOSCOPE_STATE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "State.h"


//Logicim's OscilloscopeState: a ring of samples per input, the lowest and highest voltage driven on its trace.  A
//disconnected or undriven trace samples 0V.  Logicim also keeps a colour per sample from its view's voltage colours;
//here a view works the colour out from the voltages.
class CTrace;
class COscilloscopeState : public CState
{
CONSTRUCTABLE(COscilloscopeState);
protected:
	size		muiInputCount;
	size		muiSampleCount;
	float32*	mafMinVoltage;  //muiInputCount rows of muiSampleCount.
	float32*	mafMaxVoltage;
	size		muiTickPosition;

public:
			void	Init(size uiInputCount, size uiSampleCount);
	virtual void	Kill(void);

			void	Tick(void);
			void	Sample(size uiInput, CTrace* pcTrace, int64 iTime);

			float32	GetMinVoltage(size uiInput, size uiSample);
			float32	GetMaxVoltage(size uiInput, size uiSample);
			size	GetTickPosition(void);
			size	GetSampleCount(void);
			size	GetInputCount(void);
};


#endif // __OSCILLOSCOPE_STATE_H__

