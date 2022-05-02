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
#include "Number.h"
#include <stdio.h>
#include "FastFunctions.h"
#include "IntegerHelper.h"
#include "NumberControl.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateInit(void)
{
	memset_fast(this, 0, NUMBER_SIZE(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS));
	mcMaxWholeNumbers = DEFAULT_WHOLE_NUMBERS;
	mcMaxDecimals = DEFAULT_DECIMALS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateInit(int16 cWholeNumbers, int16 cMaxDecimals)
{
	memset_fast(this, 0, NUMBER_SIZE((int)cWholeNumbers, (int)cMaxDecimals));
	mcMaxWholeNumbers = cWholeNumbers;
	mcMaxDecimals = cMaxDecimals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetSign(1);
	return this;
}
CNumber* CNumber::Init(void)
{
	PrivateInit();
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetSign(1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(int i, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int		iRemainder;
	int		iCount;
	int		iIndex;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetLastNonZeroDigit(1);
	SetSign(i);
	if (i < 0)
	{
		i = -i;
	}

	iCount = 0;
	for (;;)
	{
		iRemainder = i % 10;
		i = i / 10;
		iIndex = mcMaxWholeNumbers - iCount-1;
		if (iIndex < 0)
		{
			Overflow(GetSign());
		}
		mcDigits[iIndex] = iRemainder;
		iCount ++;
		if (i == 0)
		{
			break;
		}
	}

	SetFirstNonZerotDigit(iCount);
	CleanRight();

	return this;
}


CNumber* CNumber::Init(int i)
{
	return Init(i, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(float ff, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int		i;
	int		e;
	int		f;
	int		s;
	int		iCount;
	int		iBitMask;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);

	i = *((int*)((void*)&ff));
	s = (i & 0x80000000) >> 31;
	e = (i & 0x7F800000) >> 23;
	f = (i & 0x007FFFFF);

	if ((e == 255) && (f != 0))
	{
		SetFlag(NUMBER_FLAGS_NAN);
		return this;
	}
	if ((e == 255) && (f == 0))
	{
		return Overflow(-s);
	}
	if ((e == 0) && (f == 0))  //Zero
	{
		SetFirstNonZerotDigit(1);
		SetLastNonZeroDigit(1);
		SetSign(-s);
		return this;
	}

	if (e > 0)  //V=(-1)**S * 2 ** (E-127) * (1.F) where "1.F" is intended to represent the binary number created by prefixing F with an implicit leading 1 and a binary point.
	{
		e = e - 127;
		AddBinaryOne(e);
		for (iCount = 0; iCount < 23; iCount++)
		{
			iBitMask = 1 << (22-iCount);
			if (f & iBitMask)
			{
				AddBinaryOne(e-iCount-1);
			}
		}
	}
	else if (e == 0)  //V=(-1)**S * 2 ** (-126) * (0.F) These are "unnormalized" values.
	{
		//This only becomes import if #mcMaxDecimals > 37.
		SetFirstNonZerotDigit(1);
		SetLastNonZeroDigit(1);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}

	SetSign(-s);
	Clean();
	RoundSignificant(8);
	return this;
}
CNumber* CNumber::Init(float ff)
{
	return Init(ff, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::BinaryOne(int iBinaryExponent, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int			iNumber;

	if ((iBinaryExponent < 32) && (iBinaryExponent >= 0))
	{
		iNumber = 1 << iBinaryExponent;
		Init(iNumber, cMaxWholeNumbers, cMaxDecimals);
	}
	else if ((iBinaryExponent < 0) && (iBinaryExponent > -32))
	{
		iNumber = 1 << (-iBinaryExponent);
		Init(iNumber, cMaxWholeNumbers, cMaxDecimals);
		Inverse();
	}
	else if (iBinaryExponent >= 32)
	{
		//Complete me please...
	}
	else if (iBinaryExponent <= -32)
	{
		//Complete me please...
	}

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(const char* szNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals, int iLen)
{
	char	cDigits[64 KB];
	int		iSign;
	int		iDecimalPoint;
	int		i;
	char	c;
	int		iPos;
	BOOL	bAnyDigits;
	int		iNonZero;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);

	if (iLen == 0)
	{
		iLen = (int)strlen(szNumber);
	}
	if (iLen >= 64 KB)
	{
		return NotANumber();
	}

	iPos = 0;
	bAnyDigits = FALSE;
	iNonZero = -1;
	iDecimalPoint = 0;
	iSign = 1;
	for (i = 0; i < iLen; i++)
	{
		c = szNumber[i];
		if ((c >= '0') && (c <= '9'))
		{
			bAnyDigits = TRUE;
			if ((c != '0') && (iNonZero == -1))
			{
				iNonZero = iPos;
			}
			cDigits[iPos] = c - '0';
			iPos++;
		}
		else if (c == '.')
		{
			if (iDecimalPoint != 0)
			{
				SetFlag(NUMBER_FLAGS_NAN);
				return this;
			}
			if (!bAnyDigits)
			{
				cDigits[iPos] = 0;
				iPos++;
				bAnyDigits = TRUE;
			}
			iDecimalPoint = iPos;
		}
		else if (c == '-')
		{
			if (!bAnyDigits)
			{
				iSign = -1;
			}
			else
			{
				SetFlag(NUMBER_FLAGS_NAN);
				return this;
			}
		}
	}

	if (!bAnyDigits)
	{
		SetFlag(NUMBER_FLAGS_NAN);
		return this;
	}

	if (iNonZero == -1)
	{
		Zero();
		return this;
	}

	if (iDecimalPoint == 0)
	{
		iDecimalPoint = iPos;
	}

	if (iDecimalPoint - iNonZero > mcMaxWholeNumbers)
	{
		return Overflow(iSign);
	}

	if (iPos - iDecimalPoint > mcMaxDecimals)
	{
		iPos = iPos - (iPos - iDecimalPoint - mcMaxDecimals);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}

	memcpy_fast(&mcDigits[mcMaxWholeNumbers-iDecimalPoint], cDigits, iPos);
	SetFirstNonZerotDigit(iDecimalPoint);
	SetLastNonZeroDigit(-(iPos-iDecimalPoint));
	SetSign(iSign);

	Clean();
	return this;
}
CNumber* CNumber::Init(char* szNumber, int iLen)
{
	return Init(szNumber, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	if ((cMaxWholeNumbers + cMaxDecimals) == (pcNumber->mcMaxWholeNumbers + pcNumber->mcMaxDecimals))
	{
		memcpy_fast(this, pcNumber, NUMBER_SIZE(cMaxWholeNumbers, cMaxDecimals));
		return this;
	}
	else
	{
		PrivateCopy(pcNumber, cMaxWholeNumbers, cMaxDecimals);
		return this;
	}
}
CNumber* CNumber::Init(CNumber* pcNumber)
{
	if (DEFAULT_DIGITS == (pcNumber->mcMaxWholeNumbers + pcNumber->mcMaxDecimals))
	{
		memcpy_fast_52bytes(this, pcNumber);
		return this;
	}
	else
	{
		PrivateCopy(pcNumber, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
		return this;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Zero(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	return Init(cMaxWholeNumbers, cMaxDecimals);
}
CNumber* CNumber::Zero(void)
{
	return Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::One(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 1);
	SetSign(1);
	return this;
}
CNumber* CNumber::One(void)
{
	PrivateInit();
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 1);
	SetSign(1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Pi(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	gcNumberControl.Pi(this, cMaxWholeNumbers, cMaxDecimals);
	return this;
}
CNumber* CNumber::Pi(void)
{
	gcNumberControl.Pi(this);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::E(int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	gcNumberControl.E(this, cMaxWholeNumbers, cMaxDecimals);
	return this;
}
CNumber* CNumber::E(void)
{
	gcNumberControl.E(this);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Copy(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	return Init(pcNumber, cMaxWholeNumbers, cMaxDecimals);
}
CNumber* CNumber::Copy(CNumber* pcNumber)
{
	return Init(pcNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::NotANumber(void)
{
	mcFirstDigit = 0;
	mcLastDigit = 0;
	SetFlag(NUMBER_FLAGS_NAN);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::DivisionByZero(void)
{
	mcFirstDigit = 0;
	mcLastDigit = 0;
	SetFlag(NUMBER_FLAGS_DIVISION_BY_ZERO);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Overflow(int iSign)
{
	mcFirstDigit = 0;
	mcLastDigit = 0;
	SetSign(iSign);
	SetFlag(NUMBER_FLAGS_OVERFLOW);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Digit(char cValue, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	Zero(cMaxWholeNumbers, cMaxDecimals);
	if ((cValue >= 0) && (cValue <= 9))
	{
		SetDigitUnsafe(1, cValue);
	}
	else if ((cValue < 0) && (cValue >= -9))
	{
		SetDigitUnsafe(1, -cValue);
		SetSign(-1);
	}
	else
	{
		return NotANumber();
	}
	return this;
}
CNumber* CNumber::Digit(char cValue)
{
	Zero();
	if ((cValue >= 0) && (cValue <= 9))
	{
		SetDigitUnsafe(1, cValue);
	}
	else if ((cValue < 0) && (cValue >= -9))
	{
		SetDigitUnsafe(1, -cValue);
		SetSign(-1);
	}
	else
	{
		return NotANumber();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Digit(int iDigit, char cValue, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	Zero(cMaxWholeNumbers, cMaxDecimals);
	if ((cValue >= 0) && (cValue <= 9))
	{
		SetDigit(iDigit, cValue);
	}
	else if ((cValue < 0) && (cValue >= -9))
	{
		SetDigit(iDigit, -cValue);
		SetSign(-1);
	}
	else
	{
		return NotANumber();
	}
	return this;
}
CNumber* CNumber::Digit(int iDigit, char cValue)
{
	Zero();
	if ((cValue >= 0) && (cValue <= 9))
	{
		SetDigit(iDigit, cValue);
	}
	else if ((cValue < 0) && (cValue >= -9))
	{
		SetDigit(iDigit, -cValue);
		SetSign(-1);
	}
	else
	{
		return NotANumber();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::AddBinaryOne(int iBinaryExponent)
{
	CNumber*	pcNumber;

	pcNumber = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);

	pcNumber->BinaryOne(iBinaryExponent, mcMaxWholeNumbers, mcMaxDecimals);
	Add(pcNumber);
	gcNumberControl.Remove();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetFirstNonZerotDigit(int16 iDigit)
{
	mcFirstDigit = iDigit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetLastNonZeroDigit(int16 iDigit)
{
	mcLastDigit = iDigit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::GetFirstNonZeroDigit(void)
{
	return mcFirstDigit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::GetLastNonZeroDigit(void)
{
	return mcLastDigit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetSign(int iSign)
{
	if (iSign < 0)
	{
		SetFlag(NUMBER_FLAGS_NEGATIVE);
	}
	else
	{
		ClearFlag(NUMBER_FLAGS_NEGATIVE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int	CNumber::GetSign(void)
{
	if (IsNegative())
	{
		return -1;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsZero(void)
{
	int	iLast;
	int iFirst;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	if ((iFirst == 1) && (iLast == 1) && (GetDigit(1) == 0))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsOdd(void)
{
	int		iLast;
	char	cDigit;

	if (IsError())
	{
		return FALSE;
	}

	iLast = GetLastNonZeroDigit();
	if (iLast > 1)
	{
		return FALSE;
	}
	if (iLast <= -1)
	{
		return FALSE;
	}
	cDigit = GetDigit(1);
	return (cDigit % 2) == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsEven(void)
{
	int		iLast;
	char	cDigit;

	if (IsError())
	{
		return FALSE;
	}

	iLast = GetLastNonZeroDigit();
	if (iLast > 1)
	{
		return TRUE;
	}
	if (iLast <= -1)
	{
		return FALSE;
	}
	cDigit = GetDigit(1);
	return (cDigit % 2) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsDivisionByZero(void)
{
	return mcFlags & NUMBER_FLAGS_DIVISION_BY_ZERO;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsError(void)
{
	if (IsOverflow())
	{
		return TRUE;
	}
	if (IsNAN())
	{
		return TRUE;
	}
	if (IsDivisionByZero())
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetFlag(int16 iFlag)
{
	mcFlags |= iFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::ClearFlag(int16 iFlag)
{
	mcFlags &= ~iFlag;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::GetDecimals(void)
{
	int	iLast;

	iLast = GetLastNonZeroDigit();
	if (iLast < 0)
	{
		return -iLast;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::CleanLeft(void)
{
	int		i;
	char	c;
	int		iFirst;
	int		iLast;
	BOOL	bAnyNonZero;
	int		iFirstDiscovered;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	bAnyNonZero = FALSE;
	for (i = iFirst; i >= iLast; i--)
	{
		if (i != 0)
		{
			c = GetDigit(i);
			if ((c != 0) && (!bAnyNonZero))
			{
				bAnyNonZero = TRUE;
				iFirstDiscovered = i;
				break;
			}
		}
	}

	if (!bAnyNonZero)
	{
		SetFirstNonZerotDigit(1);
		SetLastNonZeroDigit(1);
		SetSign(1);
		return;
	}

	if (iFirst != iFirstDiscovered)
	{
		SetFirstNonZerotDigit(iFirstDiscovered);
		iFirst = iFirstDiscovered;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::CleanRight(void)
{
	int		i;
	char	c;
	int		iFirst;
	int		iLast;
	BOOL	bAnyNonZero;
	int		iLastDiscovered;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	bAnyNonZero = FALSE;
	for (i = iLast; i <= iFirst; i++)
	{
		if (i != 0)
		{
			c = GetDigit(i);
			if ((c != 0) && (!bAnyNonZero))
			{
				bAnyNonZero = TRUE;
				iLastDiscovered = i;
				break;
			}
		}
	}

	if (!bAnyNonZero)
	{
		SetFirstNonZerotDigit(1);
		SetLastNonZeroDigit(1);
		SetSign(1);
		return;
	}

	if (iLast != iLastDiscovered)
	{
		SetLastNonZeroDigit(iLastDiscovered);
		iLast = iLastDiscovered;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Clean(void)
{
	CleanLeft();
	CleanRight();
	PrivateZeroEnds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::OffsetDigit(int iCurrentDigit, int iOffset)
{
	if (iCurrentDigit < 0)
	{
		iCurrentDigit += iOffset;
		if (iCurrentDigit >= 0)
		{
			iCurrentDigit++;
		}
		return iCurrentDigit;
	}
	else if (iCurrentDigit > 0)
	{
		iCurrentDigit += iOffset;
		if (iCurrentDigit <= 0)
		{
			iCurrentDigit--;
		}
		return iCurrentDigit;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::GetDigitsBetween(int iFirst, int iLast)
{
	if (((iFirst > 0) && (iLast > 0)) || ((iFirst < 0) && (iLast < 0)))
	{
		return iFirst - iLast + 1;
	}
	else if ((iFirst > 0) && (iLast < 0))
	{
		return iFirst - iLast;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::RoundSignificant(int iSignificantDigits)
{
	int			iDigits;
	int			iDifference;
	int			iLast;
	int			iNewLast;
	char		cBeyondDigit;
	CNumber*	pcRound;
	int			iBeyondLast;

	if (iSignificantDigits <= 0)
	{
		return;
	}

	iDigits = GetDigitsBetween();
	if (iDigits > iSignificantDigits)
	{
		iLast = GetLastNonZeroDigit();
		iDifference = iDigits - iSignificantDigits;
		iNewLast = OffsetDigit(iLast, iDifference);
		iBeyondLast = OffsetDigit(iNewLast, -1);
		cBeyondDigit = GetDigit(iBeyondLast);
		if (cBeyondDigit >= 5)
		{
			pcRound = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcRound->Digit(iNewLast, 1 * GetSign(), mcMaxWholeNumbers, mcMaxDecimals);
			Add(pcRound);
			gcNumberControl.Remove();
		}
		SetLastNonZeroDigit(iNewLast);
		PrivateZeroDigits(iBeyondLast, iLast);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateZeroDigits(int iFirst, int iLast)
{
	int		iLength;
	char*	pcStart;

	pcStart = DigitToArray(iFirst);
	iLength = GetDigitsBetween(iFirst, iLast);
	memset_fast(pcStart, (unsigned char)0, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateZeroEnds(void)
{
	int		iFirst;
	int		iLast;

	iFirst = OffsetDigit(GetFirstNonZeroDigit(), 1);
	if (iFirst < mcMaxWholeNumbers)
	{
		PrivateZeroDigits(mcMaxWholeNumbers, iFirst);
	}

	iLast = GetLastNonZeroDigit();
	if (iLast > -mcMaxDecimals)
	{
		iLast = OffsetDigit(iLast, -1);
		PrivateZeroDigits(iLast, -mcMaxDecimals);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsInteger(void)
{
	int		iLast;

	iLast = GetLastNonZeroDigit();
	if (iLast > 0)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::DigitsEqual(CNumber* pcTest, int iFirstDigit, int iLastDigit)
{
	int iResult;

	iResult = memcmp(DigitToArray(iFirstDigit), pcTest->DigitToArray(iLastDigit), GetDigitsBetween(iFirstDigit, iLastDigit));
	return iResult == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Integer(void)
{
	return Truncate(-1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Ceiling(void)
{
	CNumber* pcTemp;
	CNumber* pcOne;

	pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	pcTemp->Init(this, mcMaxWholeNumbers, mcMaxDecimals);

	pcTemp->Fractional();

	Integer();

	if (!pcTemp->IsZero())
	{
		pcOne = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
		pcOne->One();
		Add(pcOne);
		gcNumberControl.Remove();
	}
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Floor(void)
{
	return Integer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Fractional(void)
{
	return TruncateHigh(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Shift(int iOffset)
{
	int		iFirst;
	int		iLast;
	int		iLength;
	int		iStart;
	char*	pcOld;
	char*	pcNew;

	if (iOffset == 0)
	{
		return this;
	}

	if (IsZero())
	{
		return this;
	}

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();
	iStart = iFirst;

	iFirst = OffsetDigit(iFirst, iOffset);
	if (iFirst > mcMaxWholeNumbers)
	{
		return Overflow(GetSign());
	}

	iLast = OffsetDigit(iLast, iOffset);
	if (iLast < -mcMaxDecimals)
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		iLast = -mcMaxDecimals;
	}
	iLength = GetDigitsBetween(iFirst, iLast);

	pcOld = DigitToArray(iStart);
	pcNew = DigitToArray(iFirst);

	memmove_fast(pcNew, pcOld, iLength);
	SetFirstNonZerotDigit(iFirst);
	SetLastNonZeroDigit(iLast);

	if (iOffset > 0)
	{
		iLast = OffsetDigit(iLast, -1);

		pcNew = DigitToArray(iLast);
		memset_fast(pcNew, (unsigned char)0, iOffset);
	}
	else
	{
		memset_fast(pcOld, (unsigned char)0, -iOffset);
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Truncate(int iDigitToTruncateInclusive)
{
	int		iLast;

	iLast = GetLastNonZeroDigit();
	if (iDigitToTruncateInclusive >= iLast)
	{
		PrivateZeroDigits(iDigitToTruncateInclusive, iLast);
		CleanRight();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::TruncateHigh(int iDigit)
{
	int		iFirst;

	iFirst = GetFirstNonZeroDigit();
	if (iDigit <= iFirst)
	{
		PrivateZeroDigits(iFirst, iDigit);
		CleanLeft();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsClean(void)
{
	CNumber*	pcTest;
	BOOL		bResult;

	pcTest = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	pcTest->Init(this, mcMaxWholeNumbers, mcMaxDecimals);
	pcTest->Clean();
	bResult = Equals(pcTest);
	gcNumberControl.Remove();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::GetWholeNumbers(void)
{
	int iFirst;

	iFirst = GetFirstNonZeroDigit();
	if (iFirst > 0)
	{
		return iFirst;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::GetDigitsBetween(void)
{
	int	iLast;
	int iFirst;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	return GetDigitsBetween(iFirst, iLast);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsNAN(void)
{
	return mcFlags & NUMBER_FLAGS_NAN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsUnderflow(void)
{
	return mcFlags & NUMBER_FLAGS_UNDERFLOW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsOverflow(void)
{
	return mcFlags & NUMBER_FLAGS_OVERFLOW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsNegative(void)
{
	return FixBool(mcFlags & NUMBER_FLAGS_NEGATIVE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsPositive(void)
{
	return !IsNegative() && !IsZero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::IsPositiveOrZero(void)
{
	return !IsNegative();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateEquals(CNumber* pcNumber, int16 iDecimals)
{
	int		iThisLastDigit;
	int		iOtherLastDigit;
	int		iThisFirstDigit;
	int		iOtherFirstDigit;
	int		iResult;

	if (IsError())
	{
		return FALSE;
	}

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	if (IsNegative() != pcNumber->IsNegative())
	{
		return FALSE;
	}

	if (iThisLastDigit < -iDecimals)
	{
		iThisLastDigit = -iDecimals;
	}
	if (iOtherLastDigit < -iDecimals)
	{
		iOtherLastDigit = -iDecimals;
	}

	if ((iThisFirstDigit != iOtherFirstDigit) || (iThisLastDigit != iOtherLastDigit))
	{
		return FALSE;
	}

	iResult = memcmp(DigitToArray(iThisFirstDigit), pcNumber->DigitToArray(iOtherFirstDigit), GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	return iResult == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::Equals(CNumber* pcNumber)
{
	return PrivateEquals(pcNumber, mcMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::GreaterThan(CNumber* pcNumber)
{
	BOOL	bThisNegative;
	BOOL	bOtherNegative;
	BOOL	bThisZero;
	BOOL	bOtherZero;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();
	bThisZero = IsZero();
	bOtherZero = pcNumber->IsZero();

	if (bThisZero)
	{
		if (bOtherZero)
		{
			return FALSE;
		}
		else if (bOtherNegative)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (bThisNegative)
	{
		if (bOtherZero)
		{
			return FALSE;
		}
		else if (bOtherNegative)
		{
			return !PrivateGreaterThanOrEquals(pcNumber);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (bOtherZero)
		{
			return TRUE;
		}
		else if (bOtherNegative)
		{
			return TRUE;
		}
		else
		{
			return PrivateGreaterThan(pcNumber);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::GreaterThanOrEquals(CNumber* pcNumber)
{
	BOOL	bThisNegative;
	BOOL	bOtherNegative;
	BOOL	bThisZero;
	BOOL	bOtherZero;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();
	bThisZero = IsZero();
	bOtherZero = pcNumber->IsZero();

	if (bThisZero)
	{
		if (bOtherZero)
		{
			return TRUE;
		}
		else if (bOtherNegative)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (bThisNegative)
	{
		if (bOtherZero)
		{
			return FALSE;
		}
		else if (bOtherNegative)
		{
			return !PrivateGreaterThan(pcNumber);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (bOtherZero)
		{
			return TRUE;
		}
		else if (bOtherNegative)
		{
			return TRUE;
		}
		else
		{
			return PrivateGreaterThanOrEquals(pcNumber);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::LessThan(CNumber* pcNumber)
{
	return !GreaterThanOrEquals(pcNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::LessThanOrEquals(CNumber* pcNumber)
{
	return !GreaterThan(pcNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateGreaterThan(CNumber* pcNumber)
{
	int		iThisLastDigit;
	int		iOtherLastDigit;
	int		iThisFirstDigit;
	int		iOtherFirstDigit;
	int		iLast;
	BOOL	bResult;

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	iLast = iThisLastDigit;
	if (iOtherLastDigit > iThisLastDigit)
	{
		iLast = iOtherLastDigit;
	}

	if (iThisFirstDigit > iOtherFirstDigit)
	{
		return TRUE;
	}
	else if (iOtherFirstDigit > iThisFirstDigit)
	{
		return FALSE;
	}

	bResult = memcmp(DigitToArray(iThisFirstDigit), pcNumber->DigitToArray(iThisFirstDigit), GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	if (bResult > 0)
	{
		return TRUE;
	}
	else if (bResult < 0)
	{
		return FALSE;
	}
	else if (iThisLastDigit < iOtherLastDigit)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateGreaterThanOrEquals(CNumber* pcNumber)
{
	int		iThisLastDigit;
	int		iOtherLastDigit;
	int		iThisFirstDigit;
	int		iOtherFirstDigit;
	int		iLast;
	BOOL	bResult;

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	iLast = iThisLastDigit;
	if (iOtherLastDigit > iThisLastDigit)
	{
		iLast = iOtherLastDigit;
	}

	if (iThisFirstDigit > iOtherFirstDigit)
	{
		return TRUE;
	}
	else if (iOtherFirstDigit > iThisFirstDigit)
	{
		return FALSE;
	}

	bResult = memcmp(DigitToArray(iThisFirstDigit), pcNumber->DigitToArray(iThisFirstDigit), GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	if (bResult > 0)
	{
		return TRUE;
	}
	else if (bResult < 0)
	{
		return FALSE;
	}
	else if (iThisLastDigit > iOtherLastDigit)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Add(CNumber* pcNumber)
{
	BOOL		bThisNegative;
	BOOL		bOtherNegative;
	CNumber*	pcTemp;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();

	if (!bThisNegative && !bOtherNegative)
	{
		PrivateAdd(pcNumber);
	}
	else if (bThisNegative && bOtherNegative)
	{
		PrivateAdd(pcNumber);
		SetSign(-1);
	}
	else if (!bThisNegative && bOtherNegative)
	{
		if (PrivateGreaterThanOrEquals(pcNumber))
		{
			PrivateSubtract(pcNumber);
		}
		else
		{
			pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->Init(pcNumber, mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->PrivateSubtract(this);
			Copy(pcTemp, mcMaxWholeNumbers, mcMaxDecimals);
			gcNumberControl.Remove();
			SetSign(-1);
		}
	}
	else if (bThisNegative && !bOtherNegative)
	{
		if (PrivateGreaterThanOrEquals(pcNumber))
		{
			PrivateSubtract(pcNumber);
			SetSign(-1);
		}
		else
		{
			pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->Init(pcNumber, mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->PrivateSubtract(this);
			Copy(pcTemp, mcMaxWholeNumbers, mcMaxDecimals);
			gcNumberControl.Remove();
		}
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Subtract(CNumber* pcNumber)
{
	BOOL		bThisNegative;
	BOOL		bOtherNegative;
	CNumber*	pcTemp;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();

	if (!bThisNegative && bOtherNegative)
	{
		PrivateAdd(pcNumber);
	}
	else if (bThisNegative && !bOtherNegative)
	{
		PrivateAdd(pcNumber);
		SetSign(-1);
	}
	else if (bThisNegative && bOtherNegative)
	{
		if (PrivateGreaterThanOrEquals(pcNumber))
		{
			PrivateSubtract(pcNumber);
			SetSign(-1);
		}
		else
		{
			pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->Init(pcNumber, mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->PrivateSubtract(this);
			Copy(pcTemp, mcMaxWholeNumbers, mcMaxDecimals);
			gcNumberControl.Remove();
			SetSign(1);
		}
	}
	else if (!bThisNegative && !bOtherNegative)
	{
		if (PrivateGreaterThanOrEquals(pcNumber))
		{
			PrivateSubtract(pcNumber);
		}
		else
		{
			pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->Init(pcNumber, mcMaxWholeNumbers, mcMaxDecimals);
			pcTemp->PrivateSubtract(this);
			Copy(pcTemp, mcMaxWholeNumbers, mcMaxDecimals);
			SetSign(-1);
			gcNumberControl.Remove();
		}
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateError(CNumber* pcNumber)
{
	if (PrivateNAN(pcNumber))
	{
		return TRUE;
	}
	if (PrivateOverflow(pcNumber))
	{
		return TRUE;
	}
	if (PrivateDivisionByZero(pcNumber))
	{
		return TRUE;
	}
	PrivateUnderflow(pcNumber);
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateAdd(CNumber* pcNumber)
{
	int		i;
	char	cCarry;
	int		iThisLastDigit;
	int		iOtherLastDigit;
	int		iLast;
	int		iFirst;
	int		iThisFirstDigit;
	int		iOtherFirstDigit;
	char	cThisDigit;
	char	cOtherDigit;
	char	cResult;

	if (PrivateError(pcNumber)) return;

	iThisLastDigit = GetLastNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	iLast = iThisLastDigit;
	if (iThisLastDigit > iOtherLastDigit)
	{
		SetLastNonZeroDigit(iOtherLastDigit);
		iLast = iOtherLastDigit;
	}

	iThisFirstDigit = GetFirstNonZeroDigit();
	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();

	iFirst = iThisFirstDigit;
	if (iThisFirstDigit < iOtherFirstDigit)
	{
		iFirst = iOtherFirstDigit;
	}

	cCarry = 0;
	for (i = iLast; i <= iFirst; i++)
	{
		if (i != 0)
		{
			cThisDigit = SafeGetDigit(i);
			cOtherDigit = pcNumber->SafeGetDigit(i);
			cResult = cThisDigit + cOtherDigit + cCarry;

			if (cResult < 10)
			{
				cCarry = 0;
			}
			else
			{
				cCarry = 1;
				cResult -= 10;
			}
			SetDigitUnsafe(i, cResult);
		}
	}

	//Oi!  You don't test for positive overflow.
	if (cCarry > 0)
	{
		if (i == 0)
		{
			i++;
		}
		SetDigitUnsafe(i, cCarry);
	}
	SetFirstNonZerotDigit(i);
	Clean();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateSubtract(CNumber* pcNumber)
{
	char	cCarry;
	int		iThisLastDigit;
	int		iOtherLastDigit;
	int		iThisFirstDigit;
	int		iOtherFirstDigit;
	char	cThisDigit;
	char	cOtherDigit;
	char	cResult;
	int		iThisLastIndex;
	int		iOtherLastIndex;
	int		iThisFirstIndex;
	int		iOtherFirstIndex;
	int		iDiff;
	int		iThisIndex;
	int		iOtherIndex;

	if (PrivateError(pcNumber)) return;

	//We're guaranteed that this is always GreaterThanOrEquals pcNumber;

	iThisFirstDigit = GetFirstNonZeroDigit();
	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();

	iThisLastDigit = GetLastNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	if (iOtherLastDigit < -mcMaxDecimals)
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		iOtherLastDigit = -mcMaxDecimals;
	}

	iThisLastIndex = PrivateGetIndex(iThisLastDigit);
	iOtherLastIndex = pcNumber->PrivateGetIndex(iOtherLastDigit);

	iThisFirstIndex = PrivateGetIndex(iThisFirstDigit);
	iOtherFirstIndex = pcNumber->PrivateGetIndex(iOtherFirstDigit);

	// xxxxxxxxxx.xxxxxxxxxxx00
	// 00yyyyyyyy.yyyyyyyyyyyyy
	cCarry = 0;
	if (iOtherLastDigit < iThisLastDigit)
	{
		SetLastNonZeroDigit(iOtherLastDigit);

		iDiff = GetDigitsBetween(iThisLastDigit, iOtherLastDigit) - 1;
		iThisIndex = PrivateGetIndex(iOtherLastDigit);
		for (iOtherIndex = iOtherLastIndex; iOtherIndex > iOtherLastIndex - iDiff; iOtherIndex--, iThisIndex--)
		{
			cThisDigit = 10;
			cOtherDigit = pcNumber->mcDigits[iOtherIndex];
			cResult = cThisDigit - (cOtherDigit + cCarry);

			if (cResult >= 10)
			{
				cCarry = 0;
				cResult -= 10;
			}
			else
			{
				cCarry = 1;
			}
			mcDigits[iThisIndex] = cResult;
		}
	}

	// xxxxxxxxxx.xxxxxxxxxxxxx
	// 00yyyyyyyy.yyyyyyyyyyy00
	else if (iThisLastDigit < iOtherLastDigit)
	{
		iDiff = iOtherLastDigit - iThisLastDigit;
		iOtherIndex = pcNumber->PrivateGetIndex(iThisLastDigit);
		for (iThisIndex = iThisLastIndex; iThisIndex > iThisLastIndex - iDiff; iOtherIndex--, iThisIndex--)
		{
			cThisDigit = mcDigits[iThisIndex] + 10;
			cOtherDigit = 0;
			cResult = cThisDigit - (cOtherDigit + cCarry);

			if (cResult >= 10)
			{
				cCarry = 0;
				cResult -= 10;
			}
			else
			{
				cCarry = 1;
			}
			mcDigits[iThisIndex] = cResult;
		}
	}
	else
	{
		iOtherIndex = iOtherLastIndex;
		iThisIndex = iThisLastIndex;
	}

	// xxxxxxxxxxx.xxxxxxxxQQQQ
	// 00yyyyyyyyy.yyyyyyyyQQQQ
	for (; iOtherIndex >= iOtherFirstIndex; iOtherIndex--, iThisIndex--)
	{
		cThisDigit = mcDigits[iThisIndex] + 10;
		cOtherDigit = pcNumber->mcDigits[iOtherIndex];
		cResult = cThisDigit - (cOtherDigit + cCarry);

		if (cResult >= 10)
		{
			cCarry = 0;
			cResult -= 10;
		}
		else
		{
			cCarry = 1;
		}
		mcDigits[iThisIndex] = cResult;
	}

	for (; iThisIndex >= iThisFirstIndex; iOtherIndex--, iThisIndex--)
	{
		cThisDigit = mcDigits[iThisIndex] + 10;
		cOtherDigit = 0;
		cResult = cThisDigit - (cOtherDigit + cCarry);

		if (cResult >= 10)
		{
			cCarry = 0;
			cResult -= 10;
		}
		else
		{
			cCarry = 1;
		}
		mcDigits[iThisIndex] = cResult;
	}

	Clean();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::PrivateGetIndex(int iDigit)
{
	//Assumes iDigit != 0.
	if (iDigit > 0)
	{
		return mcMaxWholeNumbers-iDigit;
	}
	else
	{
		return mcMaxWholeNumbers-iDigit-1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Multiply(CNumber* pcMultiplicand)
{
	char		cCarry;
	int			iThisLastDigit;
	int			iMultiplicandLastDigit;
	int			iThisFirstDigit;
	int			iMultiplicandFirstDigit;
	char		cThisDigit;
	char		cOtherDigit;
	int			i;
	int			j;
	int			iResultantDigit;
	int			iThisDigit;
	int			iOtherDigit;
	char		cResult;
	CNumber*	pcResult;
	CNumber*	pcLine;
	int			iFirstSignificantDigit;
	int			iDecimals;

	if (PrivateError(pcMultiplicand)) return this;

	if ((pcMultiplicand->IsZero()) || (IsZero()))
	{
		return Zero(mcMaxWholeNumbers, mcMaxDecimals);
	}

	iThisLastDigit = GetLastNonZeroDigit();
	iThisFirstDigit = GetFirstNonZeroDigit();

	iMultiplicandLastDigit = pcMultiplicand->GetLastNonZeroDigit();
	iMultiplicandFirstDigit = pcMultiplicand->GetFirstNonZeroDigit();

	iDecimals = mcMaxDecimals + 3;

	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	if (!pcResult)
	{
		return NULL;
	}

	pcLine = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcResult->Init(mcMaxWholeNumbers, iDecimals);

	for (j = iMultiplicandLastDigit; j <= iMultiplicandFirstDigit; j++)
	{
		if (j != 0)
		{
			iOtherDigit = j;
			if (j > 0)
			{
				iOtherDigit--;
			}
			cOtherDigit = pcMultiplicand->GetDigit(j);

			cCarry = 0;

			iResultantDigit = (iThisFirstDigit > 0 ? iThisFirstDigit-1 : iThisFirstDigit) + iOtherDigit;
			iResultantDigit = iResultantDigit >= 0 ? iResultantDigit+1 : iResultantDigit;
			if (iResultantDigit > mcMaxWholeNumbers)
			{
				gcNumberControl.Remove(1+1);
				return Overflow(GetSign()*pcMultiplicand->GetSign());
			}
			else
			{
				iFirstSignificantDigit = iResultantDigit;
			}
			pcLine->Init(mcMaxWholeNumbers, iDecimals);
			iResultantDigit = (iThisLastDigit > 0 ? iThisLastDigit-1 : iThisLastDigit) + iOtherDigit;
			iResultantDigit = iResultantDigit >= 0 ? iResultantDigit+1 : iResultantDigit;
			if (iResultantDigit < -iDecimals)
			{
				pcLine->SetFlag(NUMBER_FLAGS_UNDERFLOW);
				pcLine->SetLastNonZeroDigit(-iDecimals);
			}
			else
			{
				pcLine->SetLastNonZeroDigit(iResultantDigit);
			}

			for (i = iThisLastDigit; i <= iThisFirstDigit; i++)
			{
				if (i != 0)
				{
					iThisDigit = i;
					if (i > 0)
					{
						iThisDigit--;
					}

					iResultantDigit = iThisDigit+iOtherDigit;
					if (iResultantDigit >= 0)
					{
						iResultantDigit++;
					}

					if (iResultantDigit >= -iDecimals)
					{
						cThisDigit = GetDigit(i);
						cResult = cThisDigit*cOtherDigit + cCarry;
						cCarry = cResult/10;
						cResult %= 10;
						pcLine->SetDigitUnsafe(iResultantDigit, cResult);
					}
				}
			}

			if (cCarry != 0)
			{
				iFirstSignificantDigit = OffsetDigit(iFirstSignificantDigit, 1);
				if (iFirstSignificantDigit > mcMaxWholeNumbers)
				{
					gcNumberControl.Remove(1+1);
					return Overflow(GetSign()*pcMultiplicand->GetSign());
				}
				pcLine->SetDigitUnsafe(iFirstSignificantDigit, cCarry);
			}
			pcLine->SetFirstNonZerotDigit(iFirstSignificantDigit);

			pcResult->Add(pcLine);
			if (pcResult->IsOverflow())
			{
				gcNumberControl.Remove(1+1);
				return Overflow(GetSign()*pcMultiplicand->GetSign());
			}
		}
	}

	pcResult->SetSign(GetSign()*pcMultiplicand->GetSign());
	pcResult->Clean();
	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(1+1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Divide(CNumber* pcDivisorIn)
{
	CNumber*	pcDivisor;
	CNumber*	pcPartialDividend;
	char		cThisDigit;
	char		cPartialQuotient;
	CNumber*	pcQuotient;
	int			iDividendDigit;
	int			iQuotientDigit;
	BOOL		bExact;
	int			iLastDigit;
	int			iLastDivisorDigit;
	int			iDecimals;

	if (PrivateError(pcDivisorIn)) return this;
	if (pcDivisorIn->IsZero()) return DivisionByZero();

	iDecimals = mcMaxDecimals + 3;

	pcDivisor = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcPartialDividend = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcQuotient = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcDivisor->Init(pcDivisorIn, mcMaxWholeNumbers, iDecimals);
	pcDivisor->Absolute();
	iLastDivisorDigit = pcDivisor->GetLastNonZeroDigit();

	iDividendDigit = GetFirstNonZeroDigit();
	iLastDigit = GetLastNonZeroDigit();
	cThisDigit = GetDigit(iDividendDigit);
	pcPartialDividend->Digit(iLastDivisorDigit, cThisDigit, mcMaxWholeNumbers, iDecimals);

	if (iLastDivisorDigit > 0)
	{
		iLastDivisorDigit--;
	}
	iQuotientDigit = OffsetDigit(iDividendDigit, -iLastDivisorDigit);
	iLastDivisorDigit = pcDivisor->GetLastNonZeroDigit();

	pcQuotient->Init(mcMaxWholeNumbers, iDecimals);
	for (;;)
	{
		cPartialQuotient = pcPartialDividend->PrivateDivide(pcDivisor, &bExact);

		if (!((iQuotientDigit > mcMaxWholeNumbers) && (cPartialQuotient == 0)))
		{
			pcQuotient->SetDigit(iQuotientDigit, cPartialQuotient);
			if (pcQuotient->IsOverflow())
			{
				gcNumberControl.Remove(3);
				return Overflow(GetSign()*pcDivisorIn->GetSign());
			}
		}

		iDividendDigit = OffsetDigit(iDividendDigit, -1);
		iQuotientDigit = OffsetDigit(iQuotientDigit, -1);

		if (iQuotientDigit < -iDecimals)
		{
			pcQuotient->SetFlag(NUMBER_FLAGS_UNDERFLOW);
			break;
		}

		cThisDigit = GetDigit(iDividendDigit);
		if (bExact)
		{
			if (iDividendDigit < iLastDigit)
			{
				break;
			}
			pcPartialDividend->Digit(iLastDivisorDigit, cThisDigit, mcMaxWholeNumbers, iDecimals);
		}
		else
		{
			pcPartialDividend->Shift(1);
			pcPartialDividend->SetDigit(iLastDivisorDigit, cThisDigit);
		}
	}

	pcQuotient->SetSign(GetSign()*pcDivisorIn->GetSign());
	pcQuotient->Clean();
	Init(pcQuotient, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(3);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::PrivateDivide(CNumber* pcDivisor, BOOL* pbExact)
{
	char	cDigit;

	//"this" will contain the remainder.

	cDigit = 0;
	for (;;)
	{
		if (pcDivisor->GreaterThan(this))
		{
			if (IsZero())
			{
				*pbExact = TRUE;
				return cDigit;
			}
			else
			{
				*pbExact = FALSE;
				return cDigit;
			}
		}
		cDigit++;

		Subtract(pcDivisor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateUnderflow(CNumber* pcNumber)
{
	if (pcNumber->IsUnderflow())
	{
		this->SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateNAN(CNumber* pcNumber)
{
	if (pcNumber->IsNAN())
	{
		NotANumber();
		return TRUE;
	}
	else if (IsNAN())
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateDivisionByZero(CNumber* pcNumber)
{
	if (pcNumber->IsDivisionByZero())
	{
		DivisionByZero();
		return TRUE;
	}
	else if (IsDivisionByZero())
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNumber::PrivateOverflow(CNumber* pcNumber)
{
	if (pcNumber->IsOverflow() && IsOverflow())
	{
		if (IsNegative() != pcNumber->IsNegative())
		{
			NotANumber();
		}
		return TRUE;
	}
	else if (pcNumber->IsOverflow())
	{
		Overflow(pcNumber->GetSign());
		return TRUE;
	}
	else if (IsOverflow())
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateIntegerFactorial(int iN, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	CNumber*	pcI;
	int			i;

	if (iN < 0)
	{
		NotANumber();
		return;
	}

	One(cMaxWholeNumbers, cMaxWholeNumbers);
	if (iN == 1)
	{
		return;
	}

	pcI = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	pcI->One(mcMaxWholeNumbers, mcMaxDecimals);
	for (i = 1; i <= iN; i++)
	{
		Multiply(pcI);
		if (IsOverflow())
		{
			return;
		}
		pcI->Increment();
	}
	gcNumberControl.Remove();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateIntegerRoot(CNumber* pcRoot)
{
	CNumber* 	pcX_k;
	CNumber* 	pcX_k1;
	CNumber* 	pcOne;
	CNumber* 	pcOneOverRoot;
	CNumber* 	pcRootMinusOne;
	CNumber* 	pcX_kTimesRootMinusOne;
	CNumber* 	pcX_kToPowerRootMinusOne;
	CNumber* 	pcBaseOver;
	int			iIterations;
	int			iDecimals;
	CNumber*	pcThis;

	iDecimals = mcMaxDecimals + 3;

	pcX_k = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcX_k1 = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcOneOverRoot = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcRootMinusOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcX_kTimesRootMinusOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcX_kToPowerRootMinusOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcBaseOver = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcThis = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcThis->Init(this, mcMaxWholeNumbers, iDecimals);
	pcOne->One(mcMaxWholeNumbers, iDecimals);
	pcX_k->One(mcMaxWholeNumbers, iDecimals);

	pcRootMinusOne->Init(pcRoot, mcMaxWholeNumbers, iDecimals)->Decrement();
	pcOneOverRoot->Init(pcRoot, mcMaxWholeNumbers, iDecimals)->Inverse();

	for (iIterations = 0; ; iIterations++)
	{
		pcX_kTimesRootMinusOne->Init(pcX_k, mcMaxWholeNumbers, iDecimals)->Multiply(pcRootMinusOne);
		pcX_kToPowerRootMinusOne->Init(pcX_k, mcMaxWholeNumbers, iDecimals)->PrivateIntegerExponent(pcRootMinusOne);
		pcBaseOver->Init(pcThis, mcMaxWholeNumbers, iDecimals)->Divide(pcX_kToPowerRootMinusOne);
		pcX_k1->Init(pcBaseOver, mcMaxWholeNumbers, iDecimals)->Add(pcX_kTimesRootMinusOne);
		pcX_k1->Multiply(pcOneOverRoot);
		if (pcX_k1->PrivateEquals(pcX_k, mcMaxDecimals))
		{
			Init(pcX_k1, mcMaxWholeNumbers, mcMaxDecimals);
			break;
		}
		pcX_k->Init(pcX_k1, mcMaxWholeNumbers, iDecimals);
	}

	gcNumberControl.Remove(9);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateCopy(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int iOtherFirst;
	int	iOtherLast;
	int iLength;

	//This function assumes that pcNumber is a different size to this.
	mcMaxWholeNumbers = cMaxWholeNumbers;
	mcMaxDecimals = cMaxDecimals;
	mcFlags = 0;
	if (pcNumber->IsError())
	{
		mcFlags = pcNumber->mcFlags;
		return;
	}

	iOtherFirst = pcNumber->GetFirstNonZeroDigit();
	iOtherLast = pcNumber->GetLastNonZeroDigit();

	if (iOtherFirst > mcMaxWholeNumbers)
	{
		Overflow(pcNumber->GetSign());
		return;
	}

	if (iOtherLast < -(mcMaxDecimals))
	{
		iOtherLast = -(mcMaxDecimals);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}

	iLength = GetDigitsBetween(iOtherFirst, iOtherLast);
	memcpy_fast(DigitToArray(iOtherFirst), pcNumber->DigitToArray(iOtherFirst), iLength);

	SetFirstNonZerotDigit(iOtherFirst);
	SetLastNonZeroDigit(iOtherLast);

	SetSign(pcNumber->GetSign());
	if (pcNumber->IsUnderflow())
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}
	Clean();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateIntegerExponent(int iExponent)
{
	CNumber*	pcResult;
	CNumber*	pcTwo;
	CNumber*	pcThis;
	int			iDecimals;

	if (iExponent == 0)
	{
		One(mcMaxWholeNumbers, mcMaxDecimals);
		return;
	}
	else if (iExponent == 1)
	{
		return;
	}
	else if (iExponent == 2)
	{
		Squared();
		return;
	}
	else if (iExponent == 3)
	{
		Cubed();
		return;
	}
	else if (iExponent > 0)
	{
		iDecimals = mcMaxDecimals + 3;

		pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
		pcTwo = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
		pcThis = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

		pcResult->One(mcMaxWholeNumbers, iDecimals);
		pcTwo->Init(2, mcMaxWholeNumbers, iDecimals);
		pcThis->Init(this, mcMaxWholeNumbers, iDecimals);
		while (iExponent)
		{
			if (iExponent & 1)
			{
				pcResult->Multiply(pcThis);
				iExponent--;
			}
			pcThis->Squared();
			iExponent /= 2;
		}
		Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
		gcNumberControl.Remove(1 + 1);
	}
	else
	{
//		gcLogger.Error();
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateIntegerExponent(CNumber* pcExponentIn)
{
	CNumber*	pcResult;
	CNumber*	pcExponent;
	CNumber*	pcTwo;
	int			n;

	if (pcExponentIn->GetFirstNonZeroDigit() <= 9)
	{
		n = pcExponentIn->IntValue();
		PrivateIntegerExponent(n);
	}
	else
	{
		pcResult = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
		pcExponent = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
		pcTwo = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);

		pcExponent->Init(pcExponentIn, mcMaxWholeNumbers, mcMaxDecimals);
		pcResult->One(mcMaxWholeNumbers, mcMaxDecimals);
		pcTwo->Init(2, mcMaxWholeNumbers, mcMaxDecimals);
		while (!pcExponent->IsZero())
		{
			if (pcExponent->IsOdd())
			{
				pcResult->Multiply(this);
				pcExponent->Decrement();
			}
			Squared();
			pcExponent->Divide(pcTwo);
		}
		Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
		gcNumberControl.Remove(3);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Modulus(CNumber* pcNumber)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Inverse(void)
{
	CNumber*	pcOne;

	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	pcOne->One(mcMaxWholeNumbers, mcMaxDecimals)->Divide(this);
	Copy(pcOne, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Negate(void)
{
	if (IsNegative())
	{
		SetSign(1);
	}
	else
	{
		SetSign(-1);
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Absolute(void)
{
	ClearFlag(NUMBER_FLAGS_NEGATIVE);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Decrement(void)
{
	CNumber*	pcOne;

	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Subtract(pcOne->One(mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::LogicalNot(void)
{
	CNumber*	pcZero;

	pcZero = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	if (Equals(pcZero->Zero()))
	{
		One();
	}
	else
	{
		Zero();
	}
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::LogicalOr(CNumber* pcRight)
{
	CNumber*	pcZero;
	BOOL		bLeftZero;
	BOOL		bRightZero;

	pcZero = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	bLeftZero = Equals(pcZero->Zero());
	bRightZero = pcRight->Equals(pcZero);
	if (bLeftZero && bRightZero)
	{
		Zero();
	}
	else
	{
		One();
	}
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::LogicalAnd(CNumber* pcRight)
{
	CNumber*	pcZero;
	BOOL		bLeftZero;
	BOOL		bRightZero;

	pcZero = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	bLeftZero = Equals(pcZero->Zero());
	bRightZero = pcRight->Equals(pcZero);
	if (bLeftZero || bRightZero)
	{
		Zero();
	}
	else
	{
		One();
	}
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Increment(void)
{
	CNumber*	pcOne;

	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Add(pcOne->One(mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Power(CNumber* pcExponentIn)
{
	CNumber*	pcResult;
	int			iDecimals;
	CNumber*	pcExponent;

	if (pcExponentIn->IsInteger() && pcExponentIn->IsPositiveOrZero())
	{
		PrivateIntegerExponent(pcExponentIn);
		return this;
	}

	iDecimals = mcMaxDecimals*2;
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcExponent = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcExponent->Copy(pcExponentIn, mcMaxWholeNumbers, iDecimals);

	pcResult->Init(this, mcMaxWholeNumbers, iDecimals);
	pcResult->NaturalLogarithm();
	pcResult->Multiply(pcExponent);
	pcResult->Exponential();

	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(1+1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Root(CNumber* pcRootIn)
{
	CNumber*	pcResult;
	CNumber*	pcRoot;
	int			iDecimals;

	if (pcRootIn->IsInteger())
	{
		PrivateIntegerRoot(pcRootIn);
		return this;
	}

	iDecimals = mcMaxDecimals*2;
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcRoot = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcResult->Init(this, mcMaxWholeNumbers, iDecimals);
	pcRoot->Init(pcRootIn, mcMaxWholeNumbers, iDecimals);
	pcResult->NaturalLogarithm();
	pcResult->Divide(pcRoot);
	pcResult->Exponential();

	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(1+1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Antilog(CNumber* pcBase)
{
	CNumber*	pcResult;

	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);

	pcResult->Init(pcBase, mcMaxWholeNumbers, mcMaxDecimals);
	pcResult->Power(this);

	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::ArithmeticGeometricMean(CNumber* pcOther)
{
	CNumber*	pcAn;
	CNumber*	pcGn;
	CNumber*	pcAn_1;
	CNumber*	pcGn_1;
	int			iDecimals;
	CNumber*	pcTwo;

	iDecimals = mcMaxDecimals;
	pcAn = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcGn = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcAn_1 = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcGn_1 = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcTwo = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcTwo->Init(2, mcMaxWholeNumbers, iDecimals);

	pcAn->Init(this, mcMaxWholeNumbers, iDecimals);
	pcGn->Init(pcOther, mcMaxWholeNumbers, iDecimals);

	for (;;)
	{
		pcAn_1->Init(pcAn, mcMaxWholeNumbers, mcMaxDecimals);
		pcAn_1->Add(pcGn);
		pcAn_1->Divide(pcTwo);

		pcGn_1->Init(pcAn, mcMaxWholeNumbers, mcMaxDecimals);
		pcGn_1->Multiply(pcGn);
		pcGn_1->SquareRoot();

		if (pcAn_1->Equals(pcGn_1))
		{
			break;
		}

		pcAn->Init(pcAn_1, mcMaxWholeNumbers, iDecimals);
		pcGn->Init(pcGn_1, mcMaxWholeNumbers, iDecimals);
	}

	this->Copy(pcAn_1, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(5);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Exponential(void)  //e^this
{
	//Also	=	cosh(x)+sinh(x)
	int			i;
	CNumber*	pcFractional;
	CNumber*	pcIntegral;
	CNumber*	pcOne;
	CNumber*	pcPart;
	CNumber*	pcNumber;
	CNumber*	pcE;
	CNumber*	pcResult;
	int			iDecimals;

	iDecimals = mcMaxDecimals+3;
	pcFractional = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcIntegral = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcResult->Init(this, mcMaxWholeNumbers, iDecimals);
	pcIntegral->Init(this, mcMaxWholeNumbers, iDecimals);
	pcFractional->Init(this, mcMaxWholeNumbers, iDecimals);
	pcFractional->Fractional();
	pcIntegral->Integer();
	pcOne->One(mcMaxWholeNumbers, iDecimals);

	if (IsZero())
	{
		gcNumberControl.Remove(4);
		return One(mcMaxWholeNumbers, mcMaxDecimals);
	}

	pcPart = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcNumber = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcE = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcResult->One(mcMaxWholeNumbers, iDecimals);
	for (i = iDecimals; i >= 1; i--)
	{
		pcPart->Init(pcFractional, mcMaxWholeNumbers, iDecimals);
		pcNumber->Init(i, mcMaxWholeNumbers, iDecimals);
		pcPart->Divide(pcNumber);
		pcResult->Multiply(pcPart);
		pcResult->Increment();
	}

	pcE->E(mcMaxWholeNumbers, iDecimals);
	if (!pcIntegral->IsNegative())
	{
		pcE->PrivateIntegerExponent(pcIntegral);
	}
	else
	{
		pcE->PrivateIntegerExponent(pcIntegral->Absolute());
		pcE->Inverse();
	}
	pcResult->Multiply(pcE);
	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(7);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Logarithm(CNumber* pcBase)
{
	//Read the wiki for a faster implementation.

	CNumber*	pcOne;
	int			iInteger;
	CNumber*	pcInverseBase;
	CNumber*	pcDecimal;
	CNumber*	pcPartial;
	CNumber*	pcPointFive;
	CNumber*	pcResult;
	int			i;
	int			iSign;
	int			iDecimals;

	if (PrivateError(pcBase))
	{
		return this;
	}
	if ((IsZero()) || (IsNegative()))
	{
		return NotANumber();
	}

	iDecimals = mcMaxDecimals;

	pcOne = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcOne->One(mcMaxDecimals, iDecimals);
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcResult->Copy(this, mcMaxWholeNumbers, iDecimals);

	if (pcResult->Equals(pcOne))
	{
		gcNumberControl.Remove(1+1);
		return Zero(mcMaxWholeNumbers, mcMaxDecimals);
	}

	iSign = 1;
	if (pcResult->LessThan(pcOne))
	{
		iSign = -1;
		pcResult->Inverse();
	}

	iInteger = 0;
	if (pcBase->LessThan(pcOne))
	{
		gcNumberControl.Remove(1+1);
		return NotANumber();
	}

	pcInverseBase = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcInverseBase->Init(pcBase, mcMaxWholeNumbers, iDecimals);
	pcInverseBase->Inverse();

	while (pcResult->LessThan(pcOne))
	{
		iInteger--;
		pcResult->Multiply(pcBase);
	}

	while (pcResult->GreaterThanOrEquals(pcBase))
	{
		iInteger++;
		pcResult->Multiply(pcInverseBase);
	}

	pcPointFive = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcPartial = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcDecimal = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcPointFive->Init("0.5", mcMaxWholeNumbers, iDecimals);
	pcPartial->Init(pcPointFive, mcMaxWholeNumbers, iDecimals);
	pcDecimal->Zero(mcMaxWholeNumbers, iDecimals);
	pcResult->Squared();

	for (i = 0;; i++)
	{
		if (pcResult->GreaterThanOrEquals(pcBase))
		{
			pcDecimal->Add(pcPartial);
			pcResult->Divide(pcBase);
		}
		pcPartial->Multiply(pcPointFive);
		if (pcPartial->IsZero() || (pcPartial->GetFirstNonZeroDigit() <= -mcMaxDecimals))
		{
			break;
		}
		pcResult->Squared();
	}
	pcResult->Init(iInteger, mcMaxWholeNumbers, iDecimals);
	pcResult->Add(pcDecimal);
	pcResult->SetSign(iSign);
	Copy(pcResult, mcMaxWholeNumbers, mcMaxDecimals);

	gcNumberControl.Remove(6);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::NaturalLogarithm(void)
{
	CNumber*	pcE;
	int			iDecimals;
	CNumber*	pcResult;

	//3 seems to be the minimum
	iDecimals = mcMaxDecimals + 3;

	//There is nothing scientific behind these ifs.
	if (iDecimals > 50)
	{
		iDecimals++;
		if (iDecimals > 100)
		{
			iDecimals++;
		}
	}
	pcE = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcE->E(mcMaxWholeNumbers, iDecimals);
	pcResult->Copy(this, mcMaxWholeNumbers, iDecimals);
	pcResult->Logarithm(pcE);
	Copy(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(1+1);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::CommonLogarithm(void)
{
	CNumber*	pcTen;

	pcTen = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Logarithm(pcTen->Init(10, mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::BinaryLogarithm(void)
{
	CNumber*	pcTwo;

	pcTwo = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Logarithm(pcTwo->Init(2, mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::SquareRoot(void)
{
	CNumber*	pcTwo;

	pcTwo = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	PrivateIntegerRoot(pcTwo->Init(2, mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::CubeRoot(void)
{
	CNumber*	pcThree;

	pcThree = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	PrivateIntegerRoot(pcThree->Init(3, mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Squared(void)
{
	CNumber*	pcTemp;

	pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Multiply(pcTemp->Init(this, mcMaxWholeNumbers, mcMaxDecimals));
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Cubed(void)
{
	CNumber*	pcTemp;

	pcTemp = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	Multiply(pcTemp->Init(this, mcMaxWholeNumbers, mcMaxDecimals));
	Multiply(pcTemp);
	gcNumberControl.Remove();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Sine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Cosine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Tangent(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::ArcSine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::ArcCosine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::ArcTangent(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::HyperbolicSine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::HyperbolicCosine(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::HyperbolicTangent(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::DegreesToRadians(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::RadiansToDegrees(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNumber::IntValue(void)
{
	int		iFirst;
	int		i;
	int		iTimes;
	int		iValue;
	int		iResult;

	iFirst = GetFirstNonZeroDigit();

	if ((iFirst < 0) || (IsZero()))
	{
		return 0;
	}

	iTimes = 1;
	iResult = 0;

	for (i = 1; i <= iFirst; i++)
	{
		iValue = GetDigit(i);
		iResult += (iValue*iTimes);
		iTimes *= 10;
	}
	iResult *= GetSign();

	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CNumber::FloatValue(void)
{
	//int		iFirst;
	//int		i;
	//int		iTimes;
	//int		iValue;
	//int		iResult;
	//int		iLast;

	//iFirst = GetFirstNonZeroDigit();
	//iLast = GetLastNonZeroDigit();

	//if (IsZero())
	//{
	//	return 0;
	//}

	//iTimes = 1;
	//iResult = 0;

	//for (i = iLast; i <= iFirst; i++)
	//{
	//	if (i != 0)
	//	{
	//		iValue = GetDigit(i);
	//		iResult += (iValue*iTimes);
	//		iTimes *= 10;
	//	}
	//}
	//iResult *= GetSign();

	//return iResult;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::SafeGetDigit(int iDigit)
{
	if (iDigit <= mcFirstDigit && iDigit >= mcLastDigit)
	{
		if ((iDigit > 0) && (iDigit <= mcMaxWholeNumbers))
		{
			return mcDigits[mcMaxWholeNumbers - iDigit];
		}
		else if ((iDigit >= -mcMaxDecimals) && (iDigit < 0))
		{
			return mcDigits[mcMaxWholeNumbers - iDigit - 1];
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::GetDigit(int iDigit)
{
	if ((iDigit > 0) && (iDigit <= mcMaxWholeNumbers))
	{
		return mcDigits[mcMaxWholeNumbers-iDigit];
	}
	else if ((iDigit >= -mcMaxDecimals) && (iDigit < 0))
	{
		return mcDigits[mcMaxWholeNumbers-iDigit-1];
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetDigit(int iDigit, char cValue)
{
	int		iFirstDigit;
	int		iLastDigit;
	char*	pcValue;
	BOOL	bWasZero;

	if (iDigit > mcMaxWholeNumbers)
	{
		Overflow(GetSign());
	}
	else if (iDigit < -mcMaxDecimals)
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}
	else if (iDigit == 0)
	{
		if (cValue != '.')
		{
			SetFlag(NUMBER_FLAGS_NAN);
		}
	}
	else
	{
		if ((cValue >= 1) && (cValue <= 9))
		{
			bWasZero = IsZero();
			pcValue = DigitToArray(iDigit);
			*pcValue = cValue;

			iFirstDigit = GetFirstNonZeroDigit();
			if (iDigit > iFirstDigit)
			{
				SetFirstNonZerotDigit(iDigit);
			}
			iLastDigit = GetLastNonZeroDigit();
			if (iDigit < iLastDigit)
			{
				SetLastNonZeroDigit(iDigit);
			}

			if (bWasZero)
			{
				Clean();
			}
		}
		else if (cValue == 0)
		{
			pcValue = DigitToArray(iDigit);
			*pcValue = cValue;

			iFirstDigit = GetFirstNonZeroDigit();
			if (iDigit == iFirstDigit)
			{
				CleanRight();
			}
			iLastDigit = GetLastNonZeroDigit();
			if (iDigit == iLastDigit)
			{
				CleanLeft();
			}
		}
		else
		{
			NotANumber();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetDigitUnsafe(int iDigit, char cValue)
{
	char*	pcValue;

	pcValue = DigitToArray(iDigit);
	*pcValue = cValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CNumber::DigitToArray(int iDigit)
{
	if (iDigit > 0)
	{
		return &mcDigits[mcMaxWholeNumbers-iDigit];
	}
	else if (iDigit < 0)
	{
		return &mcDigits[mcMaxWholeNumbers-iDigit-1];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Print(CChars* pcChars)
{
	int		iFirstDigit;
	int		iLastDigit;
	int		i;
	char	c;
	int		iStart;
	int		iStop;

	if (IsNAN())
	{
		pcChars->Append("Not a number");
		return;
	}
	if (IsDivisionByZero())
	{
		pcChars->Append("Division by zero");
		return;
	}

	if (IsOverflow())
	{
		if (!IsNegative())
		{
			pcChars->Append("Positive ");
		}
		else
		{
			pcChars->Append("Negative ");
		}
		pcChars->Append("overflow");
		return;
	}

	if (IsNegative())
	{
		pcChars->Append("-");
	}

	iFirstDigit = GetFirstNonZeroDigit();
	iLastDigit = GetLastNonZeroDigit();

	iStart = iFirstDigit;
	if (iFirstDigit < 0)
	{
		iStart = 1;
	}

	iStop = iLastDigit;
	if (iLastDigit > 0)
	{
		iStop = 1;
	}

	for (i = iStart; i >= iStop; i--)
	{
		if (i != 0)
		{
			c = '0';
			if ((i <= iFirstDigit) && (i >= iLastDigit))
			{
				c = GetDigit(i) + '0';
			}
			pcChars->Append(c);
		}
		else
		{
			pcChars->Append('.');
		}
	}

	if (!IsClean())
	{
		pcChars->Append(" (Unclean!)");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Dump(void)
{
	Dump(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Dump(BOOL bNewLine)
{
	CChars		sz;

	sz.Init();
	Print(&sz);
	if (bNewLine)
	{
		sz.AppendNewLine();
	}
	sz.Dump();
	sz.Kill();
}

