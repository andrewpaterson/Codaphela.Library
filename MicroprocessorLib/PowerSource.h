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
#ifndef __POWER_SOURCE_H__
#define __POWER_SOURCE_H__
#include "Passive.h"
#include "PowerOutPort.h"


//Logicim's PowerSource: drives its Power port's trace with a fixed voltage.
class CPowerSource : public CPassive
{
CONSTRUCTABLE(CPowerSource);
protected:
	CPowerOutPort*	mpcPowerOutPort;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fVoltage);
			void			Class(void) override;

	virtual const char*		GetType(void);
			CPowerOutPort*	GetPowerOutPort(void);
};


#endif // __POWER_SOURCE_H__

