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
#ifndef __D_TYPE_FLIP_FLOP_PINS_H__
#define __D_TYPE_FLIP_FLOP_PINS_H__
#include "Pins.h"
#include "LogicPort.h"


//Logicim's DTypeFlipFlopPins: outputs "Q" and "QB", inputs "CP" (clock), "D", and the active low "SD" (set) and "RD"
//(reset).  Logicim always makes every port; its inverse out and set reset options are ignored, as here.
class CDTypeFlipFlopPins : public CPins
{
protected:
	CLogicPort*		mpcOutputQ;
	CLogicPort*		mpcOutputQB;
	CLogicPort*		mpcClock;
	CLogicPort*		mpcData;
	CLogicPort*		mpcSet;
	CLogicPort*		mpcReset;

public:
			void		Init(CVoltageConfigurationSource* pcVoltageConfiguration);
	virtual void		Kill(void);

			bool		IsClock(CLogicPort* pcPort);
			CLogicPort*	GetOutputQ(void);
			CLogicPort*	GetOutputQB(void);
			CLogicPort*	GetClock(void);
			CLogicPort*	GetData(void);
			CLogicPort*	GetSet(void);
			CLogicPort*	GetReset(void);
};


#endif // __D_TYPE_FLIP_FLOP_PINS_H__

