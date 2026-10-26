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
#ifndef __PROBE_PINS_H__
#define __PROBE_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's ProbePins: an input per bit, "Input 0" up, the least significant first.
class CProbePins : public CPins
{
CONSTRUCTABLE(CProbePins);
protected:
	CArrayLogicPortPtr	mapcInputs;

public:
			void				Init(CVoltageConfigurationSource* pcVoltageConfiguration, size uiBitWidth);
			void				Class(void) override;
			void				Free(void) override;

			CArrayLogicPortPtr*	GetInputs(void);
			CLogicPort*			GetInput(size uiIndex);
};


#endif // __PROBE_PINS_H__

