/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __STRING_HELPER_H__
#define __STRING_HELPER_H__
#include <string.h>
#include "Define.h"
#include "PrimitiveTypes.h"


int			StringCompare(const char* ps1, const char* ps2);
int			StringInsensitiveCompare(const char* ps1, const char* ps2);
char*		StrIStr(char* ps1, const char* ps2);
int			MemICmp(const void* pv1, const void* pv2, size iLength);
int			MemCmp(const void* pv1, size iLen1, const void* pv2, size iLen2);
void		MemSwp(const void* pv1, const void* pv2, lsize uiLength);
#if defined(__W65C816__)
void		MemSwp16(const void* pv1, const void* pv2, size uiLength);  //Libc65's string816_16.asm.
#endif // __W65C816__
char*		StrRev(char* szString);
char*		StrRev(char* szString, size iLength);
const char*	FindChar(const char* szString, char c, bool bReverse = false);
size		FindCharIndex(char chr, const char* str, size index);
char*		StrCpySafe(char* szDest, const char* szSource, size iDestLength, size* piSourceLength = NULL);
char*		StrCatSafe(char* szDest, const char* szSource, size iDestLength, size* piSourceLength = NULL);
char*		StrCpy(char* szDest, const char* szString, const char* szLastCharInclusive);
void		StrCpySafeStripSurroundingSpaces(char* szDest, char* szSource, size uiMaxLength);
char*		ToLower(char* szString);
char*		ToUpper(char* szString);
char		ToLower(char c);
char		ToUpper(char c);
size		StrPrintable(const char* szString);
size		StrPrintable(const char* szString, size iLength);
size		StrLen(const char* szString);  //A natural length: MAX_SIZE when szString is that long or longer.  strlen is an lsize.
size		StrLen(const char* szString, const char* szLastCharInclusive);
bool		StrEmpty(const char* szString);
char*		NullToEmpty(const char* szString);
char		GetHexChar(size c4Bit);
size		GetCharHex(char cHexChar);
size		NumberOfDigits(size uiNumber);
char*		NextString(const char* szCurrent);

char*		ByteToString(char* szDest, size iDestLength, int8 iValue, uint16 iBase = 10);
char*		ByteToString(char* szDest, size iDestLength, uint8 iValue, uint16 iBase = 10);
char*		ShortToString(char* szDest, size iDestLength, int16 iValue, uint16 iBase = 10);
char*		ShortToString(char* szDest, size iDestLength, uint16 iValue, uint16 iBase = 10);
char*		IntToString(char* szDest, size iDestLength, int32 iValue, uint16 iBase = 10);
char*		IntToString(char* szDest, size iDestLength, uint32 iValue, uint16 iBase = 10);
#ifdef __W65C816__
//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
//would be chosen).
char*		IntToString(char* szDest, size iDestLength, int16 iValue, uint16 iBase = 10);
char*		IntToString(char* szDest, size iDestLength, uint16 iValue, uint16 iBase = 10);
#endif // __W65C816__
char*		SizeToString(char* szDest, size iDestLength, size iValue, uint16 iBase = 10);
char*		LongToString(char* szDest, size iDestLength, int64 lliValue, uint16 iBase = 10);
char*		LongToString(char* szDest, size iDestLength, uint64 ulliValue, uint16 iBase = 10);
char*		FlagsToString(char* szDest, size iDestLength, int8 iFlags);
char*		FlagsToString(char* szDest, size iDestLength, uint8 iFlags);
char*		FlagsToString(char* szDest, size iDestLength, int16 iFlags);
char*		FlagsToString(char* szDest, size iDestLength, uint16 iFlags);
char*		FlagsToString(char* szDest, size iDestLength, int32 iFlags);
char*		FlagsToString(char* szDest, size iDestLength, uint32 iFlags);
char*		CharToString(char* szDest, size iDestLength, char c);
char*		CharToString(char* szDest, size iDestLength, uint8 c);
char*		CharToString(char* szDest, size iDestLength, char16 c);
char*		BoolToString(char* szDest, size iDestLength, bool b);

extern char gszDigits[17];
extern char	gszEmptyString[4];


#endif // __STRING_HELPER_H__

