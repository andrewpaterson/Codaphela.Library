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
#ifndef __LONG_TIME_H__
#define __LONG_TIME_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"


//Simulation time is a whole number of 1/1024ths of a nanosecond, as in Logicim's LongTime, so that times like a
//clock's half period add up exactly.
#define TIME_GRANULARITY_IN_NANOSECOND	1024
#define NANOSECONDS_IN_SECOND			1000000000LL


float64	TimeToNanoseconds(int64 iTime);
int64	NanosecondsToTime(int64 iNanoseconds);
int64	NanosecondsToTime(float64 fNanoseconds);
int64	SecondsToTime(float64 fSeconds);
float64	TimeToSeconds(int64 iTime);
int64	FrequencyToTime(float64 fFrequency);
void	AppendNanoseconds(CChars* psz, int64 iTime);


#endif // __LONG_TIME_H__

