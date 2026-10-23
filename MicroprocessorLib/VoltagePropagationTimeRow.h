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
#ifndef __VOLTAGE_PROPAGATION_TIME_ROW_H__
#define __VOLTAGE_PROPAGATION_TIME_ROW_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"


//Logicim's VoltagePropagationTimeRow: one row of the propagation times file, see CFamilyVoltageConfigurationStore.
//The delay type is tPD, tPHL, tPLH, tEN, tPZH, tPZL, tDIS, tPHZ or tPLZ and the delays are in nanoseconds.  A number
//that is empty in the file is NaN; Logicim has null.
struct SVoltagePropagationTimeRow
{
	CChars		szFamily;
	CChars		szLogicLevel;
	CChars		szDelayType;
	CChars		szManufacturer;
	float32		fVcc;

	float32		fTypicalDelay;
	float32		fMinimumDelay;
	float32		fMaximumDelay;

	float32		fVIH;
	float32		fVIL;
	float32		fVOH;
	float32		fVOL;

	void	Init(void);
	void	Kill(void);
};


typedef CArrayTemplate<SVoltagePropagationTimeRow>		CArrayVoltagePropagationTimeRow;
typedef CArrayTemplatePtr<SVoltagePropagationTimeRow>	CArrayVoltagePropagationTimeRowPtr;


#endif // __VOLTAGE_PROPAGATION_TIME_ROW_H__

