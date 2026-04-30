#ifndef __ASSERT_STRING_HELPER_H__
#define __ASSERT_STRING_HELPER_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestLib

Codaphela TestLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Define.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Define.h"


bool IsMultiLine(const char* szExpected, const char* szActual);
void ToTristateString(TRISTATE t, char* szString, size iStringLength);
void ToBoolString(bool b, char* szString, size iStringLength);
void ToCharString(uint8 c, char* szString, size iStringLength);
void ToCharString(char c, char* szString, size iStringLength);
void ToShortString(int16 i, char* szString, size iStringLength);
void ToShortString(uint16 i, char* szString, size iStringLength);
void ToIntString(int32 i, char* szString, size iStringLength);
void ToIntString(uint32 i, char* szString, size iStringLength);
void ToByteHexString(int8 i, char* szString, size iStringLength);
void ToByteHexString(uint8 i, char* szString, size iStringLength);
void ToShortHexString(int16 i, char* szString, size iStringLength);
void ToShortHexString(uint16 i, char* szString, size iStringLength);
void ToIntHexString(int32 i, char* szString, size iStringLength);
void ToIntHexString(uint32 i, char* szString, size iStringLength);
void ToLongString(int64 i, char* szString, size iStringLength);
void ToLongString(uint64 i, char* szString, size iStringLength);
void ToLongHexString(int64 i, char* szString, size iStringLength);
void ToLongHexString(uint64 i, char* szString, size iStringLength);
void ToFloatString(float f, char* sz, int iDecimals);
void ToFloat3String(SFloat3* psFloat3, char* sz, int iWholeNumbers, int iDecimals);
void ToDoubleString(double f, char* sz, int iDecimals);
void ToPointerString(void* pv, char* sz, size iStringLength);
void ToMD5String(uint8* puc, char* sz, size iStringLength);

bool Failed(const char* szExpected, const char* szActual, char* szPrefix, size iLine, char* szFile, bool bNewLine);


#endif // __ASSERT_STRING_HELPER_H__

