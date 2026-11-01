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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 NoVoltage(void)
{
	uint32	ui;

	ui = 0x7fc00000;
	return *((float32*)((void*)&ui));
}


//////////////////////////////////////////////////////////////////////////
//
// False for NaN, whatever its bits.  The bits are tested because
// fVoltage == fVoltage is folded to true under /fp:fast.
//
//////////////////////////////////////////////////////////////////////////
bool IsVoltage(float32 fVoltage)
{
	uint32	ui;

	ui = *((uint32*)((void*)&fVoltage));
	return ((ui & 0x7f800000) != 0x7f800000) || ((ui & 0x007fffff) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's Float.POSITIVE_INFINITY, the slew rate when there is no power.
//
//////////////////////////////////////////////////////////////////////////
float32 InfiniteVoltsPerTime(void)
{
	uint32	ui;

	ui = 0x7f800000;
	return *((float32*)((void*)&ui));
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's Float.isInfinite.
//
//////////////////////////////////////////////////////////////////////////
bool IsInfinite(float32 f)
{
	return (f == InfiniteVoltsPerTime()) || (f == -InfiniteVoltsPerTime());
}


//////////////////////////////////////////////////////////////////////////
//
// Java's Math.round: half rounds up, towards positive infinity.
//
//////////////////////////////////////////////////////////////////////////
int64 RoundFloat32ToInt64(float32 f)
{
	float64	fHalfUp;
	int64	i;

	fHalfUp = (float64)f + 0.5;
	i = (int64)fHalfUp;
	if (fHalfUp < (float64)i)
	{
		i--;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Like Logicim's Voltage.toVoltageString, "3.3V" to one decimal place, or
// "----" when there is no voltage.  Without the V they are "3.3" and
// "---".  The tenths are rounded half away from zero and worked out with
// integers so it doesn't need the float printer.  Multiplying in float32
// rounds 0.35f up to 3.5 tenths so it prints "0.4" as Java does.
//
//////////////////////////////////////////////////////////////////////////
void AppendVoltage(CChars* psz, float32 fVoltage, bool bIncludeV)
{
	float32	fTenths;
	int64	iTenths;

	if (!IsVoltage(fVoltage))
	{
		if (bIncludeV)
		{
			psz->Append("----");
		}
		else
		{
			psz->Append("---");
		}
		return;
	}

	fTenths = fVoltage * 10.0f;
	if (fTenths < 0)
	{
		iTenths = (int64)(-(float64)fTenths + 0.5);
		if (iTenths != 0)
		{
			psz->Append('-');
		}
	}
	else
	{
		iTenths = (int64)((float64)fTenths + 0.5);
	}
	psz->Append(iTenths / 10);
	psz->Append('.');
	psz->Append(iTenths % 10);
	if (bIncludeV)
	{
		psz->Append('V');
	}
}

