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
#include "Voltage.h"
#include "VoltagePropagationTimeRow.h"


//////////////////////////////////////////////////////////////////////////
//
// Every number starts empty.
//
//////////////////////////////////////////////////////////////////////////
void SVoltagePropagationTimeRow::Init(void)
{
	szFamily.Init();
	szLogicLevel.Init();
	szDelayType.Init();
	szManufacturer.Init();
	fVcc = NoVoltage();

	fTypicalDelay = NoVoltage();
	fMinimumDelay = NoVoltage();
	fMaximumDelay = NoVoltage();

	fVIH = NoVoltage();
	fVIL = NoVoltage();
	fVOH = NoVoltage();
	fVOL = NoVoltage();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SVoltagePropagationTimeRow::Kill(void)
{
	szManufacturer.Kill();
	szDelayType.Kill();
	szLogicLevel.Kill();
	szFamily.Kill();
}

