/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "65TraceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void S65TraceValue::Init(void)
{
	eValue = TV65_Undriven;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void S65TraceValue::Init(E65TraceValue eTraceValue)
{
	eValue = eTraceValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void S65TraceValue::Init(bool bValue)
{
	eValue = bValue ? TV65_High : TV65_Low;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool S65TraceValue::IsHigh(void)
{
	return eValue == TV65_High;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool S65TraceValue::IsLow(void)
{
	return eValue == TV65_Low;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void S65TraceValue::Invert(void)
{
	if (eValue == TV65_High)
	{
		eValue = TV65_Low;
	}
	else if (eValue == TV65_Low)
	{
		eValue = TV65_High;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void S65TraceValue::Print(CChars* psz)
{
	switch (eValue)
	{
	case TV65_Low:
		psz->Append("0");
		break;
	case TV65_High:
		psz->Append("1");
		break;
	case TV65_Unsettled:
		psz->Append("~");
		break;
	default:
		psz->Append(".");
		break;
	}
}

