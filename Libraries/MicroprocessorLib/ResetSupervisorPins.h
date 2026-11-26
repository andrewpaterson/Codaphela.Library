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
#ifndef __RESET_SUPERVISOR_PINS_H__
#define __RESET_SUPERVISOR_PINS_H__
#include "Pins.h"
#include "LogicPort.h"
#include "FixedVoltageConfiguration.h"


#define RESET_SUPERVISOR_PIN_RESET		"RST"
#define RESET_SUPERVISOR_PULL_UP		5500.0f


//The pins of a reset supervisor (CResetSupervisor): its active low reset, an open drain output with an internal
//pull-up (a DS1813's is 5.5k), which it also reads, as a reset button pulls it low.  No logic family has a DS1813's
//levels from 1.2V to 5.5V, so its reset is the DS1813's own (GetResetSupervisorVoltageConfiguration): low in at
//0.4V, high in at 0.7 of 5V, low out at 0.4V, any VCC above zero powering it.
class CResetSupervisorPins : public CPins
{
CONSTRUCTABLE(CResetSupervisorPins);
protected:
	CLogicPort*		mpcReset;

public:
			void			Init(void);
			void			Class(void) override;

			CLogicPort*		GetReset(void) const;
};


CVoltageConfigurationSource*	GetResetSupervisorVoltageConfiguration(void);


#endif // __RESET_SUPERVISOR_PINS_H__

