/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Whether sz (all of it) is an integer suffix: u and l, L, ll or LL, each
// at most once, in either order; or Microsoft's i8, i16, i32 or i64 after
// an optional u.  Sets psLiteral's bUnsigned, iLongs and bInt64.
//
//////////////////////////////////////////////////////////////////////////
static bool ParseIntegerSuffix(const char* sz, SIntegerLiteral* psLiteral)
{
	bool	bLongs;

	psLiteral->bUnsigned = false;
	psLiteral->iLongs = 0;
	psLiteral->bInt64 = false;
	bLongs = false;
	for (;;)
	{
		if (((*sz == 'u') || (*sz == 'U')) && !psLiteral->bUnsigned)
		{
			psLiteral->bUnsigned = true;
			sz++;
		}
		else if (((*sz == 'l') || (*sz == 'L')) && !bLongs)
		{
			//ll and LL, not lL.
			bLongs = true;
			psLiteral->iLongs = (sz[1] == sz[0]) ? 2 : 1;
			sz += psLiteral->iLongs;
		}
		else
		{
			break;
		}
	}
	if (*sz == '\0')
	{
		return true;
	}
	if (bLongs || ((*sz != 'i') && (*sz != 'I')))
	{
		return false;
	}

	//Microsoft's: [u]i8, [u]i16, [u]i32 or [u]i64, last.
	sz++;
	if ((sz[0] == '6') && (sz[1] == '4') && (sz[2] == '\0'))
	{
		psLiteral->bInt64 = true;
		return true;
	}
	return ((sz[0] == '8') && (sz[1] == '\0')) || ((((sz[0] == '1') && (sz[1] == '6')) || ((sz[0] == '3') && (sz[1] == '2'))) && (sz[2] == '\0'));
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the integer literal szText (a whole pp-number, uiLength chars):
// its base, value and suffix.  Digit separators are left out.  Suffixes
// are u and l or ll (either case, each once, in either order) and
// Microsoft's i8, i16, i32 and i64 after an optional u.  Returns
// ILR_Floating, leaving the rest unset, if it is a floating literal; one
// longer than szNumber holds is ILR_TooLarge.  [lex.icon]
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
	for (i = 0; i < uiLength; i++)
	{
		if (szText[i] != '\'')
		{
			if (iLength == sizeof(psLiteral->szNumber) - 1)
			{
				return ILR_TooLarge;
			}
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
	if ((iBase != 10) && (szEnd == szDigits))
	{
		return ILR_NoDigits;
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
	return ParseIntegerSuffix(szEnd, psLiteral) ? ILR_Integer : ILR_InvalidSuffix;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the character literal at szText (its prefix, if any, and quotes;
// no more than uiLength characters): its encoding, value and length.  The
// code and #if both read literals with it, so they agree.  [lex.ccon]
//
//////////////////////////////////////////////////////////////////////////
ECharacterLiteralResult ParseCharacterLiteral(const char* szText, size uiLength, SCharacterLiteral* psLiteral)
{
	const char*	sz;
	const char*	szEnd;
	uint32		uiCharacter;

	sz = szText;
	szEnd = szText + uiLength;
	psLiteral->eEncoding = CTE_Narrow;
	if ((sz[0] == 'u') && (sz[1] == '8'))
	{
		psLiteral->eEncoding = CTE_UTF8;
		sz += 2;
	}
	else if (sz[0] == 'L')
	{
		psLiteral->eEncoding = CTE_Wide;
		sz++;
	}
	else if (sz[0] == 'u')
	{
		psLiteral->eEncoding = CTE_UTF16;
		sz++;
	}
	else if (sz[0] == 'U')
	{
		psLiteral->eEncoding = CTE_UTF32;
		sz++;
	}
	sz++;  //The opening quote.

	psLiteral->uiValue = 0;
	psLiteral->iCount = 0;
	for (;;)
	{
		if ((sz >= szEnd) || (*sz == '\0') || (*sz == '\n'))
		{
			return CLR_Unterminated;
		}
		if (*sz == '\'')
		{
			break;
		}
		if (*sz == '\\')
		{
			sz++;
			if (!DecodeEscape(&sz, szEnd, &uiCharacter))
			{
				return CLR_InvalidEscape;
			}
		}
		else
		{
			uiCharacter = (unsigned char)*sz;
			sz++;
		}
		psLiteral->uiValue = (psLiteral->iCount == 0) ? uiCharacter : ((psLiteral->uiValue << 8) | (uiCharacter & 0xFF));
		psLiteral->iCount++;
	}
	psLiteral->uiLength = (size)(sz + 1 - szText);
	return (psLiteral->iCount == 0) ? CLR_Empty : CLR_Character;
}


//////////////////////////////////////////////////////////////////////////
//
// A character literal's value as its type has it: one narrow (or u8)
// character is a char (signed), L and u are 16 bits unsigned (wchar_t is
// 16 bits, as on Windows), U is 32 bits unsigned, and several characters
// are an int.
//
//////////////////////////////////////////////////////////////////////////
int64 CharacterLiteralValue(SCharacterLiteral* psLiteral)
{
	if (psLiteral->iCount == 1)
	{
		switch (psLiteral->eEncoding)
		{
		case CTE_Narrow:
		case CTE_UTF8:
			return (int8)psLiteral->uiValue;
		case CTE_Wide:
		case CTE_UTF16:
			return (uint16)psLiteral->uiValue;
		case CTE_UTF32:
			return (uint32)psLiteral->uiValue;
		}
	}
	return (psLiteral->eEncoding == CTE_UTF32) ? (int64)psLiteral->uiValue : (int64)(int32)psLiteral->uiValue;
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

