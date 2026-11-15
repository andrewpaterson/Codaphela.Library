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
#include <string.h>
#include "CPPNameCharacters.h"
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Whether sz (all of it) is an integer suffix: u and l, L, ll or LL, each
// at most once, in either order; or Microsoft's i8, i16, i32 or i64 after
// an optional u.  Sets psLiteral's bUnsigned, iLongs and bInt64.
//
//////////////////////////////////////////////////////////////////////////
static bool ParseIntegerSuffix(const char* sz, SNumberLiteral* psLiteral)
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
// Whether sz (all of it) is a floating suffix: none, f or l, either case.
// Sets psLiteral's eFloating.
//
//////////////////////////////////////////////////////////////////////////
static bool ParseFloatingSuffix(const char* sz, SNumberLiteral* psLiteral)
{
	psLiteral->eFloating = FL_Double;
	if ((sz[0] == 'f') || (sz[0] == 'F'))
	{
		psLiteral->eFloating = FL_Float;
		sz++;
	}
	else if ((sz[0] == 'l') || (sz[0] == 'L'))
	{
		psLiteral->eFloating = FL_LongDouble;
		sz++;
	}
	return *sz == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sz (all of it) is an identifier: what a user-defined literal's
// suffix is.  [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
static bool IsUserSuffix(const char* sz)
{
	if (!IsCPPNameStart(*sz))
	{
		return false;
	}
	for (sz++; IsCPPNameCharacter(*sz); sz++)
	{
	}
	return *sz == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
// A number literal's suffix, szSuffix (all that follows its digits): one
// of the language's, or else a user-defined literal's, which is cut from
// szNumber and its length kept.
//
//////////////////////////////////////////////////////////////////////////
static bool ParseNumberSuffix(char* szSuffix, bool bFloating, SNumberLiteral* psLiteral)
{
	psLiteral->uiUserSuffix = 0;
	if (bFloating ? ParseFloatingSuffix(szSuffix, psLiteral) : ParseIntegerSuffix(szSuffix, psLiteral))
	{
		*szSuffix = '\0';
		return true;
	}
	if (!IsUserSuffix(szSuffix))
	{
		return false;
	}

	//The literal operator is given the value as the widest type.  [lex.ext] 3
	psLiteral->uiUserSuffix = strlen(szSuffix);
	psLiteral->bUnsigned = true;
	psLiteral->iLongs = 2;
	psLiteral->bInt64 = false;
	psLiteral->eFloating = FL_LongDouble;
	*szSuffix = '\0';
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether c is a digit of iBase (2, 8, 10 or 16).
//
//////////////////////////////////////////////////////////////////////////
static bool IsDigit(char c, int iBase)
{
	if ((c >= '0') && (c <= '9'))
	{
		return true;
	}
	return (iBase == 16) && (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')));
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the number literal szText (a whole pp-number, uiLength chars):
// whether it is an integer or a floating literal, its suffix and, for an
// integer, its base and value.  Digit separators are left out of szNumber;
// each must be between two digits.  A literal longer than szNumber holds
// is NLR_TooLarge.  The code and #if both read numbers with it, so they
// agree.  [lex.icon], [lex.fcon]
//
//////////////////////////////////////////////////////////////////////////
ENumberLiteralResult ParseNumberLiteral(const char* szText, size uiLength, SNumberLiteral* psLiteral)
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
	bool		bExponent;
	uint64		ulliValue;
	uint64		ulliDigit;

	szNumber = psLiteral->szNumber;
	iLength = 0;
	for (i = 0; i < uiLength; i++)
	{
		if (szText[i] == '\'')
		{
			if ((i == 0) || (i + 1 == uiLength) || !IsDigit(szText[i - 1], 16) || !IsDigit(szText[i + 1], 16))
			{
				return NLR_InvalidSeparator;
			}
		}
		else
		{
			if (iLength == sizeof(psLiteral->szNumber) - 1)
			{
				return NLR_TooLarge;
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

	//Find where the digits end and the suffix starts.  An exponent is e (p in hexadecimal), a sign if there is one, and
	//decimal digits.
	bFloating = false;
	bExponent = false;
	for (szEnd = szDigits; ; szEnd++)
	{
		c = *szEnd;
		if (IsDigit(c, bExponent ? 10 : iBase))
		{
		}
		else if ((c == '.') && !bFloating && (iBase != 2))
		{
			bFloating = true;
		}
		else if (!bExponent && (((iBase == 10) && ((c == 'e') || (c == 'E'))) || ((iBase == 16) && ((c == 'p') || (c == 'P')))))
		{
			bFloating = true;
			bExponent = true;
			if ((szEnd[1] == '+') || (szEnd[1] == '-'))
			{
				szEnd++;
			}
			if (!IsDigit(szEnd[1], 10))
			{
				return NLR_NoExponent;
			}
		}
		else
		{
			break;
		}
	}

	if (bFloating)
	{
		if ((iBase == 16) && !bExponent)
		{
			return NLR_NoExponent;
		}
		return ParseNumberSuffix(szEnd, true, psLiteral) ? NLR_Floating : NLR_InvalidSuffix;
	}
	if ((iBase != 10) && (szEnd == szDigits))
	{
		return NLR_NoDigits;
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
			return NLR_InvalidDigit;
		}
		if (ulliValue > (0xFFFFFFFFFFFFFFFFULL - ulliDigit) / iBase)
		{
			return NLR_TooLarge;
		}
		ulliValue = ulliValue * iBase + ulliDigit;
	}

	psLiteral->ulliValue = ulliValue;
	psLiteral->iBase = iBase;
	return ParseNumberSuffix(szEnd, false, psLiteral) ? NLR_Integer : NLR_InvalidSuffix;
}


//////////////////////////////////////////////////////////////////////////
//
// What is wrong with a number literal, for an error; NULL if nothing.
//
//////////////////////////////////////////////////////////////////////////
const char* NumberLiteralError(ENumberLiteralResult eResult)
{
	switch (eResult)
	{
	case NLR_Integer:
	case NLR_Floating:
		return NULL;
	case NLR_InvalidDigit:
		return "Invalid digit in integer literal";
	case NLR_TooLarge:
		return "Integer literal is too large";
	case NLR_InvalidSuffix:
		return "Invalid number literal suffix";
	case NLR_NoDigits:
		return "Integer literal has no digits";
	case NLR_NoExponent:
		return "Floating literal has no exponent digits";
	case NLR_InvalidSeparator:
		return "A digit separator must be between two digits";
	}
	return "Invalid number literal";
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the uiLength characters at sz are the prefix of a string
// literal (bString) or a character literal: u8, u, U or L, and for a
// string R after one of them or alone.  [lex.ccon], [lex.string]
//
//////////////////////////////////////////////////////////////////////////
bool IsLiteralPrefix(const char* sz, size uiLength, bool bString)
{
	if (bString && (uiLength > 0) && (sz[uiLength - 1] == 'R'))
	{
		uiLength--;
		if (uiLength == 0)
		{
			return true;
		}
	}
	if (uiLength == 1)
	{
		return (sz[0] == 'L') || (sz[0] == 'u') || (sz[0] == 'U');
	}
	return (uiLength == 2) && (sz[0] == 'u') && (sz[1] == '8');
}


//////////////////////////////////////////////////////////////////////////
//
// The encoding a string or character literal's prefix gives it.
//
//////////////////////////////////////////////////////////////////////////
ECTEncoding LiteralEncoding(const char* szText)
{
	switch (szText[0])
	{
	case 'u':
		return (szText[1] == '8') ? CTE_UTF8 : CTE_UTF16;
	case 'U':
		return CTE_UTF32;
	case 'L':
		return CTE_Wide;
	default:
		return CTE_Narrow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The length of the string or character literal at szText (no more than
// uiLength characters), from its prefix to its closing quote: what is
// left of the text after it is a user-defined suffix.  0 if it has no
// closing quote.
//
//////////////////////////////////////////////////////////////////////////
size QuotedLiteralLength(const char* szText, size uiLength)
{
	size	i;
	char	cQuote;

	for (i = 0; (i < uiLength) && (szText[i] != '"') && (szText[i] != '\''); i++)
	{
	}
	if (i == uiLength)
	{
		return 0;
	}

	cQuote = szText[i];
	for (i++; i < uiLength; i++)
	{
		if (szText[i] == '\\')
		{
			i++;
		}
		else if (szText[i] == cQuote)
		{
			return i + 1;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The largest value one code unit of a character literal holds.  Wide
// characters are 16 bits, as on Windows.
//
//////////////////////////////////////////////////////////////////////////
static uint32 LargestCodeUnit(ECTEncoding eEncoding)
{
	switch (eEncoding)
	{
	case CTE_Wide:
	case CTE_UTF16:
		return 0xFFFF;
	case CTE_UTF32:
		return 0xFFFFFFFF;
	default:
		return 0xFF;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a character to a character literal: the first is its value, and
// more are packed in below it a byte each (as MSVC does).
//
//////////////////////////////////////////////////////////////////////////
static void AddCharacter(SCharacterLiteral* psLiteral, uint32 uiCharacter)
{
	psLiteral->uiValue = (psLiteral->iCount == 0) ? uiCharacter : ((psLiteral->uiValue << 8) | (uiCharacter & 0xFF));
	psLiteral->iCount++;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the character literal at szText (its prefix, if any, and quotes;
// no more than uiLength characters): its encoding, value and length.  The
// code and #if both read literals with it, so they agree.
//
// A character is the same however it is spelled, in the source's UTF-8 or
// as a universal character name: in a narrow literal it is its UTF-8
// bytes, each a character of the literal (as a string has them), and in
// the others it must be one code unit.  A numeric escape is a code unit
// itself.  [lex.ccon]
//
//////////////////////////////////////////////////////////////////////////
ECharacterLiteralResult ParseCharacterLiteral(const char* szText, size uiLength, SCharacterLiteral* psLiteral)
{
	const char*	sz;
	const char*	szEnd;
	uint32		uiCharacter;
	uint32		uiLargest;
	bool		bCodePoint;
	char		szUTF8[UTF8_MAX_BYTES];
	size		uiBytes;
	size		i;

	szEnd = szText + uiLength;
	psLiteral->eEncoding = LiteralEncoding(szText);
	for (sz = szText; *sz != '\''; sz++)
	{
	}
	sz++;  //The opening quote.

	uiLargest = LargestCodeUnit(psLiteral->eEncoding);
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
			bCodePoint = (*sz == 'u') || (*sz == 'U');
			if (!DecodeEscape(&sz, szEnd, &uiCharacter))
			{
				return CLR_InvalidEscape;
			}
			if (!bCodePoint && (uiCharacter > uiLargest))
			{
				return CLR_EscapeTooLarge;
			}
		}
		else
		{
			bCodePoint = true;
			uiCharacter = DecodeUTF8(&sz, szEnd);
		}

		if (!bCodePoint || (uiCharacter < 0x80))
		{
			AddCharacter(psLiteral, uiCharacter);
		}
		else if (psLiteral->eEncoding == CTE_Narrow)
		{
			uiBytes = EncodeUTF8(uiCharacter, szUTF8);
			for (i = 0; i < uiBytes; i++)
			{
				AddCharacter(psLiteral, (uint8)szUTF8[i]);
			}
		}
		else if ((psLiteral->eEncoding == CTE_UTF8) || (uiCharacter > uiLargest))
		{
			return CLR_NotOneCodeUnit;
		}
		else
		{
			AddCharacter(psLiteral, uiCharacter);
		}
	}
	psLiteral->uiLength = (size)(sz + 1 - szText);
	if (psLiteral->iCount == 0)
	{
		return CLR_Empty;
	}
	if ((psLiteral->iCount > 1) && (psLiteral->eEncoding != CTE_Narrow) && (psLiteral->eEncoding != CTE_Wide))
	{
		return CLR_SeveralCharacters;
	}
	return CLR_Character;
}


//////////////////////////////////////////////////////////////////////////
//
// What is wrong with a character literal, for an error; NULL if nothing.
//
//////////////////////////////////////////////////////////////////////////
const char* CharacterLiteralError(ECharacterLiteralResult eResult)
{
	switch (eResult)
	{
	case CLR_Character:
		return NULL;
	case CLR_Empty:
		return "Empty character literal";
	case CLR_InvalidEscape:
		return "Invalid escape sequence";
	case CLR_Unterminated:
		return "Unterminated character literal";
	case CLR_EscapeTooLarge:
		return "The escape sequence's value is too large for the character literal's type";
	case CLR_NotOneCodeUnit:
		return "The character is more than one code unit of the character literal's encoding";
	case CLR_SeveralCharacters:
		return "More than one character in a u8, u or U character literal";
	}
	return "Invalid character literal";
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


//////////////////////////////////////////////////////////////////////////
//
// The character whose UTF-8 *psz is at (a byte that doesn't start one is
// that byte), with *psz moved past it.  Reads no further than szEnd.
//
//////////////////////////////////////////////////////////////////////////
uint32 DecodeUTF8(const char** psz, const char* szEnd)
{
	const uint8*	pu;
	uint32			uiCharacter;
	int				iMore;
	int				i;

	pu = (const uint8*)*psz;
	iMore = ((pu[0] & 0xE0) == 0xC0) ? 1 : ((pu[0] & 0xF0) == 0xE0) ? 2 : ((pu[0] & 0xF8) == 0xF0) ? 3 : 0;
	uiCharacter = (iMore == 0) ? pu[0] : (pu[0] & (0x3F >> iMore));
	for (i = 1; i <= iMore; i++)
	{
		if (((const char*)&pu[i] >= szEnd) || ((pu[i] & 0xC0) != 0x80))
		{
			(*psz)++;
			return pu[0];
		}
		uiCharacter = (uiCharacter << 6) | (pu[i] & 0x3F);
	}
	*psz += iMore + 1;
	return uiCharacter;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the UTF-8 bytes of the character uiCharacter to szDest (room
// for UTF8_MAX_BYTES, no zero after them) and returns how many.
//
//////////////////////////////////////////////////////////////////////////
size EncodeUTF8(uint32 uiCharacter, char* szDest)
{
	size	uiBytes;
	size	i;

	if (uiCharacter < 0x80)
	{
		szDest[0] = (char)uiCharacter;
		return 1;
	}

	//The first byte has as many high bits set as there are bytes; the rest hold 6 bits each under 10.
	uiBytes = (uiCharacter < 0x800) ? 2 : (uiCharacter < 0x10000) ? 3 : 4;
	for (i = uiBytes - 1; i > 0; i--)
	{
		szDest[i] = (char)(0x80 | (uiCharacter & 0x3F));
		uiCharacter >>= 6;
	}
	szDest[0] = (char)((0xF0 << (4 - uiBytes)) | uiCharacter);
	return uiBytes;
}

