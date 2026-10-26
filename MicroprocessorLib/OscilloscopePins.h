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
#ifndef __OSCILLOSCOPE_PINS_H__
#define __OSCILLOSCOPE_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's OscilloscopePins: an input per channel, "Input" or "Input 0" up.  They have no voltage configuration; the
//oscilloscope samples its traces' voltages rather than reading logic levels.
class COscilloscopePins : public CPins
{
CONSTRUCTABLE(COscilloscopePins);
protected:
	CArrayLogicPortPtr	mapcInputs;

public:
			void				Init(size uiInputCount);
			void				Class(void) override;
			void				Free(void) override;

			CArrayLogicPortPtr*	GetInputs(void);
			CLogicPort*			GetInput(size uiIndex);
};


#endif // __OSCILLOSCOPE_PINS_H__

