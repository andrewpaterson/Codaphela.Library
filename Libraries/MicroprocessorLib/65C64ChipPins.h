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
#ifndef __65C64_CHIP_PINS_H__
#define __65C64_CHIP_PINS_H__
#include "Pins.h"
#include "LogicPort.h"
#include "LogicPortBus.h"
#include "65C64Pins.h"


//The 65C64's simulator signals, see C65C64Pins, as CLogicPorts on analog traces.  "Address 0" up and "Data 0" up are
//the least significant bits first.  These are not the spreadsheet's pinout.
class C65C64ChipPins : public CPins
{
CONSTRUCTABLE(C65C64ChipPins);
protected:
	CLogicPortBus	mcAddress;
	CLogicPortBus	mcData;
	CLogicPort*		mpcPHI2;
	CLogicPort*		mpcRESB;
	CLogicPort*		mpcRWB;
	CLogicPort*		mpcLowerByte;
	CLogicPort*		mpcUpperByte;
	CLogicPort*		mpcOPV;
	CLogicPort*		mpcVPA;
	CLogicPort*		mpcVDA;
	CLogicPort*		mpcVPB;
	CLogicPort*		mapcEIRQB[NUM_65C64_EDGE_INTERRUPTS];
	CLogicPort*		mpcLIRQB;

public:
			void			Init(CVoltageConfigurationSource* pcVoltageConfiguration);
			void			Class(void) override;
			void			Free(void) override;

			CLogicPortBus*	GetAddress(void);
			CLogicPortBus*	GetData(void);
			CLogicPort*		GetPHI2(void);
			CLogicPort*		GetRESB(void);
			CLogicPort*		GetRWB(void);
			CLogicPort*		GetLowerByte(void);
			CLogicPort*		GetUpperByte(void);
			CLogicPort*		GetOPV(void);
			CLogicPort*		GetVPA(void);
			CLogicPort*		GetVDA(void);
			CLogicPort*		GetVPB(void);
			CLogicPort*		GetEIRQB(size uiInterrupt);
			CLogicPort*		GetLIRQB(void);
};


#endif // __65C64_CHIP_PINS_H__

