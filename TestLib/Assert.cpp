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
#include <Time.h>
#include "BaseLib/Chars.h"
#include "BaseLib/NewLine.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FileCompare.h"
#include "BaseLib/Float3.h"
#include "Assert.h"


int	giTestsRun;
int	giTestsPassed;
int	giTestsFailed;

int	giTotalTestsRun;
int	giTotalTestsPassed;
int	giTotalTestsFailed;

clock_t gClock;

bool Failed(void);
bool IsMultiLine(const char* szExpected, const char* szActual);
void ToTristateString(TRISTATE t, char* szString, size iStringLength);
void ToBoolString(bool b, char* szString, size iStringLength);
void ToCharString(uint8 c, char* szString, size iStringLength);
void ToCharString(char c, char* szString, size iStringLength);
void ToShortString(int16 i, char* szString, size iStringLength);
void ToShortString(uint16 i, char* szString, size iStringLength);
void ToIntString(int32 i, char* szString, size iStringLength);
void ToIntString(uint32 i, char* szString, size iStringLength);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateBeginTests(char* szFile)
{
	CChars	sz;

	giTestsRun = 0;
	giTestsPassed = 0;
	giTestsFailed = 0;

	sz.Init();
	sz.Append("--------------- ");
	sz.Append(szFile);
	sz.Append(" ---------------\n");
	gcLogger.Add(sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrivateTestStatistics(void)
{
	CChars	sz;

	sz.Init();

	if (giTestsRun > 0)
	{
		sz.Append("Tests Run: ");
		sz.Append(giTestsRun);
		sz.AppendNewLine();
		sz.Append("Passed: ");
		sz.Append(giTestsPassed);
		sz.AppendNewLine();
		sz.Append("Failed: ");
		sz.Append(giTestsFailed);
	}
	else
	{
		sz.Append("No Tests Run!");
	}

	sz.AppendNewLine();
	sz.AppendNewLine();

	gcLogger.Add(sz.Text());

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InitTotalStatistics(void)
{
	gClock = clock();

	gcLogger.Init();

	giTotalTestsRun = 0;
	giTotalTestsPassed = 0;
	giTotalTestsFailed = 0;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int TestTotalStatistics(void)
{
	CChars		sz;
	clock_t		time;

	time = clock() - gClock;

	sz.Init();
	sz.Append("------------------------------ Total Results ------------------------------\n");

	if (!gbFastFunctions)
	{
		FastFunctionsInit();
	}
	if (!gbNumberControl)
	{
		NumberInit();
	}

	if (giTotalTestsRun > 0)
	{
		sz.Append("Total Tests Run: ");
		sz.Append(giTotalTestsRun);
		sz.AppendNewLine();
		sz.Append("Total Passed: ");
		sz.Append(giTotalTestsPassed);
		sz.AppendNewLine();
		sz.Append("Total Failed: ");
		sz.Append(giTotalTestsFailed);
		sz.AppendNewLine();
		sz.AppendNewLine();
		sz.Append("Time Taken: ");
		sz.Append((float)time / ((float)CLOCKS_PER_SEC), 2);
		sz.Append("s");
		sz.AppendNewLine();
		sz.AppendNewLine();

		gcLogger.Add(sz.Text());

	}
	sz.Kill();

	if (gbNumberControl)
	{
		NumberKill();
	}
	if (gbFastFunctions)
	{
		FastFunctionsKill();
	}

	gcLogger.Kill();
	return giTotalTestsFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const char* szExpected, const uint8* szActual, bool bTestCase, size iLine, char* szFile)
{
	return PrivateAssertString(szExpected, (char*)szActual, bTestCase, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const uint8* szExpected, const uint8* szActual, bool bTestCase, size iLine, char* szFile)
{
	return PrivateAssertString((char*)szExpected, (char*)szActual, bTestCase, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const char* szExpected, const char* szActual, bool bTestCase, size iLine, char* szFile)
{
	if ((szExpected == NULL) && (szActual == NULL))
	{
		return Pass();
	}
	else if (szActual == NULL)
	{
		return Failed(szExpected, "** NULL **", iLine, szFile, false);
	}
	else if (szExpected == NULL)
	{
		return Failed("** NULL **", szActual, iLine, szFile, false);
	}
	else
	{
		if (bTestCase)
		{
			if (StringCompare(szExpected, szActual) != 0)
			{
				return Failed(szExpected, szActual, iLine, szFile, IsMultiLine(szExpected, szActual));
			}
			else
			{
				return Pass();
			}
		}
		else
		{
			if (StringInsensitiveCompare(szExpected, szActual) != 0)
			{
				return Failed(szExpected, szActual, iLine, szFile, IsMultiLine(szExpected, szActual));
			}
			else
			{
				return Pass();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* GetNextLine(uint32* piIndex, CArrayChars* pac)
{
	CChars*	pc;

	for (;;)
	{
		if (*piIndex >= pac->NumElements())
		{
			return NULL;
		}
		else
		{
			pc = pac->Get(*piIndex);
			pc->StripWhiteSpace();
			(*piIndex)++;
			if (!pc->Empty())
			{
				return pc->Text();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertStringApproximate(const char* szExpected, const char* szActual, bool bTestCase, size iLine, char* szFile)
{
	CChars			cExpected;
	CArrayChars		acExpected;
	CChars			cActual;
	CArrayChars		acActual;
	uint32			iExpected;
	uint32			iActual;
	char*			szExpectedLine;
	char*			szActualLine;
	bool			bResult;

	cExpected.Init(szExpected);
	cExpected.PassifyNewlines();
	acExpected.Init();
	cExpected.SplitLines(&acExpected);
	cExpected.Kill();

	cActual.Init(szActual);
	cActual.PassifyNewlines();
	acActual.Init();
	cActual.SplitLines(&acActual);
	cActual.Kill();

	iExpected = 0;
	iActual = 0;
	for (; ;)
	{
		szActualLine = GetNextLine(&iActual, &acActual);
		szExpectedLine = GetNextLine(&iExpected, &acExpected);
		if ((szActualLine == NULL) && (szExpectedLine == NULL))
		{
			break;
		}
		bResult = PrivateAssertString(szExpectedLine, szActualLine, bTestCase, iLine, szFile);
		if (!bResult)
		{
			break;
		}
	}

	acExpected.Kill();
	acActual.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertStringStartsWith(const char* szExpected, const char* szActual, bool bTestCase, size iLine, char* szFile)
{
	int		iStrLenExpected;
	int		iStrLenActual;
	int		i;

	if ((szExpected == NULL) && (szActual == NULL))
	{
		return Pass();
	}
	else if (szActual == NULL)
	{
		return Failed(szExpected, "** NULL **", iLine, szFile, false);
	}
	else if (szExpected == NULL)
	{
		return Failed("** NULL **", szActual, iLine, szFile, false);
	}
	else
	{
		iStrLenActual = StrLen(szActual);
		iStrLenExpected = StrLen(szExpected);

		if (iStrLenActual < iStrLenExpected)
		{
			return Failed(szExpected, szActual, iLine, szFile, IsMultiLine(szExpected, szActual));
		}

		if (bTestCase)
		{
			for (i = 0; i < iStrLenExpected; i++)
			{
				if (szExpected[i] != szActual[i])
				{
					return Failed(szExpected, szActual, iLine, szFile, IsMultiLine(szExpected, szActual));
				}
			}
		}
		else
		{
			for (i = 0; i < iStrLenExpected; i++)
			{
				if (ToLower(szExpected[i]) != ToLower(szActual[i]))
				{
					return Failed(szExpected, szActual, iLine, szFile, IsMultiLine(szExpected, szActual));
				}
			}
		}
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (tExpected != tActual)
	{
		ToTristateString(tExpected, szExpected, 32);
		ToTristateString(tActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertBool(bool bExpected, bool bActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (!((((bExpected == 0) && (bActual == 0))) || (((bExpected != 0) && (bActual != 0)))))
	{
		ToBoolString(bExpected, szExpected, 32);
		ToBoolString(bActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertChar(char cExpected, char cActual, size iLine, char* szFile)
{
	char szExpected[5];
	char szActual[5];

	if (cExpected != cActual)
	{
		ToCharString(cExpected, szExpected, 5);
		ToCharString(cActual, szActual, 5);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertInt(int32 iExpected, int32 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected, 32);
		ToIntString(iActual, szActual, 323);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertSize(size iExpected, size iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		if (sizeof(size) == 2)
		{
			ToShortString((uint16)iExpected, szExpected, 32);
			ToShortString((uint16)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
		}
		else if (sizeof(size) == 4)
		{
			ToIntString((uint32)iExpected, szExpected, 32);
			ToIntString((uint32)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
		}
		else if (sizeof(size) == 8)
		{
			ToLongString((uint64)iExpected, szExpected, 32);
			ToLongString((uint64)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
		}
		else
		{
			return Failed("Valid size", "Invalid size", iLine, szFile, false);
		}
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertIntHex(int32 iExpected, int32 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntHexString(iExpected, szExpected, 32);
		ToIntHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShort(uint16 iExpected, uint16 iActual, size iLine, char* szFile)
{
	return PrivateAssertShort((int16)iExpected, (int16)iActual, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShort(int16 iExpected, uint16 iActual, size iLine, char* szFile)
{
	return PrivateAssertShort(iExpected, (int16)iActual, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShort(int16 iExpected, int16 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected, 32);
		ToIntString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShortHex(int16 iExpected, int16 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntHexString(iExpected, szExpected, 32);
		ToIntHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertLongLongInt(int64 iExpected, int64 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongString(iExpected, szExpected, 32);
		ToLongString(iActual, szActual, 32);
		strcat(szExpected, "LL");
		strcat(szActual, "LL");
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertLongLongIntHex(int64 iExpected, int64 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongHexString(iExpected, szExpected, 32);
		ToLongHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertLongHex(int64 iExpected, int64 iActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongHexString(iExpected, szExpected, 32);
		ToLongHexString(iActual, szActual, 32);
		strcat(szExpected, "LL");
		strcat(szActual, "LL");
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFloat(float fExpected, float fActual, int iDecimals, size iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	float	fTolerance;

	fTolerance = FloatToleranceForDecimals(iDecimals);
	if (!FloatEqual(fExpected, fActual, fTolerance))
	{
		ToFloatString(fExpected, szExpected, iDecimals);
		ToFloatString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFloat(float fExpected, float fActual, size iLine, char* szFile)
{
	return PrivateAssertFloat(fExpected, fActual, 8, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertDouble(double fExpected, double fActual, int iDecimals, size iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	double	fTolerance;

	fTolerance = DoubleToleranceForDecimals(iDecimals);
	if (!DoubleEqual(fExpected, fActual, fTolerance))
	{
		ToDoubleString(fExpected, szExpected, iDecimals);
		ToDoubleString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertLongDouble(float96 fExpected, float96 fActual, int iDecimals, size iLine, char* szFile)
{
	char			szExpected[32];
	char			szActual[32];
	float96		fTolerance;

	fTolerance = DoubleToleranceForDecimals(iDecimals);
	if (!LongDoubleEqual(fExpected, fActual, fTolerance))
	{
		ToDoubleString(fExpected, szExpected, iDecimals);
		ToDoubleString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, size iLine, char* szFile)
{
	char	szExpected[96];
	char	szActual[96];
	float	fTolerance;
	int		iWholeNumbers;
	int		iOther;

	fTolerance = FloatToleranceForDecimals(iDecimals);
	if (!fExpected.CloselyEqual(pfActual, fTolerance))
	{
		iWholeNumbers = fExpected.WholeNumbers();
		iOther = pfActual->WholeNumbers();
		if (iOther > iWholeNumbers)
		{
			iWholeNumbers = iOther;
		}
		iWholeNumbers++;
		if (iWholeNumbers < 2)
		{
			iWholeNumbers = 2;
		}

		ToFloat3String(&fExpected, szExpected, iWholeNumbers, iDecimals);
		ToFloat3String(pfActual, szActual, iWholeNumbers, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertMemory(void* pvExpected, void* pvActual, size iSize, size iLine, char* szFile)
{
	size	i;
	char 	szExpected[32];
	char 	szActual[32];
	uint8	uiExpected;
	uint8	uiActual;

	for (i = 0; i < iSize; i++)
	{
		if (((uint8*)pvExpected)[i] != ((uint8*)pvActual)[i])
		{
			uiExpected = ((uint8*)pvExpected)[i];
			uiActual = ((uint8*)pvActual)[i];
			sprintf(szExpected, "mem[%i] == %u", i, uiExpected);
			sprintf(szActual, "mem[%i] == %u", i, uiActual);
			return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
		}
	}

	i = memcmp(pvExpected, pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "memcmp() == 0");
		sprintf(szActual, "memcmp() == %i", i);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	zerocmp(void* pv, size iSize)
{
	size	i;

	for (i = 0; i < iSize; i++)
	{
		if (((char*)pv)[i] > 0)
		{
			return 1;
		}
		if (((char*)pv)[i] < 0)
		{
			return -1;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertZero(void* pvActual, size iSize, size iLine, char* szFile)
{
	int		i;
	char 	szExpected[32];
	char 	szActual[32];

	i = zerocmp(pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "zerocmp() == 0");
		sprintf(szActual, "zerocmp() == %i", i);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertNegative(int i, size iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i >= 0)
	{
		sprintf(szExpected, ">= 0");
		ToIntString(i, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertPositive(int i, size iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i <= 0)
	{
		sprintf(szExpected, "<= 0");
		ToIntString(i, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertNumber(const char* szExpected, CNumber* pcActual, size iLine, char* szFile)
{
	CNumber*	pcExpected;
	CChars		szExpectedAsChars;
	CChars		szActual;
	CChars		szFake;
	int			iIndex;
	int16		iDecimals;
	bool		bResult;

	szFake.Fake((char*)szExpected);
	iIndex = szFake.Find(0, '.');
	if (iIndex != -1)
	{
		iDecimals = (int16)(szFake.Length() - iIndex);
	}
	else
	{
		iDecimals = 0;
	}

	pcExpected = gcNumberControl.Add(pcActual->GetMaxWholeNumbers(), iDecimals);
	pcExpected->Init(szExpected, pcActual->GetMaxWholeNumbers(), iDecimals);
	if (!pcExpected->Equals(pcActual))
	{
		szExpectedAsChars.Init();
		pcExpected->Print(&szExpectedAsChars);

		szActual.Init();
		pcActual->Print(&szActual);

		bResult = Failed((const char*)szExpectedAsChars.Text(), (const char*)szActual.Text(), iLine, szFile, false);
		szExpectedAsChars.Kill();
		szActual.Kill();
	}
	else
	{
		bResult = Pass();
	}
	gcNumberControl.Remove();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertPointer(void* pvExpected, void* pvActual, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (pvExpected != pvActual)
	{
		ToPointerString(pvExpected, szExpected, 32);
		ToPointerString(pvActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertMD5(uint8* pucExpected, uint8* pucActual, size iLine, char* szFile)
{
	char szExpected[33];
	char szActual[33];

	if ((pucActual == NULL) && (pucExpected == NULL))
	{
		return Pass();
	}
	else if (pucActual == NULL)
	{
		ToMD5String(pucExpected, szExpected, 33);
		return Failed((const char*)szExpected, "** NULL **", iLine, szFile, false);
	}

	if (memcmp(pucExpected, pucActual, 16) != 0)
	{
		ToMD5String(pucExpected, szExpected, 33);
		ToMD5String(pucActual, szActual, 33);
		return Failed((const char*)szExpected, (const char*)szActual, iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertNotNull(void* pvActual, size iLine, char* szFile)
{
	if (NULL == pvActual)
	{
		return Failed((const char*)"Not NULL", (const char*)"NULL", iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertNull(void* pvActual, size iLine, char* szFile)
{
	if (NULL != pvActual)
	{
		return Failed("NULL", "Not NULL", iLine, szFile, false);
	}
	else
	{
		return Pass();
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, size iLine, char* szFile)
{
	CFileCompare	cCompare;
	bool			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFilename, szActualFilename, &szExpected, &szActual);

	if (!bResult)
	{
		bResult = Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), iLine, szFile, false);
		szActual.Kill();
		szExpected.Kill();
		return bResult;
	}
	else
	{
		szActual.Kill();
		szExpected.Kill();
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, size iLine, char* szFile)
{
	CFileCompare	cCompare;
	bool			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFilename, pcMemory, iLength, &szExpected, &szActual);

	if (!bResult)
	{
		Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), iLine, szFile, false);
		szActual.Kill();
		szExpected.Kill();
		return false;
	}
	else
	{
		szActual.Kill();
		szExpected.Kill();
		return Pass();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, size iLine, char* szFile)
{
	size iLength;

	iLength = strlen(szString);
	return PrivateAssertFileMemory(szExpectedFilename, (void*)szString, iLength, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Failed(size iLine, char* szFile)
{
	char    szLine[1024];
	CChars  szError;

	Failed();

	IntToString(szLine, 1024, iLine, 10);
	szError.Init("Failed: ");
	szError.Append(szFile);
	szError.Append(" line ");
	szError.Append(szLine);
	szError.AppendNewLine();

	gcLogger.Add(szError.Text());

	szError.Kill();

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Failed(const char* szExpected, const char* szActual, size iLine, char* szFile, bool bNewLine)
{
	char    szLine[1024];
	CChars  szError;

	Failed();

	IntToString(szLine, 1024, iLine, 10);
	szError.Init("Failed: ");
	szError.Append(szFile);
	szError.Append(" line ");
	szError.Append(szLine);
	szError.AppendNewLine();

	szError.Append("Expected: ");
	if (bNewLine)
	{
		szError.Append("--->");
		szError.AppendNewLine();
	}
	szError.Append(szExpected);
	if (bNewLine)
	{
		szError.Append("<---");
	}
	szError.AppendNewLine();

	szError.Append("Actual:   ");
	if (bNewLine)
	{
		szError.Append("--->");
		szError.AppendNewLine();
	}
	szError.Append(szActual);
	if (bNewLine)
	{
		szError.Append("<---");
	}
	szError.AppendNewLine();
	szError.AppendNewLine();

	gcLogger.Add(szError.Text());

	szError.Kill();

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsMultiLine(const char* szExpected, const char* szActual)
{
	int		iCount;

	iCount = CountNewLines((char*)szExpected, StrLen(szExpected));
	if (iCount > 0)
	{
		return true;
	}

	iCount = CountNewLines((char*)szActual, StrLen(szActual));
	if (iCount > 0)
	{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Failed(void)
{
	giTestsRun++;
	giTestsFailed++;

	giTotalTestsRun++;
	giTotalTestsFailed++;

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Pass(void)
{
	giTestsRun++;
	giTestsPassed++;

	giTotalTestsRun++;
	giTotalTestsPassed++;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToTristateString(TRISTATE t, char* szString, size iStringLength)
{
	if (t == TRITRUE)
	{
		StrCpySafe(szString, "TRITRUE", iStringLength);
	}
	else if (t == TRIFALSE)
	{
		StrCpySafe(szString, "TRIFALSE", iStringLength);
	}
	else if (t == TRIERROR)
	{
		StrCpySafe(szString, "TRIERROR", iStringLength);
	}
	else if (t == TRIGNORED)
	{
		StrCpySafe(szString, "TRIGNORED", iStringLength);
	}	
	else
	{
		StrCpySafe(szString, "UNDEFINED", iStringLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToBoolString(bool b, char* szString, size iStringLength)
{
	if (b)
	{
		StrCpySafe(szString, "true", iStringLength);
	}
	else
	{
		StrCpySafe(szString, "false", iStringLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToShortString(int16 i, char* szString, size iStringLength)
{
	ShortToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToShortString(uint16 i, char* szString, size iStringLength)
{
	ShortToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntString(int32 i, char* szString, size iStringLength)
{
	IntToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntString(uint32 i, char* szString, size iStringLength)
{
	IntToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongString(int64 i, char* szString, size iStringLength)
{
	LongToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongString(uint64 i, char* szString, size iStringLength)
{
	LongToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntHexString(int32 i, char* szString, size iStringLength)
{
	return ToIntHexString((uint32)i, szString, iStringLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntHexString(uint32 i, char* szString, size iStringLength)
{
	IntToString(&szString[2], iStringLength - 2, i, 16);
	szString[0] = '0';
	szString[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToCharString(uint8 c, char* szString, size iStringLength)
{
	return ToCharString((char)c, szString, iStringLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToCharString(char c, char* szString, size iStringLength)
{
	CharToString(c);
	if ((c >= 0 && c <= 31) || (c == 127) || ((uint8)c >= 128 && (uint8)c <= 159))
	{
		ToIntHexString(c, szString, iStringLength);
	}
	else
	{
		szString[0] = c;
		szString[1] = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToFloatString(float f, char* sz, int iDecimals)
{
	char szFormatter[10];

	sprintf(szFormatter, "%%.%if", iDecimals);
	sprintf(sz, szFormatter, f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToFloat3String(SFloat3* psFloat3, char* sz, int iWholeNumbers, int iDecimals)
{
	CChars	c;

	c.Init();
	psFloat3->Print(&c, iWholeNumbers, iDecimals);
	c.CopyIntoBuffer(sz, -1);
	c.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToDoubleString(double f, char* sz, int iDecimals)
{
	char szFormatter[20];

	sprintf(szFormatter, "%%.%if", iDecimals);
	sprintf(sz, szFormatter, f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongDoubleString(float96 f, char* sz, int iDecimals)
{
	char szFormatter[20];

	sprintf(szFormatter, "%%.%ifL", iDecimals);
	sprintf(sz, szFormatter, f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongHexString(int64 i, char* szString, size iStringLength)
{
	return ToLongHexString((uint64)i, szString, iStringLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongHexString(uint64 i, char* szString, size iStringLength)
{
	LongToString(&szString[2], iStringLength - 2, i, 16);
	szString[0] = '0';
	szString[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToPointerString(void* pv, char* szString, size iStringLength)
{
	CChars	c;

	IntToString(szString, iStringLength, (uint32)pv, 16);

	c.Init(szString);
	c.RightAlign('0', 8);
	c.Insert(0, "0x");
	strcpy(szString, c.Text());
	c.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToMD5String(uint8* puc, char* sz, size iStringLength)
{
	CChars	c;

	c.Init();
	c.AppendHexLoHi(puc, 16);
	c.CopyIntoBuffer(sz, iStringLength);
	c.Kill();
}

