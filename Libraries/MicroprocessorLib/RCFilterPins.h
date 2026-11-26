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
#ifndef __RC_FILTER_PINS_H__
#define __RC_FILTER_PINS_H__
#include "Pins.h"
#include "AnalogInPort.h"


#define RC_FILTER_PIN_IN		"In"
#define RC_FILTER_PIN_OUT		"Out"


//The pins of an RC element (CRCFilter): its input, which follows the voltage the resistor's driven end is going to,
//and its output, the capacitor's end, which it slews along RC curves.  Neither has a logic family: the output's
//voltages are the input's.
class CRCFilterPins : public CPins
{
CONSTRUCTABLE(CRCFilterPins);
protected:
	CAnalogInPort*	mpcIn;
	CLogicPort*		mpcOut;

public:
			void			Init(void);
			void			Class(void) override;
			CAnalogInPort*	GetIn(void) const;
			CLogicPort*		GetOut(void) const;
};


#endif // __RC_FILTER_PINS_H__

