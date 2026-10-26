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
#ifndef __OR_GATE_PINS_H__
#define __OR_GATE_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's OrGatePins: an "Output" and inputCount inputs, "Input" or "Input 0" up.
class COrGatePins : public CPins
{
CONSTRUCTABLE(COrGatePins);
protected:
	CArrayLogicPortPtr	mapcInputs;
	CLogicPort*			mpcOutput;

public:
			void				Init(size uiInputCount, CVoltageConfigurationSource* pcVoltageConfiguration);
			void				Class(void) override;
			void				Free(void) override;

			CLogicPort*			GetOutput(void);
			CArrayLogicPortPtr*	GetInputs(void);
			CLogicPort*			GetInput(size uiIndex);
};


#endif // __OR_GATE_PINS_H__

