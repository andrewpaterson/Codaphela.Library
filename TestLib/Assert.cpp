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
void ToTristateString(TRISTATE t, char* szString, int iStringLength);
void ToBoolString(bool b, char* szString, int iStringLength);
void ToCharString(char c, char* szString, int iStringLength);
void ToIntString(int i, char* szString, int iStringLength);
void ToIntHexString(int i, char* szString, int iStringLength);
void ToLongLongIntString(long long int i, char* szString, int iStringLength);
void ToLongLongIntHexString(long long int i, char* szString, int iStringLength);
void ToFloatString(float f, char* sz, int iDecimals);
void ToFloat3String(SFloat3* psFloat3, char* sz, int iWholeNumbers, int iDecimals);
void ToDoubleString(double f, char* sz, int iDecimals);
void ToPointerString(void* pv, char* sz, int iStringLength);
void ToMD5String(unsigned char* puc, char* sz, int iStringLength);


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
bool PrivateAssertString(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile)
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
char* GetNextLine(int* piIndex, CArrayChars* pac)
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
bool PrivateAssertStringApproximate(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile)
{
	CChars			cExpected;
	CArrayChars		acExpected;
	CChars			cActual;
	CArrayChars		acActual;
	int				iExpected;
	int				iActual;
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
bool PrivateAssertStringStartsWith(const char* szExpected, const char* szActual, bool bTestCase, int iLine, char* szFile)
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
bool PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, int iLine, char* szFile)
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
bool PrivateAssertBool(bool bExpected, bool bActual, int iLine, char* szFile)
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
bool PrivateAssertChar(char cExpected, char cActual, int iLine, char* szFile)
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
bool PrivateAssertInt(int iExpected, int iActual, int iLine, char* szFile)
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
bool PrivateAssertSize(size_t iExpected, size_t iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntString(iExpected, szExpected, 32);
		ToLongLongIntString(iActual, szActual, 32);
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
bool PrivateAssertIntHex(int iExpected, int iActual, int iLine, char* szFile)
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
bool PrivateAssertShort(int16 iExpected, int16 iActual, int iLine, char* szFile)
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
bool PrivateAssertShortHex(int16 iExpected, int16 iActual, int iLine, char* szFile)
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
bool PrivateAssertLongLongInt(long long int iExpected, long long int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntString(iExpected, szExpected, 32);
		ToLongLongIntString(iActual, szActual, 32);
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
bool PrivateAssertLongLongIntHex(long long int iExpected, long long int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntHexString(iExpected, szExpected, 32);
		ToLongLongIntHexString(iActual, szActual, 32);
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
bool PrivateAssertLongHex(long long int iExpected, long long int iActual, int iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongLongIntHexString(iExpected, szExpected, 32);
		ToLongLongIntHexString(iActual, szActual, 32);
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
bool PrivateAssertFloat(float fExpected, float fActual, int iDecimals, int iLine, char* szFile)
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
bool PrivateAssertFloat(float fExpected, float fActual, int iLine, char* szFile)
{
	return PrivateAssertFloat(fExpected, fActual, 8, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertDouble(double fExpected, double fActual, int iDecimals, int iLine, char* szFile)
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
bool PrivateAssertLongDouble(long double fExpected, long double fActual, int iDecimals, int iLine, char* szFile)
{
	char			szExpected[32];
	char			szActual[32];
	long double		fTolerance;

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
bool PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, int iLine, char* szFile)
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
bool PrivateAssertMemory(void* pvExpected, void* pvActual, int iSize, int iLine, char* szFile)
{
	int		i;
	char 	szExpected[32];
	char 	szActual[32];

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
bool PrivateAssertZero(void* pvActual, int iSize, int iLine, char* szFile)
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
bool PrivateAssertNegative(int i, int iLine, char* szFile)
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
bool PrivateAssertPositive(int i, int iLine, char* szFile)
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
bool PrivateAssertNumber(const char* szExpected, CNumber* pcActual, int iLine, char* szFile)
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
bool PrivateAssertPointer(void* pvExpected, void* pvActual, int iLine, char* szFile)
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
bool PrivateAssertMD5(unsigned char* pucExpected, unsigned char* pucActual, int iLine, char* szFile)
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
bool PrivateAssertNotNull(void* pvActual, int iLine, char* szFile)
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
bool PrivateAssertNull(void* pvActual, int iLine, char* szFile)
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
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, int iLine, char* szFile)
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
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size_t iLength, int iLine, char* szFile)
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
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, int iLine, char* szFile)
{
	size_t iLength;

	iLength = strlen(szString);
	return PrivateAssertFileMemory(szExpectedFilename, (void*)szString, iLength, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool Failed(int iLine, char* szFile)
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
bool Failed(const char* szExpected, const char* szActual, int iLine, char* szFile, bool bNewLine)
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
void ToTristateString(TRISTATE t, char* szString, int iStringLength)
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
void ToBoolString(bool b, char* szString, int iStringLength)
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
void ToIntString(int i, char* szString, int iStringLength)
{
	IntToString(szString, iStringLength, i, 10);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToIntHexString(int i, char* szString, int iStringLength)
{
	IntToString(&szString[2], iStringLength-2, i, 16);
	szString[0] = '0';
	szString[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToCharString(char c, char* szString, int iStringLength)
{
	CharToString(c);
	if ((c >= 0 && c <= 31) || (c == 127) || ((unsigned char)c >= 128 && (unsigned char)c <= 159))
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
void ToLongLongIntString(long long int i, char* szString, int iStringLength)
{
    IntToString(szString, iStringLength, i, 10);
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
void ToLongDoubleString(long double f, char* sz, int iDecimals)
{
	char szFormatter[20];

	sprintf(szFormatter, "%%.%ifL", iDecimals);
	sprintf(sz, szFormatter, f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToLongLongIntHexString(long long int i, char* szString, int iStringLength)
{
	IntToString(&szString[2], iStringLength-2, i, 16);
	szString[0] = '0';
	szString[1] = 'x';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ToPointerString(void* pv, char* szString, int iStringLength)
{
	CChars	c;

	IntToString(szString, iStringLength, (int)(size_t)pv, 16);

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
void ToMD5String(unsigned char* puc, char* sz, int iStringLength)
{
	CChars	c;

	c.Init();
	c.AppendHexLoHi(puc, 16);
	c.CopyIntoBuffer(sz, iStringLength);
	c.Kill();
}

