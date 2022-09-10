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
int			MemICmp(const void* pv1, const void* pv2, int iLength);
int			MemCmp(const void* pv1, int iLen1, const void* pv2, int iLen2);
void		MemSwp(const void* pv1, const void* pv2, size_t uiLength);
char*		StrRev(char* szString);
char*		StrRev(char* szString, int iLength);
const char*	FindChar(const char* szString, char c, bool bReverse = false);
char*		StrCpySafe(char* szDest, const char* szSource, int iDestLength, int* piSourceLength = NULL);
char*		StrCpy(char* szDest, const char* szString, const char* szLastCharInclusive);
void		ToLower(char* szString);
void		ToUpper(char* szString);
char		ToLower(char c);
char		ToUpper(char c);
int			StrEmpty(const char* szString);
int			StrPrintable(const char* szString);
int			StrPrintable(const char* szString, int iLength);
size_t		StrLen(const char* szString, const char* szLastCharInclusive = NULL);
char*		NullToEmpty(const char* szString);
char		GetHexChar(char c4Bit);

char*		IntToString(char* szDest, int iDestLength, int iValue, int iBase = 10);
char*		IntToString(char* szDest, int iDestLength, unsigned int iValue, int iBase = 10);
char*		IntToString(char* szDest, int iDestLength, int64 lliValue, int iBase = 10);
char*		IntToString(char* szDest, int iDestLength, uint64 ulliValue, int iBase = 10);
char*		FlagsToString(char* szDest, int iDestLength, int iFlags);
char*		FlagsToString(char* szDest, int iDestLength, short int iFlags);
char*		FlagsToString(char* szDest, int iDestLength, char iFlags);
char*		CharToString(char* szDest, int iDestLength, char c);
char*		CharToString(char* szDest, int iDestLength, unsigned char c);
char*		CharToString(char* szDest, int iDestLength, char16 c);
char*		BoolToString(char* szDest, int iDestLength, bool b);

extern char gszDigits[17];
extern char	gszEmptyString[4];


#endif // !__STRING_HELPER_H__

