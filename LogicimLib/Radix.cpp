/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "Radix.h"


//////////////////////////////////////////////////////////////////////////
//
// The most digits a value of iBitWidth bits takes, at least one.  Logicim
// worked decimal out from 2^(bits - 1), a digit short for some widths (it
// gave one digit for four bits); this uses the largest value, 2^bits - 1.
//
//////////////////////////////////////////////////////////////////////////
int CalculateMaxDigits(ERadix eRadix, int iBitWidth)
{
	int		iMaxDigits;
	uint64	uiMax;

	iMaxDigits = 0;
	if (eRadix == RADIX_Binary)
	{
		iMaxDigits = iBitWidth;
	}
	else if (eRadix == RADIX_Hexadecimal)
	{
		iMaxDigits = (iBitWidth + 3) / 4;
	}
	else if (eRadix == RADIX_Decimal)
	{
		uiMax = (iBitWidth >= 64) ? 0xFFFFFFFFFFFFFFFFULL : ((1ULL << iBitWidth) - 1);
		do
		{
			iMaxDigits++;
			uiMax /= 10;
		}
		while (uiMax != 0);
	}
	if (iMaxDigits < 1)
	{
		iMaxDigits = 1;
	}
	return iMaxDigits;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's RadixHelper.getBinaryChar.
//
//////////////////////////////////////////////////////////////////////////
static char TraceValueChar(STraceValue* psValue)
{
	switch (psValue->eValue)
	{
	case TV_High:
		return '1';
	case TV_Low:
		return '0';
	case TV_Unsettled:
		return 'X';
	default:
		return '.';
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Bit 0 is asValues[0].  Binary is a character per bit, most significant
// first.  Hexadecimal is a digit per four bits (the top one perhaps
// fewer), X for a nybble with an unsettled bit and . for one with an
// undriven bit.  Decimal is the value,
// or iMaxDigits of X (unsettled) or . (undriven) if any bit is.
//
//////////////////////////////////////////////////////////////////////////
void AppendTraceValues(CChars* psz, STraceValue* asValues, int iBitWidth, ERadix eRadix, int iMaxDigits)
{
	int			i;
	int			j;
	int			iNybble;
	bool		bUnsettled;
	bool		bUndriven;
	uint64		uiValue;

	if (eRadix == RADIX_Binary)
	{
		for (i = iBitWidth - 1; i >= 0; i--)
		{
			psz->Append(TraceValueChar(&asValues[i]));
		}
	}
	else if (eRadix == RADIX_Hexadecimal)
	{
		//Logicim dropped the bits of a part nybble at the top; they're written here.
		for (i = ((iBitWidth + 3) / 4) - 1; i >= 0; i--)
		{
			iNybble = 0;
			bUnsettled = false;
			bUndriven = false;
			for (j = 0; (j < 4) && (i * 4 + j < iBitWidth); j++)
			{
				if (asValues[i * 4 + j].eValue == TV_High)
				{
					iNybble |= 1 << j;
				}
				else if (asValues[i * 4 + j].eValue == TV_Undriven)
				{
					bUndriven = true;
				}
				else if (asValues[i * 4 + j].eValue == TV_Unsettled)
				{
					bUnsettled = true;
				}
			}
			if (bUnsettled)
			{
				psz->Append('X');
			}
			else if (bUndriven)
			{
				psz->Append('.');
			}
			else
			{
				psz->Append("0123456789ABCDEF"[iNybble]);
			}
		}
	}
	else if (eRadix == RADIX_Decimal)
	{
		uiValue = 0;
		for (i = 0; i < iBitWidth; i++)
		{
			if (asValues[i].eValue == TV_High)
			{
				uiValue |= 1ULL << i;
			}
			else if ((asValues[i].eValue == TV_Undriven) || (asValues[i].eValue == TV_Unsettled))
			{
				for (j = 0; j < iMaxDigits; j++)
				{
					psz->Append(TraceValueChar(&asValues[i]));
				}
				return;
			}
		}
		psz->Append(uiValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* RadixToString(ERadix eRadix)
{
	switch (eRadix)
	{
	case RADIX_Binary:
		return "binary";
	case RADIX_Decimal:
		return "decimal";
	case RADIX_Hexadecimal:
		return "hexadecimal";
	default:
		return "";
	}
}


//////////////////////////////////////////////////////////////////////////
//
// RADIX_Cannot if it isn't one RadixToString gives.
//
//////////////////////////////////////////////////////////////////////////
ERadix StringToRadix(const char* szRadix)
{
	if (szRadix == NULL)
	{
		return RADIX_Cannot;
	}
	if (StringCompare(szRadix, "binary") == 0)
	{
		return RADIX_Binary;
	}
	if (StringCompare(szRadix, "decimal") == 0)
	{
		return RADIX_Decimal;
	}
	if (StringCompare(szRadix, "hexadecimal") == 0)
	{
		return RADIX_Hexadecimal;
	}
	return RADIX_Cannot;
}
