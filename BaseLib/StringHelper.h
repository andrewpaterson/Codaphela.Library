/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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


int			StringCompare(const char* ps1, const char* ps2);
int			StringInsensitiveCompare(const char* ps1, const char* ps2);
char*		StrIStr(char* ps1, const char* ps2);
int			MemICmp(const void* pv1, const void* pv2, int iLength);
int			MemCmp(const void* pv1, int iLen1, const void* pv2, int iLen2);
void		MemSwp(const void* pv1, const void* pv2, size_t uiLength);
char*		IToA(int iValue, char* szResult, int iBase = 10);
char*		IToA(unsigned int iValue, char* szResult, int iBase = 10);
char*		IToA(long long int lliValue, char* szResult, int iBase = 10);
char*		IToA(unsigned long long int ulliValue, char* szResult, int iBase = 10);
char*		StrRev(char* szString);
char*		StrRev(char* szString, int iLength);
const char*	FindChar(const char* szString, char c, BOOL bReverse = 0);
void		StrCpySafe(char* szDest, const char* szSource, int iDestLength);
void		ToLower(char* szString);
void		ToUpper(char* szString);
char		ToLower(char c);
char		ToUpper(char c);
int			StrEmpty(const char* szString);
int			StrPrintable(const char* szString);
int			StrPrintable(const char* szString, int iLength);
char*		NullToEmpty(const char* szString);

extern char	gszEmptyString[4];


#endif // !__STRING_HELPER_H__

