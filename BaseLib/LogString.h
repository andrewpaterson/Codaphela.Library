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
#ifndef __LOG_TO_STRING_H__
#define __LOG_TO_STRING_H__
#include "PrimitiveTypes.h"


#define __METHOD__ MethodToString(__FILE__, __LINE__, __ENGINE_PRETTY_FUNCTION__)
#define LOG_TO_STRING_SCRATCH_PAD_SIZE	32
#define LOG_TO_STRING_MAX_LENGTH		512


int IncrementLogToStringCount(void);

char* CharToString(char c);
char* CharToString(unsigned char c);
char* CharToString(char16 c);
char* IntToString(int i, int iBase = 10);
char* LongLongToString(long long int lli, int iBase = 10);
char* FloatToString(float f, int iDecimals = -1);
char* DoubleToString(double d, int iDecimals = -1);
char* SizeToString(size_t i, int iBase = 10);
char* PointerToString(void* pv);
char* MethodToString(const char* szFile, int iLine, const char* szFunction);
char* IntToFlags(int iInt);
char* ShortToFlags(int siShort);
char* StringToString(char* sz);
char* StringToString(char* szStart, char* szLastCharInclusive);
char* BoolToString(bool b);

extern char gaszLogToStringScratchPad[LOG_TO_STRING_SCRATCH_PAD_SIZE][LOG_TO_STRING_MAX_LENGTH];
extern int  giLogToStringCount;


#endif // !__LOG_TO_STRING_H__

