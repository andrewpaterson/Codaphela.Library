/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestLib

Codaphela TestLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifdef _MSC_VER
#include "IntelTimer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StartIntelTimer(int* piHigh, int* piLow)
{
	int	iEax1;
	int	iEdx1;

	__asm
	{
		mov ecx, 0x10
		rdtsc
		mov iEax1, eax
		mov iEdx1, edx
	}
	*piHigh = iEdx1;
	*piLow = iEax1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StopIntelTimer(int* piHigh, int* piLow)
{
	int	iEax2;
	int	iEdx2;

	__asm
	{
		mov ecx, 0x10
		rdtsc
		mov iEax2, eax
		mov iEdx2, edx
	}
	*piHigh = iEdx2;
	*piLow = iEax2;
}

#endif // WIN32
