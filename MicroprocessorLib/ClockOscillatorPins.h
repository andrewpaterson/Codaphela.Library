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
#ifndef __CLOCK_OSCILLATOR_PINS_H__
#define __CLOCK_OSCILLATOR_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's ClockOscillatorPins: "Output", and "Output2", the inverse, if asked for.
class CClockOscillatorPins : public CPins
{
protected:
	CLogicPort*		mpcOutput;
	CLogicPort*		mpcOutputInverse;

public:
			void		Init(CVoltageConfigurationSource* pcVoltageConfiguration, bool bInverseOut);
	virtual void		Kill(void);

			CLogicPort*	GetOutput(void);
			CLogicPort*	GetOutputInverse(void);
};


#endif // __CLOCK_OSCILLATOR_PINS_H__

