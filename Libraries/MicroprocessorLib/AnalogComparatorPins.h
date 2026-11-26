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
#ifndef __ANALOG_COMPARATOR_PINS_H__
#define __ANALOG_COMPARATOR_PINS_H__
#include "Pins.h"
#include "AnalogComparatorInPort.h"
#include "FixedVoltageConfiguration.h"


#define ANALOG_COMPARATOR_PIN_PLUS	"IN+"
#define ANALOG_COMPARATOR_PIN_MINUS	"IN-"
#define ANALOG_COMPARATOR_PIN_OUT	"OUT"


//The pins of an analog comparator (CAnalogComparator): its two inputs, each read against the other, and its output.
//No logic family has a comparator's output, so it is the MAX9010's own (GetAnalogComparatorVoltageConfiguration).
class CAnalogComparatorPins : public CPins
{
CONSTRUCTABLE(CAnalogComparatorPins);
protected:
	CAnalogComparatorInPort*	mpcPlus;
	CAnalogComparatorInPort*	mpcMinus;
	CLogicPort*					mpcOut;

public:
			void						Init(void);
			void						Class(void) override;
			CAnalogComparatorInPort*	GetPlus(void) const;
			CAnalogComparatorInPort*	GetMinus(void) const;
			CLogicPort*					GetOut(void) const;
};


CVoltageConfigurationSource*	GetAnalogComparatorVoltageConfiguration(void);


#endif // __ANALOG_COMPARATOR_PINS_H__

