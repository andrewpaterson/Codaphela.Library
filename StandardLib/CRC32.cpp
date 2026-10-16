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
#include "CRC32.h"


#define CRC_32_POLYNOMIAL	0xEDB88320


uint32	gauiCRC32Table[256];
bool	gbCRC32TableBuilt = false;


//////////////////////////////////////////////////////////////////////////
//
// The table is built the first time it is needed.
//
//////////////////////////////////////////////////////////////////////////
void CRC32BuildTable(void)
{
	uint32	uiValue;
	int		i;
	int		j;

	for (i = 0; i < 256; i++)
	{
		uiValue = (uint32)i;
		for (j = 0; j < 8; j++)
		{
			uiValue = (uiValue & 1) ? (CRC_32_POLYNOMIAL ^ (uiValue >> 1)) : (uiValue >> 1);
		}
		gauiCRC32Table[i] = uiValue;
	}
	gbCRC32TableBuilt = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCRC32::Init(void)
{
	if (!gbCRC32TableBuilt)
	{
		CRC32BuildTable();
	}
	muiCRC = 0xFFFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCRC32::Update(uint8* puiData, size uiLength)
{
	size	i;

	for (i = 0; i < uiLength; i++)
	{
		muiCRC = gauiCRC32Table[(muiCRC ^ puiData[i]) & 0xFF] ^ (muiCRC >> 8);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CCRC32::Get(void)
{
	return muiCRC ^ 0xFFFFFFFF;
}
