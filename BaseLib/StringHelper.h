/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
void		MemSwp(const void* pv1, const void* pv2, size uiLength);
char*		StrRev(char* szString);
char*		StrRev(char* szString, size iLength);
const char*	FindChar(const char* szString, char c, bool bReverse = false);
size		FindCharIndex(char chr, char* str, size index);
char*		StrCpySafe(char* szDest, const char* szSource, size iDestLength, size* piSourceLength = NULL);
char*		StrCpy(char* szDest, const char* szString, const char* szLastCharInclusive);
void		StrCpySafeStripSurroundingSpaces(char* szDest, char* szSource, size uiMaxLength);
void		ToLower(char* szString);
void		ToUpper(char* szString);
char		ToLower(char c);
char		ToUpper(char c);
bool		StrEmpty(const char* szString);
size		StrPrintable(const char* szString);
size		StrPrintable(const char* szString, size iLength);
size		StrLen(const char* szString, const char* szLastCharInclusive = NULL);
char*		NullToEmpty(const char* szString);
char		GetHexChar(char c4Bit);

char*		ByteToString(char* szDest, size iDestLength, int8 iValue, uint16 iBase = 10);
char*		ByteToString(char* szDest, size iDestLength, uint8 iValue, uint16 iBase = 10);
char*		ShortToString(char* szDest, size iDestLength, int16 iValue, uint16 iBase = 10);
char*		ShortToString(char* szDest, size iDestLength, uint16 iValue, uint16 iBase = 10);
char*		IntToString(char* szDest, size iDestLength, int32 iValue, uint16 iBase = 10);
char*		IntToString(char* szDest, size iDestLength, uint32 iValue, uint16 iBase = 10);
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

