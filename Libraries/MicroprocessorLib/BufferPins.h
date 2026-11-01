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
#ifndef __BUFFER_PINS_H__
#define __BUFFER_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's BufferPins: an input and an output for each buffer, "Input" and "Output" or "Input 0" and "Output 0" up.
class CBufferPins : public CPins
{
CONSTRUCTABLE(CBufferPins);
protected:
	CArrayLogicPortPtr	mapcInputs;
	CArrayLogicPortPtr	mapcOutputs;

public:
			void				Init(size uiBufferCount, CVoltageConfigurationSource* pcVoltageConfiguration);
			void				Class(void) override;
			void				Free(void) override;

			CArrayLogicPortPtr*	GetInputs(void);
			CArrayLogicPortPtr*	GetOutputs(void);
			CLogicPort*			GetInput(size uiIndex);
			CLogicPort*			GetOutput(size uiIndex);
};


#endif // __BUFFER_PINS_H__

