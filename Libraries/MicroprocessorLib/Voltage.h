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
#ifndef __VOLTAGE_H__
#define __VOLTAGE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"


//Voltages are float32 volts, as in Logicim.  A trace or port that nothing drives has no voltage, which is NaN, as
//Logicim's Float.NaN.
float32	NoVoltage(void);
bool	IsVoltage(float32 fVoltage);
float32	InfiniteVoltsPerTime(void);
bool	IsInfinite(float32 f);
int64	RoundFloat32ToInt64(float32 f);
void	AppendVoltage(CChars* psz, float32 fVoltage, bool bIncludeV = true);


#endif // __VOLTAGE_H__

