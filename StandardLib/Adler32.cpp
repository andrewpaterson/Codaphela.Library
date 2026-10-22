/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

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
#include "Adler32.h"


#define ADLER_32_MODULUS	65521

//The largest number of bytes that can be summed before muiB could overflow 32 bits.
#define ADLER_32_MAX_RUN	5552


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAdler32::Init(void)
{
	muiA = 1;
	muiB = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAdler32::Update(uint8* puiData, size uiLength)
{
	size	uiRun;
	size	i;

	while (uiLength > 0)
	{
		uiRun = (uiLength < ADLER_32_MAX_RUN) ? uiLength : ADLER_32_MAX_RUN;
		for (i = 0; i < uiRun; i++)
		{
			muiA += puiData[i];
			muiB += muiA;
		}
		muiA %= ADLER_32_MODULUS;
		muiB %= ADLER_32_MODULUS;
		puiData += uiRun;
		uiLength -= uiRun;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAdler32::Get(void)
{
	return (muiB << 16) | muiA;
}
