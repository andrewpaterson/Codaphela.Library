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
#include "LongTime.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 TimeToNanoseconds(int64 iTime)
{
	return iTime / (float64)TIME_GRANULARITY_IN_NANOSECOND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 NanosecondsToTime(int64 iNanoseconds)
{
	return iNanoseconds * TIME_GRANULARITY_IN_NANOSECOND;
}


//////////////////////////////////////////////////////////////////////////
//
// Rounds towards zero, as Logicim's cast to long does.
//
//////////////////////////////////////////////////////////////////////////
int64 NanosecondsToTime(float64 fNanoseconds)
{
	return (int64)(fNanoseconds * TIME_GRANULARITY_IN_NANOSECOND);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 SecondsToTime(float64 fSeconds)
{
	return NanosecondsToTime((float64)NANOSECONDS_IN_SECOND * fSeconds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 TimeToSeconds(int64 iTime)
{
	return TimeToNanoseconds(iTime) / (float64)NANOSECONDS_IN_SECOND;
}


//////////////////////////////////////////////////////////////////////////
//
// The period of fFrequency, in hertz.
//
//////////////////////////////////////////////////////////////////////////
int64 FrequencyToTime(float64 fFrequency)
{
	return SecondsToTime(1.0 / fFrequency);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the time in nanoseconds to one decimal place, like "12.5ns".
// The tenths are rounded half away from zero.  Worked out with integers
// so it doesn't need the float printer.
//
//////////////////////////////////////////////////////////////////////////
void AppendNanoseconds(CChars* psz, int64 iTime)
{
	int64	iTenths;

	if (iTime < 0)
	{
		iTenths = (-iTime * 10 + TIME_GRANULARITY_IN_NANOSECOND / 2) / TIME_GRANULARITY_IN_NANOSECOND;
		if (iTenths != 0)
		{
			psz->Append('-');
		}
	}
	else
	{
		iTenths = (iTime * 10 + TIME_GRANULARITY_IN_NANOSECOND / 2) / TIME_GRANULARITY_IN_NANOSECOND;
	}
	psz->Append(iTenths / 10);
	psz->Append('.');
	psz->Append(iTenths % 10);
	psz->Append("ns");
}

