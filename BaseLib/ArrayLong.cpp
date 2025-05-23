/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "DebugOutput.h"
#include "StringHelper.h"
#include "ArrayLong.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayLong::AddList(int64 iStop, ...)
{
	va_list		vaMarker;
	int64		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, int64);
	while (iValue != iStop)
	{
		Add(iValue);
		iValue = va_arg(vaMarker, int64);
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayLong::Dump(void)
{
	uint32	i;
	int64	iValue;
	char	sz[32];


	EngineOutput("[");
	for (i = 0; i < miUsedElements; i++)
	{
		iValue = GetValue(i);
		LongToString(sz, 32, iValue);
		EngineOutput(sz);
		EngineOutput(" ");
	}
	EngineOutput("]\n");
}

