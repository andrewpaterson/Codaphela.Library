/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "FastFunctions.h"


BOOL gbFastFunctions = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FastFunctionsInit(void)
{
	InitMemcpyFast();
	InitMemcpyFastBackwards();
	InitMemcmpFast();
	InitMemsetFast();
	gbFastFunctions = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FastFunctionsKill(void)
{
	//Does nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void memmove_fast(void* pvDest, void* pvSource, int iByteSize)
{
	if (pvDest < pvSource)
	{
		memcpy_fast(pvDest, pvSource, iByteSize);
	}
	else if (pvDest > pvSource)
	{
		memcpy_fast_backwards(pvDest, pvSource, iByteSize);
	}
}

