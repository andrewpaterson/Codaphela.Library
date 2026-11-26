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
#include "LongTime.h"
#include "ResetSupervisorPins.h"


//The DS1813's levels, made when first asked for.  It holds no memory, so it is never killed.
CFixedVoltageConfiguration	gcResetSupervisorVoltageConfiguration;
bool						gbResetSupervisorVoltageConfiguration = false;


//////////////////////////////////////////////////////////////////////////
//
// A DS1813's reset: low in at 0.4V, high in at 0.7 x 5V, low out at 0.4V
// (at 10mA), 10ns to switch.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfigurationSource* GetResetSupervisorVoltageConfiguration(void)
{
	if (!gbResetSupervisorVoltageConfiguration)
	{
		gcResetSupervisorVoltageConfiguration.Init(5.0f, 0.4f, 3.5f, 0.4f, 5.0f, NanosecondsToTime((int64)10), NanosecondsToTime((int64)10));
		gbResetSupervisorVoltageConfiguration = true;
	}
	return &gcResetSupervisorVoltageConfiguration;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisorPins::Init(void)
{
	PreInit();
	CPins::Init();
	mpcReset = NewMalloc<CLogicPort>();
	mpcReset->Init(PT_Bidirectional, this, RESET_SUPERVISOR_PIN_RESET, GetResetSupervisorVoltageConfiguration());
	mpcReset->SetPullUp(RESET_SUPERVISOR_PULL_UP);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CResetSupervisorPins::Class(void)
{
	CPins::Class();
	U_Pointer(mpcReset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogicPort* CResetSupervisorPins::GetReset(void) const { return mpcReset; }

