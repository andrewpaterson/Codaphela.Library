/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ASSERT_FUNCTIONS_H__
#define __ASSERT_FUNCTIONS_H__
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
bool Failed(int iLine, char* szFile);

bool PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, int iLine, char* szFile);
bool PrivateAssertBool(bool bExpected, bool bActual, int iLine, char* szFile);
bool PrivateAssertChar(char cExpected, char cActual, int iLine, char* szFile);
bool PrivateAssertInt(int iExpected, int iActual, int iLine, char* szFile);
bool PrivateAssertIntHex(int iExpected, int iActual, int iLine, char* szFile);
bool PrivateAssertShort(int16 iExpected, int16 iActual, int iLine, char* szFile);
bool PrivateAssertShort(int16 iExpected, uint16 iActual, int iLine, char* szFile);
bool PrivateAssertShort(uint16 iExpected, uint16 iActual, int iLine, char* szFile);
bool PrivateAssertShortHex(int16 iExpected, int16 iActual, int iLine, char* szFile);
bool PrivateAssertLongLongInt(int64 iExpected, int64 iActual, int iLine, char* szFile);
bool PrivateAssertLongLongIntHex(int64 iExpected, int64 iActual, int iLine, char* szFile);
bool PrivateAssertFloat(float fExpected, float fActual, int iDecimals, int iLine, char* szFile);
bool PrivateAssertFloat(float fExpected, float fActual, int iLine, char* szFile);
bool PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, int iLine, char* szFile);
bool PrivateAssertDouble(double fExpected, double fActual, int iDecimals, int iLine, char* szFile);
bool PrivateAssertLongDouble(float96 fExpected, float96 fActual, int iDecimals, int iLine, char* szFile);
bool PrivateAssertSize(size iExpected, size iActual, int iLine, char* szFile);
bool PrivateAssertString(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile);
bool PrivateAssertString(const char* szExpected, const uint8* szActual, bool bTestCase, int iLine, char* szFile);
bool PrivateAssertString(const uint8* szExpected, const uint8* szActual, bool bTestCase, int iLine, char* szFile);
bool PrivateAssertStringApproximate(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile);
bool PrivateAssertMemory(void* pvExpected, void* pvActual, int iSize, int iLine, char* szFile);
bool PrivateAssertZero(void* pvActual, int iSize, int iLine, char* szFile);
bool PrivateAssertNegative(int i, int iLine, char* szFile);
bool PrivateAssertPositive(int i, int iLine, char* szFile);
bool PrivateAssertNumber(const char* szExpected, CNumber* pcActual, int iLine, char* szFile);
bool PrivateAssertPointer(void* pvExpected, void* pvActual, int iLine, char* szFile);
bool PrivateAssertNotNull(void* pvActual, int iLine, char* szFile);
bool PrivateAssertNull(void* pvActual, int iLine, char* szFile);
bool PrivateAssertMD5(uint8* pucExpected, uint8* pucActual, int iLine, char* szFile);
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, int iLine, char* szFile);
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, int iLine, char* szFile);
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, int iLine, char* szFile);
bool PrivateAssertStringStartsWith(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile);

bool Failed(const char* szExpected, const char* szActual, int iLine, char* szFile, bool bNewLine);

#define TestStatistics()					PrivateTestStatistics()
#define BeginTests()						PrivateBeginTests(__FILE__)
#define AssertString(e, a)					Validate(PrivateAssertString(e, a, true, __LINE__, __FILE__))
#define AssertStringCase(e, a, c)			Validate(PrivateAssertString(e, a, c, __LINE__, __FILE__))
#define AssertStringApproximate(e, a)		Validate(PrivateAssertStringApproximate(e, a, true, __LINE__, __FILE__))
#define AssertStringStartsWith(e, a)		Validate(PrivateAssertStringStartsWith(e, a, true, __LINE__, __FILE__))
#define AssertTristate(e, a)				Validate(PrivateAssertTristate(e, a, __LINE__, __FILE__))
#define AssertTritrue(a)					Validate(PrivateAssertTristate(TRITRUE, a, __LINE__, __FILE__))
#define AssertTrifalse(a)					Validate(PrivateAssertTristate(TRIFALSE, a, __LINE__, __FILE__))
#define AssertTrierror(a)					Validate(PrivateAssertTristate(TRIERROR, a, __LINE__, __FILE__))
#define AssertBool(e, a)					Validate(PrivateAssertBool(e, a, __LINE__, __FILE__))
#define AssertTrue(a)						Validate(PrivateAssertBool(true, a, __LINE__, __FILE__))
#define AssertFalse(a)						Validate(PrivateAssertBool(false, a, __LINE__, __FILE__))
#define AssertChar(e, a)					Validate(PrivateAssertChar(e, a, __LINE__, __FILE__))
#define AssertInt(e, a)		    			Validate(PrivateAssertInt(e, a, __LINE__, __FILE__))
#define AssertIntHex(e, a)					Validate(PrivateAssertIntHex(e, a, __LINE__, __FILE__))
#define AssertShort(e, a)					Validate(PrivateAssertShort(e, a, __LINE__, __FILE__))
#define AssertShortHex(e, a)				Validate(PrivateAssertShortHex(e, a, __LINE__, __FILE__))
#define AssertLong(e, a)				Validate(PrivateAssertLongLongInt(e, a, __LINE__, __FILE__))
#define AssertLongLongIntHex(e, a)			Validate(PrivateAssertLongLongIntHex(e, a, __LINE__, __FILE__))
#define AssertFloat(e, a, t)				Validate(PrivateAssertFloat(e, a, t, __LINE__, __FILE__))
#define AssertFloat3(e, a, t)				Validate(PrivateAssertFloat3(e, a, t, __LINE__, __FILE__))
#define AssertDouble(e, a, t)				Validate(PrivateAssertDouble(e, a, t, __LINE__, __FILE__))
#define AssertLongDouble(e, a, t)			Validate(PrivateAssertLongDouble(e, a, t, __LINE__, __FILE__))
#define AssertSize(e, a)		    		Validate(PrivateAssertSize(e, a, __LINE__, __FILE__))
#define AssertMemory(e, a, s)				Validate(PrivateAssertMemory(e, a, s, __LINE__, __FILE__))
#define AssertZero(a, s)					Validate(PrivateAssertZero(a, s, __LINE__, __FILE__))
#define AssertNegative(a)					Validate(PrivateAssertNegative(a, __LINE__, __FILE__))
#define AssertPositive(a)					Validate(PrivateAssertPositive(a, __LINE__, __FILE__))
#define AssertNumber(e, a)					Validate(PrivateAssertNumber(e, a, __LINE__, __FILE__))
#define AssertPointer(e, a)					Validate(PrivateAssertPointer(e, a, __LINE__, __FILE__))
#define AssertNotNull(a)					Validate(PrivateAssertNotNull(a, __LINE__, __FILE__))
#define AssertNull(a)						Validate(PrivateAssertNull(a, __LINE__, __FILE__))
#define AssertMD5(e, a)						Validate(PrivateAssertMD5(e, a, __LINE__, __FILE__))
#define AssertFile(e, a)					Validate(PrivateAssertFile(e, a, __LINE__, __FILE__))
#define AssertFileMemory(e, a, l)			Validate(PrivateAssertFileMemory(e, a, l, __LINE__, __FILE__))
#define AssertFileString(e, a)				Validate(PrivateAssertFileString(e, a, __LINE__, __FILE__))
#define Fail()								Validate(Failed(__LINE__, __FILE__))

#endif // __ASSERT_FUNCTIONS_H__

