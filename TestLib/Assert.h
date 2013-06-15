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


void PrivateBeginTests(char* szFile);
void PrivateTestStatistics(char* szFile);
int  TestTotalStatistics(void);
void InitTotalStatistics(void);

BOOL Pass(void);
BOOL Fail(char* szExpected, char* szActual, int iLine, char* szFile);


BOOL PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, int iLine, char* szFile);
BOOL PrivateAssertBool(BOOL bExpected, BOOL bActual, int iLine, char* szFile);
BOOL PrivateAssertChar(char cExpected, char cActual, int iLine, char* szFile);
BOOL PrivateAssertInt(int iExpected, int iActual, int iLine, char* szFile);
BOOL PrivateAssertIntHex(int iExpected, int iActual, int iLine, char* szFile);
BOOL PrivateAssertShort(short iExpected, short iActual, int iLine, char* szFile);
BOOL PrivateAssertShortHex(short iExpected, short iActual, int iLine, char* szFile);
BOOL PrivateAssertLongLongInt(long long int iExpected, long long int iActual, int iLine, char* szFile);
BOOL PrivateAssertFloat(float fExpected, float fActual, int iDecimals, int iLine, char* szFile);
BOOL PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, int iLine, char* szFile);
BOOL PrivateAssertString(char* szExpected, char* szActual, BOOL bTestCase, int iLine, char* szFile);
BOOL PrivateAssertMemory(void* pvExpected, void* pvActual, int iSize, int iLine, char* szFile);
BOOL PrivateAssertZero(void* pvActual, int iSize, int iLine, char* szFile);
BOOL PrivateAssertNegative(int i, int iLine, char* szFile);
BOOL PrivateAssertPositive(int i, int iLine, char* szFile);
BOOL PrivateAssertNumber(char* szExpected, CNumber* pcActual, int iLine, char* szFile);
BOOL PrivateAssertPointer(void* pvExpected, void* pvActual, int iLine, char* szFile);
BOOL PrivateAssertNotNull(void* pvActual, int iLine, char* szFile);
BOOL PrivateAssertNull(void* pvActual, int iLine, char* szFile);
BOOL PrivateAssertMD5(unsigned char* pucExpected, unsigned char* pucActual, int iLine, char* szFile);
BOOL PrivateAssertFile(char* szExpectedFileName, char* szActualFileName, int iLine, char* szFile);
BOOL PrivateAssertFile(char* szExpectedFileName, void* pcMemory, int iLength, int iLine, char* szFile);


#define Break()	__asm int 3;
#define EmbreakFailure(test)	if (!test) Break();


#define TestStatistics()			PrivateTestStatistics(__FILE__)
#define BeginTests()				PrivateBeginTests(__FILE__)
#define AssertString(e, a)			EmbreakFailure(PrivateAssertString(e, a, TRUE, __LINE__, __FILE__))
#define AssertStringCase(e, a, c)	EmbreakFailure(PrivateAssertString(e, a, c, __LINE__, __FILE__))
#define AssertTristate(e, a)		EmbreakFailure(PrivateAssertTristate(e, a, __LINE__, __FILE__))
#define AssertBool(e, a)			EmbreakFailure(PrivateAssertBool(e, a, __LINE__, __FILE__))
#define AssertTrue(a)				EmbreakFailure(PrivateAssertBool(TRUE, a, __LINE__, __FILE__))
#define AssertFalse(a)				EmbreakFailure(PrivateAssertBool(FALSE, a, __LINE__, __FILE__))
#define AssertChar(e, a)			EmbreakFailure(PrivateAssertChar(e, a, __LINE__, __FILE__))
#define AssertInt(e, a)		    	EmbreakFailure(PrivateAssertInt(e, a, __LINE__, __FILE__))
#define AssertIntHex(e, a)			EmbreakFailure(PrivateAssertIntHex(e, a, __LINE__, __FILE__))
#define AssertShort(e, a)			EmbreakFailure(PrivateAssertShort(e, a, __LINE__, __FILE__))
#define AssertShortHex(e, a)		EmbreakFailure(PrivateAssertShortHex(e, a, __LINE__, __FILE__))
#define AssertLongLongInt(e, a)		EmbreakFailure(PrivateAssertLongLongInt(e, a, __LINE__, __FILE__))
#define AssertFloat(e, a, t)		EmbreakFailure(PrivateAssertFloat(e, a, t, __LINE__, __FILE__))
#define AssertFloat3(e, a, t)		EmbreakFailure(PrivateAssertFloat3(e, a, t, __LINE__, __FILE__))
#define AssertMemory(e, a, s)		EmbreakFailure(PrivateAssertMemory(e, a, s, __LINE__, __FILE__))
#define AssertZero(a, s)			EmbreakFailure(PrivateAssertZero(a, s, __LINE__, __FILE__))
#define AssertNegative(a)			EmbreakFailure(PrivateAssertNegative(a, __LINE__, __FILE__))
#define AssertPositive(a)			EmbreakFailure(PrivateAssertPositive(a, __LINE__, __FILE__))
#define AssertNumber(e, a)			EmbreakFailure(PrivateAssertNumber(e, a, __LINE__, __FILE__))
#define AssertPointer(e, a)			EmbreakFailure(PrivateAssertPointer(e, a, __LINE__, __FILE__))
#define AssertNotNull(a)			EmbreakFailure(PrivateAssertNotNull(a, __LINE__, __FILE__))
#define AssertNull(a)				EmbreakFailure(PrivateAssertNull(a, __LINE__, __FILE__))
#define AssertMD5(e, a)				EmbreakFailure(PrivateAssertMD5(e, a, __LINE__, __FILE__))
#define AssertFile(e, a)			EmbreakFailure(PrivateAssertFile(e, a, __LINE__, __FILE__))
#define AssertFileMemory(e, a, l)	EmbreakFailure(PrivateAssertFile(e, a, l, __LINE__, __FILE__))


#endif // __ASSERT_FUNCTIONS_H__

