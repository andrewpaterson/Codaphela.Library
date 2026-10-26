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
#ifndef __COUNTER_PINS_H__
#define __COUNTER_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's CounterPins: inputs "Clock", "Data 0" up, the active low "Reset" and "Load", "Enable" and
//"Enable Terminal"; outputs "Count 0" up and "Terminal Count".  Bit 0 is the least significant of both the data and
//the count; Logicim reads the data with "Data 0" the most significant, the reverse of the count, which looks like a
//mistake.
class CCounterPins : public CPins
{
CONSTRUCTABLE(CCounterPins);
protected:
	CLogicPort*			mpcClock;
	CLogicPort*			mpcReset;
	CLogicPort*			mpcLoad;
	CArrayLogicPortPtr	mapcData;
	CLogicPort*			mpcCountEnable;
	CLogicPort*			mpcCountEnableTerminal;
	CArrayLogicPortPtr	mapcOutput;
	CLogicPort*			mpcTerminalOutput;

public:
			void				Init(CVoltageConfigurationSource* pcVoltageConfiguration, size uiBitWidth);
			void				Class(void) override;
			void				Free(void) override;

			bool				IsClock(CLogicPort* pcPort);
			uint32				ReadData(CTimeline* pcTimeline, size uiBitWidth);
			void				WriteOutput(CTimeline* pcTimeline, uint32 uiData, size uiBitWidth);

			CLogicPort*			GetClock(void);
			CLogicPort*			GetReset(void);
			CLogicPort*			GetLoad(void);
			CArrayLogicPortPtr*	GetData(void);
			CLogicPort*			GetCountEnable(void);
			CLogicPort*			GetCountEnableTerminal(void);
			CArrayLogicPortPtr*	GetOutput(void);
			CLogicPort*			GetTerminalOutput(void);
};


#endif // __COUNTER_PINS_H__

