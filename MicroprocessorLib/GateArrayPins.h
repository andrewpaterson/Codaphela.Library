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
#ifndef __GATE_ARRAY_PINS_H__
#define __GATE_ARRAY_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//The pins of an array of gates that each have the same number of inputs, named as 74 series parts name them: gate 1's
//inputs are "1A", "1B" and so on and its output is "1Y".  The inputs are kept gate by gate.
class CGateArrayPins : public CPins
{
CONSTRUCTABLE(CGateArrayPins);
protected:
	size				muiGateCount;
	size				muiInputCount;
	CArrayLogicPortPtr	mapcInputs;
	CArrayLogicPortPtr	mapcOutputs;

public:
			void				Init(size uiGateCount, size uiInputCount, CVoltageConfigurationSource* pcVoltageConfiguration);
			void				Class(void) override;
			void				Free(void) override;

			size				GetGateCount(void);
			size				GetInputCount(void);
			CLogicPort*			GetInput(size uiGate, size uiInput);
			CLogicPort*			GetOutput(size uiGate);
			size				FindGate(CLogicPort* pcInput);

	static	void				GetInputName(CChars* psz, size uiGate, size uiInput);
	static	void				GetOutputName(CChars* psz, size uiGate);
};


#endif // __GATE_ARRAY_PINS_H__

