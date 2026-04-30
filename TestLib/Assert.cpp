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
#include <string.h>
#include <stdio.h>
#include "BaseLib/NewLine.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/Define.h"
#include "BaseLib/FileCompare.h"
#include "BaseLib/Float3.h"
#include "Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const char* szExpected, const uint8* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertString(szExpected, (char*)szActual, bTestCase, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const uint8* szExpected, const uint8* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertString((char*)szExpected, (char*)szActual, bTestCase, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertString(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile)
{
	if ((szExpected == NULL) && (szActual == NULL))
	{
		return Pass();
	}
	else if (szActual == NULL)
	{
		return Failed(szExpected, "** NULL **", szPrefix, iLine, szFile, false);
	}
	else if (szExpected == NULL)
	{
		return Failed("** NULL **", szActual, szPrefix, iLine, szFile, false);
	}
	else
	{
		if (bTestCase)
		{
			if (StringCompare(szExpected, szActual) != 0)
			{
				return Failed(szExpected, szActual, szPrefix, iLine, szFile, IsMultiLine(szExpected, szActual));
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
				return Failed(szExpected, szActual, szPrefix, iLine, szFile, IsMultiLine(szExpected, szActual));
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
			pc->StripWhitespace();
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
bool PrivateAssertStringApproximate(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile)
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
		bResult = PrivateAssertString(szExpectedLine, szActualLine, bTestCase, szPrefix, iLine, szFile);
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
bool PrivateAssertStringStartsWith(const char* szExpected, const char* szActual, bool bTestCase, char* szPrefix, size iLine, char* szFile)
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
		return Failed(szExpected, "** NULL **", szPrefix, iLine, szFile, false);
	}
	else if (szExpected == NULL)
	{
		return Failed("** NULL **", szActual, szPrefix, iLine, szFile, false);
	}
	else
	{
		iStrLenActual = StrLen(szActual);
		iStrLenExpected = StrLen(szExpected);

		if (iStrLenActual < iStrLenExpected)
		{
			return Failed(szExpected, szActual, NULL, iLine, szFile, IsMultiLine(szExpected, szActual));
		}

		if (bTestCase)
		{
			for (i = 0; i < iStrLenExpected; i++)
			{
				if (szExpected[i] != szActual[i])
				{
					return Failed(szExpected, szActual, NULL, iLine, szFile, IsMultiLine(szExpected, szActual));
				}
			}
		}
		else
		{
			for (i = 0; i < iStrLenExpected; i++)
			{
				if (ToLower(szExpected[i]) != ToLower(szActual[i]))
				{
					return Failed(szExpected, szActual, NULL, iLine, szFile, IsMultiLine(szExpected, szActual));
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
bool PrivateAssertTristate(TRISTATE tExpected, TRISTATE tActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (tExpected != tActual)
	{
		ToTristateString(tExpected, szExpected, 32);
		ToTristateString(tActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertBool(bool bExpected, bool bActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (!((((bExpected == 0) && (bActual == 0))) || (((bExpected != 0) && (bActual != 0)))))
	{
		ToBoolString(bExpected, szExpected, 32);
		ToBoolString(bActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertChar(char cExpected, char cActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[5];
	char szActual[5];

	if (cExpected != cActual)
	{
		ToCharString(cExpected, szExpected, 5);
		ToCharString(cActual, szActual, 5);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertInt(int32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertInt(iExpected, (int32)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertInt(uint32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertInt((int32)iExpected, (int32)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertInt(int32 iExpected, int32 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected, 32);
		ToIntString(iActual, szActual, 323);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertSize(size iExpected, size iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		if (sizeof(size) == 2)
		{
			ToShortString((uint16)iExpected, szExpected, 32);
			ToShortString((uint16)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
		}
		else if (sizeof(size) == 4)
		{
			ToIntString((uint32)iExpected, szExpected, 32);
			ToIntString((uint32)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
		}
		else if (sizeof(size) == 8)
		{
			ToLongString((uint64)iExpected, szExpected, 32);
			ToLongString((uint64)iActual, szActual, 323);
			return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
		}
		else
		{
			return Failed("Valid size", "Invalid size", szPrefix, iLine, szFile, false);
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
bool PrivateAssertIntHex(int32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertIntHex(iExpected, (int32)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertIntHex(uint32 iExpected, uint32 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertIntHex((int32)iExpected, (int32)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertIntHex(int32 iExpected, int32 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntHexString(iExpected, szExpected, 32);
		ToIntHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertByte(uint8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertByte((int8)iExpected, (int8)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertByte(int8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertByte(iExpected, (int8)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertByte(int8 iExpected, int8 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected, 32);
		ToIntString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertByteHex(int8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertByteHex(iExpected, (int8)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertByteHex(uint8 iExpected, uint8 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertByteHex((int8)iExpected, (int8)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertByteHex(int8 iExpected, int8 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToByteHexString(iExpected, szExpected, 32);
		ToByteHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertShort(uint16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertShort((int16)iExpected, (int16)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShort(int16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertShort(iExpected, (int16)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShort(int16 iExpected, int16 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToIntString(iExpected, szExpected, 32);
		ToIntString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertShortHex(int16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertShortHex(iExpected, (int16)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShortHex(uint16 iExpected, uint16 iActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertShortHex((int16)iExpected, (int16)iActual, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertShortHex(int16 iExpected, int16 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToShortHexString(iExpected, szExpected, 32);
		ToShortHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertLongLongInt(int64 iExpected, int64 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongString(iExpected, szExpected, 32);
		ToLongString(iActual, szActual, 32);
		strcat(szExpected, "LL");
		strcat(szActual, "LL");
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertLongLongIntHex(int64 iExpected, int64 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongHexString(iExpected, szExpected, 32);
		ToLongHexString(iActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertLongHex(int64 iExpected, int64 iActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (iExpected != iActual)
	{
		ToLongHexString(iExpected, szExpected, 32);
		ToLongHexString(iActual, szActual, 32);
		strcat(szExpected, "LL");
		strcat(szActual, "LL");
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertFloat(float fExpected, float fActual, int iDecimals, char* szPrefix, size iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	float	fTolerance;

	fTolerance = FloatToleranceForDecimals(iDecimals);
	if (!FloatEqual(fExpected, fActual, fTolerance))
	{
		ToFloatString(fExpected, szExpected, iDecimals);
		ToFloatString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertFloat(float fExpected, float fActual, char* szPrefix, size iLine, char* szFile)
{
	return PrivateAssertFloat(fExpected, fActual, 8, szPrefix, iLine, szFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool PrivateAssertDouble(double fExpected, double fActual, int iDecimals, char* szPrefix, size iLine, char* szFile)
{
	char	szExpected[32];
	char	szActual[32];
	double	fTolerance;

	fTolerance = DoubleToleranceForDecimals(iDecimals);
	if (!DoubleEqual(fExpected, fActual, fTolerance))
	{
		ToDoubleString(fExpected, szExpected, iDecimals);
		ToDoubleString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertLongDouble(float96 fExpected, float96 fActual, int iDecimals, char* szPrefix, size iLine, char* szFile)
{
	char			szExpected[32];
	char			szActual[32];
	float96		fTolerance;

	fTolerance = DoubleToleranceForDecimals(iDecimals);
	if (!LongDoubleEqual(fExpected, fActual, fTolerance))
	{
		ToDoubleString(fExpected, szExpected, iDecimals);
		ToDoubleString(fActual, szActual, iDecimals);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertFloat3(SFloat3 fExpected, SFloat3* pfActual, int iDecimals, char* szPrefix, size iLine, char* szFile)
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
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertMemory(void* pvExpected, void* pvActual, size iSize, char* szPrefix, size iLine, char* szFile)
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
			return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
		}
	}

	i = memcmp(pvExpected, pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "memcmp() == 0");
		sprintf(szActual, "memcmp() == %i", i);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
	size		i;

	for (i = 0; i < iSize; i++)
	{
		if (((int8*)pv)[i] > 0)
		{
			return 1;
		}
		if (((int8*)pv)[i] < 0)
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
bool PrivateAssertZero(void* pvActual, size iSize, char* szPrefix, size iLine, char* szFile)
{
	int		i;
	char 	szExpected[32];
	char 	szActual[32];

	i = zerocmp(pvActual, iSize);
	if (i != 0)
	{
		sprintf(szExpected, "zerocmp() == 0");
		sprintf(szActual, "zerocmp() == %i", i);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertNegative(int i, char* szPrefix, size iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i >= 0)
	{
		sprintf(szExpected, ">= 0");
		ToIntString(i, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertPositive(int i, char* szPrefix, size iLine, char* szFile)
{
	char 	szExpected[32];
	char 	szActual[32];

	if (i <= 0)
	{
		sprintf(szExpected, "<= 0");
		ToIntString(i, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertNumber(const char* szExpected, CNumber* pcActual, char* szPrefix, size iLine, char* szFile)
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
	if (!pcExpected)
	{
		pcExpected = gcNumberControl.Add();
	}
	pcExpected->Init(szExpected, pcActual->GetMaxWholeNumbers(), iDecimals);
	if (!pcExpected->Equals(pcActual))
	{
		szExpectedAsChars.Init();
		pcExpected->Print(&szExpectedAsChars);

		szActual.Init();
		pcActual->Print(&szActual);

		bResult = Failed((const char*)szExpectedAsChars.Text(), (const char*)szActual.Text(), szPrefix, iLine, szFile, false);
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
bool PrivateAssertPointer(void* pvExpected, void* pvActual, char* szPrefix, size iLine, char* szFile)
{
	char szExpected[32];
	char szActual[32];

	if (pvExpected != pvActual)
	{
		ToPointerString(pvExpected, szExpected, 32);
		ToPointerString(pvActual, szActual, 32);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertMD5(uint8* pucExpected, uint8* pucActual, char* szPrefix, size iLine, char* szFile)
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
		return Failed((const char*)szExpected, "** NULL **", szPrefix, iLine, szFile, false);
	}

	if (memcmp(pucExpected, pucActual, 16) != 0)
	{
		ToMD5String(pucExpected, szExpected, 33);
		ToMD5String(pucActual, szActual, 33);
		return Failed((const char*)szExpected, (const char*)szActual, szPrefix, iLine, szFile, false);
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
bool PrivateAssertNotNull(void* pvActual, char* szPrefix, size iLine, char* szFile)
{
	if (NULL == pvActual)
	{
		return Failed((const char*)"Not NULL", (const char*)"NULL", szPrefix, iLine, szFile, false);
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
bool PrivateAssertNull(void* pvActual, char* szPrefix, size iLine, char* szFile)
{
	if (NULL != pvActual)
	{
		return Failed("NULL", "Not NULL", szPrefix, iLine, szFile, false);
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
bool PrivateAssertFile(const char* szExpectedFilename, char* szActualFilename, char* szPrefix, size iLine, char* szFile)
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
		bResult = Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), szPrefix, iLine, szFile, false);
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
bool PrivateAssertFileMemory(const char* szExpectedFilename, void* pcMemory, size iLength, char* szPrefix, size iLine, char* szFile)
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
		Failed((const char*)szExpected.Text(), (const char*)szActual.Text(), szPrefix, iLine, szFile, false);
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
bool PrivateAssertFileString(const char* szExpectedFilename, const char* szString, char* szPrefix, size iLine, char* szFile)
{
	size iLength;

	iLength = strlen(szString);
	return PrivateAssertFileMemory(szExpectedFilename, (void*)szString, iLength, szPrefix, iLine, szFile);
}

