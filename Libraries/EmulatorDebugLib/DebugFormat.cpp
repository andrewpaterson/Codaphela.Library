/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <string.h>
#include "DebugFormat.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AppendDebugHex(CChars* psz, uint64 uiValue, int iDigits)
{
	char	sz[32];

	if (iDigits < 1)
	{
		iDigits = 1;
	}
	sprintf(sz, "%0*llX", iDigits, (unsigned long long)uiValue);
	psz->Append(sz);
}


//////////////////////////////////////////////////////////////////////////
//
// A bank's digits are the bits above 16 (at least two).
//
//////////////////////////////////////////////////////////////////////////
void AppendDebugAddress(CChars* psz, uint32 uiAddress, EDebugAddressFormat eFormat, size uiAddressBits)
{
	psz->Append('$');
	if ((eFormat == DAF_Bank) && (uiAddressBits > 16))
	{
		AppendDebugHex(psz, uiAddress >> 16, (int)((uiAddressBits - 16 + 3) / 4) < 2 ? 2 : (int)((uiAddressBits - 16 + 3) / 4));
		psz->Append(':');
		AppendDebugHex(psz, uiAddress & 0xFFFF, 4);
	}
	else
	{
		AppendDebugHex(psz, uiAddress, (int)((uiAddressBits + 3) / 4));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value in as many digits as the register's bits, then a flags
// register's letters: upper case for a set bit, lower for a clear one.
//
//////////////////////////////////////////////////////////////////////////
void AppendDebugRegisterValue(CChars* psz, SDebugRegister* psRegister, uint64 uiValue)
{
	size	uiLetters;
	size	i;
	char	c;
	bool	bSet;

	psz->Append('$');
	AppendDebugHex(psz, uiValue, (int)((psRegister->uiBits + 3) / 4));

	uiLetters = 0;
	while ((uiLetters < DEBUG_REGISTER_FLAGS_SIZE) && (psRegister->szFlags[uiLetters] != '\0'))
	{
		uiLetters++;
	}
	if ((psRegister->uiKind != DRK_Flags) || (uiLetters == 0))
	{
		return;
	}
	psz->Append(' ');
	for (i = 0; i < uiLetters; i++)
	{
		c = psRegister->szFlags[i];
		bSet = ((uiValue >> (uiLetters - 1 - i)) & 1) != 0;
		if ((c >= 'A') && (c <= 'Z') && !bSet)
		{
			c = (char)(c - 'A' + 'a');
		}
		else if ((c >= 'a') && (c <= 'z') && bSet)
		{
			c = (char)(c - 'a' + 'A');
		}
		psz->Append(c);
	}
}
