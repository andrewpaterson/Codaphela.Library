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
#include "BaseLib/FileBasic.h"
#include "BaseLib/Define.h"
#include "BaseLib/Log.h"
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

BOOL Fail(void);
BOOL Pass(void);
BOOL Fail(char* szExpected, char* szActual, int iLine, char* szFile);
void ToTristateString(TRISTATE t, char* sz);
void ToBoolString(BOOL b, char* sz);
void ToCharString(char c, char* sz);
void ToIntString(int i, char* sz);
void ToIntHexString(int i, char* sz);
void ToLongLongIntString(long long int i, char* sz);
void ToLongLongIntHexString(long long int i, char* sz);
void ToFloatString(float f, char* sz, int iDecimals);
void ToFloat3String(SFloat3* psFloat3, char* sz, int iWholeNumbers, int iDecimals);
void ToDoubleString(double f, char* sz, int iDecimals);
void ToPointerString(void* pv, char* sz);
void ToMD5String(unsigned char* puc, char* sz);


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
		sz.Append((float)time / ((float)CLOCKS_PER_SEC));
		sz.Append("s");
		sz.AppendNewLine();
		sz.AppendNewLine();

		gcLogger.Add(sz.Text());

		sz.Kill();
	}
	gcLogger.Kill();
	return giTotalTestsFailed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL PrivateAssertString(char* szExpected, char* szActual, BOOL bTestCase, int iLine, char* szFile)
{
	if ((szExpected == NULL) && (szActual == NULL))
	{
		return Pass();
	}
	else if (szActual == NULL)
	{
		return Fail(szExpected, "** NULL **", iLine, szFile);
	}
	else if (szExpected == NULL)
	{
		return Fail("** NULL **", szActual, iLine, szFile);
	}
	else
	{
		if (bTestCase)
		{
			if (strcmp(szExpected, szActual) != 0)
			{
				return Fail(szExpected, szActual, iLine, szFile);
			}
			else
			{
				return Pass();
			}
		}
		else
		{
			if (StrICmp(szExpected, szActual) != 0)
			{
				return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (tExpected != tActual)
	{
		ToTristateString(tExpected, szExpected);
		ToTristateString(tActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertBool(BOOL bExpected, BOOL bActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (!((((bExpected == 0) && (bActual == 0))) || (((bExpected != 0) && (bActual != 0)))))
	{
		ToBoolString(bExpected, szExpected);
		ToBoolString(bActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertChar(char cExpected, char cActual, int iLine, char* szFile)
{
	char szExpected[2];
	char szActual[2];

	if (cExpected != cActual)
	{
		ToCharString(cExpected, szExpected);
		ToCharString(cActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertInt(int iExpected, int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected);
		ToIntString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertIntHex(int iExpected, int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntHexString(iExpected, szExpected);
		ToIntHexString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertShort(short iExpected, short iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected);
		ToIntString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertShortHex(short iExpected, short iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntHexString(iExpected, szExpected);
		ToIntHexString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertLongLongInt(long long int iExpected, long long int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntString(iExpected, szExpected);
		ToLongLongIntString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertLongHex(long long int iExpected, long long int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntHexString(iExpected, szExpected);
		ToLongLongIntHexString(iActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertFloat(float fExpected, float fActual, int iDecimals, int iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	float	fTolerance;

	fTolerance = FloatToleranceForDecimals(iDecimals);
	if (!FloatEqual(fExpected, fActual, fTolerance))
	{
		ToFloatString(fExpected, szExpected, iDecimals);
		ToFloatString(fActual, szActual, iDecimals);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertDouble(double fExpected, double fActual, int iDecimals, int iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	double	fTolerance;

	fTolerance = DoubleToleranceForDecimals(iDecimals);
	if (!DoubleEqual(fExpected, fActual, fTolerance))
	{
		ToDoubleString(fExpected, szExpected, iDecimals);
		ToDoubleString(fActual, szActual, iDecimals);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, int iLine, char* szFile)
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
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertMemory(void* pvExpected, void* pvActual, int iSize, int iLine, char* szFile)
{
	int		i;
	char 	szExpected[32];
	char 	szActual[32];

	i = memcmp(pvExpected, pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "memcmp() == 0");
		sprintf(szActual, "memcmp() == %i", i);
		return Fail(szExpected, szActual, iLine, szFile);
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
int	zerocmp(void* pv, int iSize)
{
	int		i;

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
BOOL PrivateAssertZero(void* pvActual, int iSize, int iLine, char* szFile)
{
	int		i;
	char 	szExpected[32];
	char 	szActual[32];

	i = zerocmp(pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "zerocmp() == 0");
		sprintf(szActual, "zerocmp() == %i", i);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertNegative(int i, int iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i >= 0)
	{
		sprintf(szExpected, ">= 0");
		ToIntString(i, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertPositive(int i, int iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i <= 0)
	{
		sprintf(szExpected, "<= 0");
		ToIntString(i, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertNumber(char* szExpected, CNumber* pcActual, int iLine, char* szFile)
{
	CNumber*	pcExpected;
	CChars		szExpectedAsChars;
	CChars		szActual;
	CChars		szFake;
	int			iIndex;
	short		iDecimals;
	BOOL		bResult;

	szFake.Fake(szExpected);
	iIndex = szFake.Find(0, '.');
	if (iIndex != -1)
	{
		iDecimals = (short)(szFake.Length() - iIndex);
	}
	else
	{
		iDecimals = 0;
	}

	pcExpected = gcNumberControl.Add(pcActual->mcMaxWholeNumbers, iDecimals);
	pcExpected->Init(szExpected, pcActual->mcMaxWholeNumbers, iDecimals);
	if (!pcExpected->Equals(pcActual))
	{
		pcExpected->ToString(&szExpectedAsChars);
		pcActual->ToString(&szActual);
		bResult = Fail(szExpectedAsChars.Text(), szActual.Text(), iLine, szFile);
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
BOOL PrivateAssertPointer(void* pvExpected, void* pvActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (pvExpected != pvActual)
	{
		ToPointerString(pvExpected, szExpected);
		ToPointerString(pvActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertMD5(unsigned char* pucExpected, unsigned char* pucActual, int iLine, char* szFile)
{
	char szExpected[33];
	char szActual[33];

	if ((pucActual == NULL) && (pucExpected == NULL))
	{
		return Pass();
	}
	else if (pucActual == NULL)
	{
		ToMD5String(pucExpected, szExpected);
		return Fail(szExpected, "** NULL **", iLine, szFile);
	}

	if (memcmp(pucExpected, pucActual, 16) != 0)
	{
		ToMD5String(pucExpected, szExpected);
		ToMD5String(pucActual, szActual);
		return Fail(szExpected, szActual, iLine, szFile);
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
BOOL PrivateAssertNotNull(void* pvActual, int iLine, char* szFile)
{
	if (NULL == pvActual)
	{
		return Fail("Not NULL", "NULL", iLine, szFile);
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
BOOL PrivateAssertNull(void* pvActual, int iLine, char* szFile)
{
	if (NULL != pvActual)
	{
		return Fail("NULL", "Not NULL", iLine, szFile);
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
BOOL PrivateAssertFile(char* szExpectedFileName, char* szActualFileName, int iLine, char* szFile)
{
	CFileCompare	cCompare;
	BOOL			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFileName, szActualFileName, &szExpected, &szActual);

	if (!bResult)
	{
		bResult = Fail(szExpected.Text(), szActual.Text(), iLine, szFile);
		szActual.Kill();
		szExpected.Kill();
		return bResult;
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
BOOL PrivateAssertFileMemory(char* szExpectedFileName, void* pcMemory, int iLength, int iLine, char* szFile)
{
	CFileCompare	cCompare;
	BOOL			bResult;
	CChars			szExpected;
	CChars			szActual;

	szExpected.Init();
	szActual.Init();
	bResult = cCompare.Compare(szExpectedFileName, pcMemory, iLength, &szExpected, &szActual);

	if (!bResult)
	{
		bResult = Fail(szExpected.Text(), szActual.Text(), iLine, szFile);
		szActual.Kill();
		szExpected.Kill();
		return bResult;
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
BOOL PrivateAssertFileString(char* szExpectedFileName, char* szString, int iLine, char* szFile)
{
	int iLength;

	iLength = strlen(szString);
	return PrivateAssertFileMemory(szExpectedFileName, szString, iLength, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL Fail(char* szExpected, char* szActual, int iLine, char* szFile)
{
	char    szLine[1024];
	CChars  szError;

	Fail();

	IToA(iLine, szLine, 10);
	szError.Init("Failed: ");
	szError.Append(szFile);
	szError.Append(" line ");
	szError.Append(szLine);
	szError.AppendNewLine();

	szError.Append("Expected: ");
	szError.Append(szExpected);
	szError.AppendNewLine();

	szError.Append("Actual:   ");
	szError.Append(szActual);
	szError.AppendNewLine();
	szError.AppendNewLine();

	gcLogger.Add(szError.Text());

	szError.Kill();

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL Fail(void)
{
	giTestsRun++;
	giTestsFailed++;

	giTotalTestsRun++;
	giTotalTestsFailed++;

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL Pass(void)
{
	giTestsRun++;
	giTestsPassed++;

	giTotalTestsRun++;
	giTotalTestsPassed++;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToTristateString(TRISTATE t, char* sz)
{
	if (t == TRITRUE)
	{
		strcpy(sz, "TRITRUE");
	}
	else if (t == TRIFALSE)
	{
		strcpy(sz, "TRIFALSE");
	}
	else if (t == TRIERROR)
	{
		strcpy(sz, "TRIERROR");
	}
	else
	{
		strcpy(sz, "UNDEFINED");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToBoolString(BOOL b, char* sz)
{
	if (b)
	{
		strcpy(sz, "TRUE");
	}
	else
	{
		strcpy(sz, "FALSE");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToCharString(char c, char* sz)
{
	sz[0] = c;
	sz[1] = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntString(int i, char* sz)
{
	IToA(i, sz, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntHexString(int i, char* sz)
{
	IToA(i, &sz[2], 16);
	sz[0] = '0';
	sz[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongLongIntString(long long int i, char* sz)
{
    IToA((int)i, sz, 10);
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

	c.Init(70);
	psFloat3->ToString(&c, iWholeNumbers, iDecimals);
	strcpy(sz, c.Text());
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
void ToLongLongIntHexString(long long int i, char* sz)
{
	IToA(i, &sz[2], 16);
	sz[0] = '0';
	sz[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToPointerString(void* pv, char* sz)
{
	CChars	c;

	IToA((int)(size_t)pv, sz, 16);
	c.Init(sz);
	c.RightAlign('0', 8);
	c.Insert(0, "0x");
	strcpy(sz, c.Text());
	c.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToMD5String(unsigned char* puc, char* sz)
{
	CChars	c;

	c.Init();
	c.AppendHexLoHi(puc, 16);
	strcpy(sz, c.Text());
	c.Kill();
}

