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
#include "BaseLib/ConstructorCall.h"
#include "PowerPinNames.h"
#include "PowerSource.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerSource::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, float32 fVoltage)
{
	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mpcPowerOutPort = NewMalloc<CPowerOutPort>();
	mpcPowerOutPort->Init(POWER_PIN_POWER, this, fVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPowerSource::GetType(void) { return "Power Source"; }
CPowerOutPort* CPowerSource::GetPowerOutPort(void) { return mpcPowerOutPort; }

