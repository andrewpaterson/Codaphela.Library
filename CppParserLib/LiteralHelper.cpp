/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Reads the integer literal szText (a whole pp-number, uiLength chars):
// its base, value and suffix.  Digit separators are left out.  Suffixes
// are u, l, ll in any order and Microsoft's i8, i16, i32, i64 and ui64.
// Returns ILR_Floating, leaving the rest unset, if it is a floating
// literal.  [lex.icon]
//
//////////////////////////////////////////////////////////////////////////
EIntegerLiteralResult ParseIntegerLiteral(const char* szText, size uiLength, SIntegerLiteral* psLiteral)
{
	char*		szNumber;
	char*		szDigits;
	char*		szEnd;
	char*		sz;
	size		i;
	size		iLength;
	int			iBase;
	char		c;
	bool		bFloating;
	uint64		ulliValue;
	uint64		ulliDigit;

	szNumber = psLiteral->szNumber;
	iLength = 0;
	for (i = 0; (i < uiLength) && (iLength < 255); i++)
	{
		if (szText[i] != '\'')
		{
			szNumber[iLength] = szText[i];
			iLength++;
		}
	}
	szNumber[iLength] = '\0';

	iBase = 10;
	szDigits = szNumber;
	if ((szNumber[0] == '0') && ((szNumber[1] == 'x') || (szNumber[1] == 'X')))
	{
		iBase = 16;
		szDigits = &szNumber[2];
	}
	else if ((szNumber[0] == '0') && ((szNumber[1] == 'b') || (szNumber[1] == 'B')))
	{
		iBase = 2;
		szDigits = &szNumber[2];
	}

	//Find where the digits end and the suffix starts.
	bFloating = false;
	szEnd = szDigits;
	for (;;)
	{
		c = *szEnd;
		if ((c >= '0') && (c <= '9'))
		{
		}
		else if ((iBase == 16) && (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
		{
		}
		else if (c == '.')
		{
			bFloating = true;
		}
		else if ((iBase == 10) && ((c == 'e') || (c == 'E')))
		{
			bFloating = true;
			if ((szEnd[1] == '+') || (szEnd[1] == '-'))
			{
				szEnd++;
			}
		}
		else if ((iBase == 16) && ((c == 'p') || (c == 'P')))
		{
			bFloating = true;
			if ((szEnd[1] == '+') || (szEnd[1] == '-'))
			{
				szEnd++;
			}
		}
		else
		{
			break;
		}
		szEnd++;
	}

	if (bFloating)
	{
		return ILR_Floating;
	}

	if ((iBase == 10) && (szDigits[0] == '0') && (szEnd > szDigits + 1))
	{
		iBase = 8;
		szDigits++;
	}

	ulliValue = 0;
	for (sz = szDigits; sz < szEnd; sz++)
	{
		c = *sz;
		if ((c >= '0') && (c <= '9'))
		{
			ulliDigit = c - '0';
		}
		else if ((c >= 'a') && (c <= 'f'))
		{
			ulliDigit = c - 'a' + 10;
		}
		else
		{
			ulliDigit = c - 'A' + 10;
		}

		if (ulliDigit >= (uint64)iBase)
		{
			return ILR_InvalidDigit;
		}
		if (ulliValue > (0xFFFFFFFFFFFFFFFFULL - ulliDigit) / iBase)
		{
			return ILR_TooLarge;
		}
		ulliValue = ulliValue * iBase + ulliDigit;
	}

	psLiteral->ulliValue = ulliValue;
	psLiteral->iBase = iBase;
	psLiteral->bUnsigned = false;
	psLiteral->iLongs = 0;
	psLiteral->bInt64 = false;
	for (sz = szEnd; *sz != '\0';)
	{
		if ((*sz == 'u') || (*sz == 'U'))
		{
			psLiteral->bUnsigned = true;
			sz++;
		}
		else if ((*sz == 'l') || (*sz == 'L'))
		{
			psLiteral->iLongs++;
			sz++;
		}
		else if (((*sz == 'i') || (*sz == 'I')) && (sz[1] == '6') && (sz[2] == '4'))
		{
			psLiteral->bInt64 = true;
			sz += 3;
		}
		else if (((*sz == 'i') || (*sz == 'I')) && (((sz[1] == '3') && (sz[2] == '2')) || ((sz[1] == '1') && (sz[2] == '6'))))
		{
			sz += 3;
		}
		else if (((*sz == 'i') || (*sz == 'I')) && (sz[1] == '8'))
		{
			sz += 2;
		}
		else
		{
			return ILR_InvalidSuffix;
		}
	}
	if (psLiteral->iLongs > 2)
	{
		return ILR_InvalidSuffix;
	}
	return ILR_Integer;
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes the escape sequence starting at *psz, the character after the
// backslash, reading no further than szEnd.  On success *psz is moved past
// it and *puiValue is its value.  Returns false for an unknown escape or
// a hex escape (x, u or U) with no digits.  [lex.ccon] Table 7
//
//////////////////////////////////////////////////////////////////////////
bool DecodeEscape(const char** psz, const char* szEnd, uint32* puiValue)
{
	const char*	sz;
	uint32		uiValue;
	int			iDigits;
	int			iMaxDigits;
	char		c;

	sz = *psz;
	c = *sz;
	sz++;
	switch (c)
	{
	case 'n':	uiValue = '\n';	break;
	case 't':	uiValue = '\t';	break;
	case 'v':	uiValue = '\v';	break;
	case 'b':	uiValue = '\b';	break;
	case 'r':	uiValue = '\r';	break;
	case 'f':	uiValue = '\f';	break;
	case 'a':	uiValue = '\a';	break;
	case '\\':	uiValue = '\\';	break;
	case '?':	uiValue = '?';	break;
	case '\'':	uiValue = '\'';	break;
	case '"':	uiValue = '"';	break;
	case 'x':
	case 'u':
	case 'U':
		uiValue = 0;
		iMaxDigits = (c == 'u') ? 4 : 8;
		for (iDigits = 0; (sz < szEnd) && (iDigits < iMaxDigits); iDigits++)
		{
			c = *sz;
			if ((c >= '0') && (c <= '9'))
			{
				uiValue = uiValue * 16 + (c - '0');
			}
			else if ((c >= 'a') && (c <= 'f'))
			{
				uiValue = uiValue * 16 + (c - 'a' + 10);
			}
			else if ((c >= 'A') && (c <= 'F'))
			{
				uiValue = uiValue * 16 + (c - 'A' + 10);
			}
			else
			{
				break;
			}
			sz++;
		}
		if (iDigits == 0)
		{
			return false;
		}
		break;
	default:
		if ((c >= '0') && (c <= '7'))
		{
			uiValue = c - '0';
			for (iDigits = 1; (sz < szEnd) && (iDigits < 3) && (*sz >= '0') && (*sz <= '7'); iDigits++)
			{
				uiValue = uiValue * 8 + (*sz - '0');
				sz++;
			}
		}
		else
		{
			return false;
		}
		break;
	}

	*psz = sz;
	*puiValue = uiValue;
	return true;
}

