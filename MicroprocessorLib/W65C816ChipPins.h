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
#ifndef __W65C816_CHIP_PINS_H__
#define __W65C816_CHIP_PINS_H__
#include "Pins.h"
#include "LogicPort.h"
#include "LogicPortBus.h"


//Logicim's W65C816Pins: the W65C816S's pins as CLogicPorts on analog traces.  "Address 0" up and "Data 0" up are the
//least significant bits first, as buses for the processor's CW65C816Pins.  Logicim reads the data with "Data 0" the
//most significant, the reverse of how it writes it; here both are least significant first.  RDY is bidirectional, as
//the processor pulls it low while waiting.
class CW65C816ChipPins : public CPins
{
CONSTRUCTABLE(CW65C816ChipPins);
protected:
	CLogicPortBus	mcAddress;
	CLogicPortBus	mcData;
	CLogicPort*		mpcVPB;
	CLogicPort*		mpcRDY;
	CLogicPort*		mpcABORTB;
	CLogicPort*		mpcIRQB;
	CLogicPort*		mpcNMIB;
	CLogicPort*		mpcMLB;
	CLogicPort*		mpcVPA;
	CLogicPort*		mpcVDA;
	CLogicPort*		mpcRESB;
	CLogicPort*		mpcMX;
	CLogicPort*		mpcPHI2;
	CLogicPort*		mpcBE;
	CLogicPort*		mpcE;
	CLogicPort*		mpcRWB;

public:
			void			Init(CVoltageConfigurationSource* pcVoltageConfiguration);
			void			Class(void) override;
			void			Free(void) override;

			CLogicPortBus*	GetAddress(void);
			CLogicPortBus*	GetData(void);
			CLogicPort*		GetVPB(void);
			CLogicPort*		GetRDY(void);
			CLogicPort*		GetABORTB(void);
			CLogicPort*		GetIRQB(void);
			CLogicPort*		GetNMIB(void);
			CLogicPort*		GetMLB(void);
			CLogicPort*		GetVPA(void);
			CLogicPort*		GetVDA(void);
			CLogicPort*		GetRESB(void);
			CLogicPort*		GetMX(void);
			CLogicPort*		GetPHI2(void);
			CLogicPort*		GetBE(void);
			CLogicPort*		GetE(void);
			CLogicPort*		GetRWB(void);
};


#endif // __W65C816_CHIP_PINS_H__

