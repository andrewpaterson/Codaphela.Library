#ifndef __ASSERT_FUNCTIONS_H__
#define __ASSERT_FUNCTIONS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include <string.h>
#include <stdio.h>
#include "BaseLib/Define.h"
#include "BaseLib/NumberControl.h"
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Define.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/Validation.h"


void PrivateBeginTests(char* szFile);
void PrivateTestStatistics();
int  TestTotalStatistics(void);
void InitTotalStatistics(void);

bool Pass(void);
bool Failed(size iLine, char* szFile);

bool PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertBool(bool bExpected, bool bActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertChar(char cExpected, char cActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertInt(int32 iExpected, int32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertInt(int32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertInt(uint32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertIntHex(int32 iExpected, int32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertIntHex(int32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertIntHex(uint32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByte(int8 iExpected, int8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByte(int8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByte(uint8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByteHex(int8 iExpected, int8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByteHex(int8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertByteHex(uint8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShort(int16 iExpected, int16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShort(int16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShort(uint16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShortHex(int16 iExpected, int16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShortHex(int16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertShortHex(uint16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertLongLongInt(int64 iExpected, int64 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertLongLongIntHex(int64 iExpected, int64 iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFloat(float fExpected, float fActual, int iDecimals, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFloat(float fExpected, float fActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertDouble(double fExpected, double fActual, int iDecimals, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertLongDouble(float96 fExpected, float96 fActual, int iDecimals, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertSize(size iExpected, size iActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertString(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertString(const char* szExpected, const uint8* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertString(const uint8* szExpected, const uint8* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertStringApproximate(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertMemory(void* pvExpected, void* pvActual, size iSize, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertZero(void* pvActual, size iSize, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertNegative(int i, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertPositive(int i, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertNumber(const char* szExpected, CNumber* pcActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertPointer(void* pvExpected, void* pvActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertNotNull(void* pvActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertNull(void* pvActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertMD5(uint8* pucExpected, uint8* pucActual, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, char* szPrefix, size iLine, char* szFile);
bool PrivateAssertStringStartsWith(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile);

bool Failed(const char* szExpected, const char* szActual, char* szPrefix, size iLine, char* szFile, bool bNewLine);

#define TestStatistics()					PrivateTestStatistics()
#define BeginTests()						PrivateBeginTests(__FILE__)
#define AssertString(e, a)					Validate(PrivateAssertString(e, a, true, NULL, __LINE__, __FILE__))
#define AssertStringCase(e, a, c)			Validate(PrivateAssertString(e, a, c, NULL, __LINE__, __FILE__))
#define AssertStringApproximate(e, a)		Validate(PrivateAssertStringApproximate(e, a, true, NULL, __LINE__, __FILE__))
#define AssertStringStartsWith(e, a)		Validate(PrivateAssertStringStartsWith(e, a, true, NULL, __LINE__, __FILE__))
#define AssertTristate(e, a)				Validate(PrivateAssertTristate(e, a, NULL, __LINE__, __FILE__))
#define AssertTritrue(a)					Validate(PrivateAssertTristate(TRITRUE, a, NULL, __LINE__, __FILE__))
#define AssertTrifalse(a)					Validate(PrivateAssertTristate(TRIFALSE, a, NULL, __LINE__, __FILE__))
#define AssertTrierror(a)					Validate(PrivateAssertTristate(TRIERROR, a, NULL, __LINE__, __FILE__))
#define AssertBool(e, a)					Validate(PrivateAssertBool(e, a, NULL, __LINE__, __FILE__))
#define AssertTrue(a)						Validate(PrivateAssertBool(true, a, NULL, __LINE__, __FILE__))
#define AssertFalse(a)						Validate(PrivateAssertBool(false, a, NULL, __LINE__, __FILE__))
#define AssertChar(e, a)					Validate(PrivateAssertChar(e, a, NULL, __LINE__, __FILE__))
#define AssertByte(e, a)					Validate(PrivateAssertByte(e, a, NULL, __LINE__, __FILE__))
#define AssertByteHex(e, a)					Validate(PrivateAssertByteHex(e, a, NULL, __LINE__, __FILE__))
#define AssertShort(e, a)					Validate(PrivateAssertShort(e, a, NULL, __LINE__, __FILE__))
#define AssertShortHex(e, a)				Validate(PrivateAssertShortHex(e, a, NULL, __LINE__, __FILE__))
#define AssertInt(e, a)		    			Validate(PrivateAssertInt(e, a, NULL, __LINE__, __FILE__))
#define AssertIntHex(e, a)					Validate(PrivateAssertIntHex(e, a, NULL, __LINE__, __FILE__))
#define AssertLong(e, a)					Validate(PrivateAssertLongLongInt(e, a, NULL, __LINE__, __FILE__))
#define AssertLongLongIntHex(e, a)			Validate(PrivateAssertLongLongIntHex(e, a, NULL, __LINE__, __FILE__))
#define AssertFloat(e, a, t)				Validate(PrivateAssertFloat(e, a, t, NULL, __LINE__, __FILE__))
#define AssertFloat3(e, a, t)				Validate(PrivateAssertFloat3(e, a, t, NULL, __LINE__, __FILE__))
#define AssertDouble(e, a, t)				Validate(PrivateAssertDouble(e, a, t, NULL, __LINE__, __FILE__))
#define AssertLongDouble(e, a, t)			Validate(PrivateAssertLongDouble(e, a, t, NULL, __LINE__, __FILE__))
#define AssertSize(e, a)		    		Validate(PrivateAssertSize(e, a, NULL, __LINE__, __FILE__))
#define AssertMemory(e, a, s)				Validate(PrivateAssertMemory(e, a, s, NULL, __LINE__, __FILE__))
#define AssertZero(a, s)					Validate(PrivateAssertZero(a, s, NULL, __LINE__, __FILE__))
#define AssertNegative(a)					Validate(PrivateAssertNegative(a, NULL, __LINE__, __FILE__))
#define AssertPositive(a)					Validate(PrivateAssertPositive(a, NULL, __LINE__, __FILE__))
#define AssertNumber(e, a)					Validate(PrivateAssertNumber(e, a, NULL, __LINE__, __FILE__))
#define AssertPointer(e, a)					Validate(PrivateAssertPointer(e, a, NULL, __LINE__, __FILE__))
#define AssertNotNull(a)					Validate(PrivateAssertNotNull(a, NULL, __LINE__, __FILE__))
#define AssertNull(a)						Validate(PrivateAssertNull(a, NULL, __LINE__, __FILE__))
#define AssertMD5(e, a)						Validate(PrivateAssertMD5(e, a, NULL, __LINE__, __FILE__))
#define AssertFile(e, a)					Validate(PrivateAssertFile(e, a, NULL, __LINE__, __FILE__))
#define AssertFileMemory(e, a, l)			Validate(PrivateAssertFileMemory(e, a, l, NULL, __LINE__, __FILE__))
#define AssertFileString(e, a)				Validate(PrivateAssertFileString(e, a, NULL, __LINE__, __FILE__))
#define Fail()								Validate(Failed(__LINE__, __FILE__))

#endif // __ASSERT_FUNCTIONS_H__

