/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "DataTypes.h"
#include <string.h>
#include "IntegerHelper.h"
#include "FloatHelper.h"
#include "NumberControl.h"
#include "StackMemory.h"


#define ZERO '0'
#define NINE '9'
#define DOT '.'


//The bits of a word's nibbles from nibble n up, and from nibble 0 up to nibble n (constant masks: a W65C816 shifts by
//a variable count a bit at a time).
static const uint16 gauiNumberNibblesFrom[4] = { 0xFFFF, 0xFFF0, 0xFF00, 0xF000 };
static const uint16 gauiNumberNibblesTo[4] = { 0x000F, 0x00FF, 0x0FFF, 0xFFFF };


//////////////////////////////////////////////////////////////////////////
//
// BCDAdd, or BCDSubtract when bSubtract.
//
//////////////////////////////////////////////////////////////////////////
static uint16 NumberBCD(uint16* pauiDest, uint16* pauiSource, size uiWords, uint16 uiCarry, bool bSubtract)
{
	return bSubtract ? BCDSubtract(pauiDest, pauiSource, uiWords, uiCarry) : BCDAdd(pauiDest, pauiSource, uiWords, uiCarry);
}


//////////////////////////////////////////////////////////////////////////
//
// The word uiHigh moved up iNibbles nibbles (0 to 3) with the top of uiLow
// coming in below it.  The shifts are constants (a W65C816 shifts by a
// variable count a bit at a time).
//
//////////////////////////////////////////////////////////////////////////
static uint16 NumberCombineNibbles(uint16 uiHigh, uint16 uiLow, int16 iNibbles)
{
	switch (iNibbles)
	{
	case 1:
		return (uint16)((uiHigh << 4) | (uiLow >> 12));
	case 2:
		return (uint16)((uiHigh << 8) | (uiLow >> 8));
	case 3:
		return (uint16)((uiHigh << 12) | (uiLow >> 4));
	default:
		return uiHigh;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateInit(void)
{
	memset(this, 0, NUMBER_SIZE(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS));
	mcMaxWholeNumbers = DEFAULT_WHOLE_NUMBERS;
	mcMaxDecimals = DEFAULT_DECIMALS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateInit(size cWholeNumbers, size cMaxDecimals)
{
	memset(this, 0, NUMBER_SIZE(cWholeNumbers, cMaxDecimals));
	mcMaxWholeNumbers = (int16)cWholeNumbers;
	mcMaxDecimals = (int16)cMaxDecimals;
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
CNumber* CNumber::Init(int32 iNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int32		iRemainder;
	int16		iCount;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetLastNonZeroDigit(1);
	if (iNumber < 0)
	{
		iNumber = -iNumber;
		SetSign(-1);
	}
	else
	{
		SetSign(1);
	}

	iCount = 0;
	for (;;)
	{
		iRemainder = iNumber % 10;
		iNumber = iNumber / 10;
		if (iCount >= mcMaxWholeNumbers)
		{
			return Overflow(GetSign());
		}
		SetDigitUnsafe(iCount + 1, (char)iRemainder);
		iCount ++;
		if (iNumber == 0)
		{
			break;
		}
	}

	SetFirstNonZerotDigit(iCount);
	CleanRight();

	return this;
}


CNumber* CNumber::Init(int32 iNumber)
{
	return Init(iNumber, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
}


#ifdef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(int16 iNumber)
{
	return Init((int32)iNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(int16 iNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	return Init((int32)iNumber, cMaxWholeNumbers, cMaxDecimals);
}
#endif // __W65C816__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(float32 ff, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int32		i;
	int32		e;
	int32		f;
	int32		s;
	uint32		iCount;
	int32		iBitMask;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);

	i = *((int32*)((void*)&ff));
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
			iBitMask = (int32)1 << (22-iCount);
			if (f & iBitMask)
			{
				AddBinaryOne(e - (int32)iCount - 1);
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
CNumber* CNumber::Init(float32 ff)
{
	return Init(ff, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::BinaryOne(int16 iBinaryExponent, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	int32	iNumber;

	if ((iBinaryExponent < 32) && (iBinaryExponent >= 0))
	{
		iNumber = (int32)1 << iBinaryExponent;
		Init(iNumber, cMaxWholeNumbers, cMaxDecimals);
	}
	else if ((iBinaryExponent < 0) && (iBinaryExponent > -32))
	{
		iNumber = (int32)1 << (-iBinaryExponent);
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
CNumber* CNumber::Init(const char* szNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals, size iLen)
{
	CStackMemory<64>	cStack;
	char*	cDigits;
	int16	iSign;
	int16	iDecimalPoint;
	size	i;
	char	c;
	int16	iPos;
	bool	bAnyDigits;
	int16	iNonZero;

	PrivateInit(cMaxWholeNumbers, cMaxDecimals);

	if (iLen == 0)
	{
		iLen = StrLen(szNumber);
	}
	if (iLen >= MAX_INT16)
	{
		return NotANumber();
	}
	cDigits = (char*)cStack.Init(iLen + 1);  //Each digit, and a 0 before a leading point.  Usually on the stack.

	iPos = 0;
	bAnyDigits = false;
	iNonZero = -1;
	iDecimalPoint = 0;
	iSign = 1;
	for (i = 0; i < iLen; i++)
	{
		c = szNumber[i];
		if ((c >= ZERO) && (c <= NINE))
		{
			bAnyDigits = true;
			if ((c != ZERO) && (iNonZero == -1))
			{
				iNonZero = iPos;
			}
			cDigits[iPos] = c - ZERO;
			iPos++;
		}
		else if (c == DOT)
		{
			if (iDecimalPoint != 0)
			{
				cStack.Kill();
				SetFlag(NUMBER_FLAGS_NAN);
				return this;
			}
			if (!bAnyDigits)
			{
				cDigits[iPos] = 0;
				iPos++;
				bAnyDigits = true;
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
				cStack.Kill();
				SetFlag(NUMBER_FLAGS_NAN);
				return this;
			}
		}
	}

	if (!bAnyDigits)
	{
		cStack.Kill();
		return NotANumber();
	}

	if (iNonZero == -1)
	{
		cStack.Kill();
		Zero(mcMaxWholeNumbers, mcMaxDecimals);
		return this;
	}

	if (iDecimalPoint == 0)
	{
		iDecimalPoint = iPos;
	}

	if (iDecimalPoint - iNonZero > mcMaxWholeNumbers)
	{
		cStack.Kill();
		return Overflow(iSign);
	}

	if (iPos - iDecimalPoint > mcMaxDecimals)
	{
		//Only digits that aren't 0 are lost.
		for (i = (size)(iDecimalPoint + mcMaxDecimals); i < (size)iPos; i++)
		{
			if (cDigits[i] != 0)
			{
				SetFlag(NUMBER_FLAGS_UNDERFLOW);
				break;
			}
		}
		iPos = iPos - (iPos - iDecimalPoint - mcMaxDecimals);
	}

	for (i = (size)((iDecimalPoint > mcMaxWholeNumbers) ? (iDecimalPoint - mcMaxWholeNumbers) : 0); i < (size)iPos; i++)
	{
		//cDigits[0] is the digit iDecimalPoint (the digits after the point skip 0); leading zeros past the biggest
		//whole number are left out.
		SetDigitUnsafe((int16)(((int16)i < iDecimalPoint) ? (iDecimalPoint - (int16)i) : (iDecimalPoint - (int16)i - 1)), cDigits[i]);
	}
	cStack.Kill();
	SetFirstNonZerotDigit(iDecimalPoint);
	SetLastNonZeroDigit(-(iPos-iDecimalPoint));
	SetSign(iSign);

	Clean();
	return this;
}
CNumber* CNumber::Init(const char* szNumber, size iLen)
{
	return Init(szNumber, DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS, iLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Init(CNumber* pcNumber, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	if ((cMaxWholeNumbers == pcNumber->mcMaxWholeNumbers) && (cMaxDecimals == pcNumber->mcMaxDecimals))
	{
		memcpy(this, pcNumber, NUMBER_SIZE(cMaxWholeNumbers, cMaxDecimals));
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
	if ((DEFAULT_WHOLE_NUMBERS == pcNumber->mcMaxWholeNumbers) && (DEFAULT_DECIMALS == pcNumber->mcMaxDecimals))
	{
		memcpy(this, pcNumber, NUMBER_SIZE(DEFAULT_WHOLE_NUMBERS, DEFAULT_DECIMALS));
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
CNumber* CNumber::One(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive)
{
	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 1);
	SetPositive(bPositive);
	return this;
}
CNumber* CNumber::One(bool bPositive)
{
	PrivateInit();
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 1);
	SetPositive(bPositive);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Two(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive)
{
	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 2);
	SetPositive(bPositive);
	return this;
}
CNumber* CNumber::Two(bool bPositive)
{
	PrivateInit();
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 2);
	SetPositive(bPositive);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Half(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive)
{
	PrivateInit(cMaxWholeNumbers, cMaxDecimals);
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetDigitUnsafe(1, 2);
	SetPositive(bPositive);
	return this;
}
CNumber* CNumber::Half(bool bPositive)
{
	PrivateInit();
	SetFirstNonZerotDigit(-1);
	SetLastNonZeroDigit(-1);
	SetDigitUnsafe(-1, 5);
	SetPositive(bPositive);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Pi(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive)
{
	gcNumberControl.Pi(this, cMaxWholeNumbers, cMaxDecimals, bPositive);
	return this;
}
CNumber* CNumber::Pi(bool bPositive)
{
	gcNumberControl.Pi(this, bPositive);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::E(int16 cMaxWholeNumbers, int16 cMaxDecimals, bool bPositive)
{
	gcNumberControl.E(this, cMaxWholeNumbers, cMaxDecimals, bPositive);
	return this;
}
CNumber* CNumber::E(bool bPositive)
{
	gcNumberControl.E(this, bPositive);
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
CNumber* CNumber::Overflow(int16 iSign)
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
CNumber* CNumber::Digit(int16 iDigit, char cValue, int16 cMaxWholeNumbers, int16 cMaxDecimals)
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
CNumber* CNumber::Digit(int16 iDigit, char cValue)
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
void CNumber::AddBinaryOne(int16 iBinaryExponent)
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
void CNumber::SetSign(int16 iSign)
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
void CNumber::SetPositive(bool bPositive)
{
	if (bPositive)
	{
		ClearFlag(NUMBER_FLAGS_NEGATIVE);
	}
	else
	{
		SetFlag(NUMBER_FLAGS_NEGATIVE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::GetSign(void)
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
bool CNumber::IsZero(void)
{
	int16	iLast;
	int16	iFirst;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	if ((iFirst == 1) && (iLast == 1) && (GetDigitUnsafe(1) == 0))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsOdd(void)
{
	int16	iLast;
	char	cDigit;

	if (IsError())
	{
		return false;
	}

	iLast = GetLastNonZeroDigit();
	if (iLast > 1)
	{
		return false;
	}
	if (iLast <= -1)
	{
		return false;
	}
	cDigit = GetDigitUnsafe(1);
	return (cDigit % 2) == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsEven(void)
{
	int16	iLast;
	char	cDigit;

	if (IsError())
	{
		return false;
	}

	iLast = GetLastNonZeroDigit();
	if (iLast > 1)
	{
		return true;
	}
	if (iLast <= -1)
	{
		return false;
	}
	cDigit = GetDigitUnsafe(1);
	return (cDigit % 2) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsDivisionByZero(void)
{
	return mcFlags & NUMBER_FLAGS_DIVISION_BY_ZERO;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsError(void)
{
	if (IsOverflow())
	{
		return true;
	}
	if (IsNAN())
	{
		return true;
	}
	if (IsDivisionByZero())
	{
		return true;
	}
	return false;
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
int16 CNumber::GetDecimals(void)
{
	int16	iLast;

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
int16 CNumber::GetMaxWholeNumbers(void)
{
	return mcMaxWholeNumbers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::GetMaxDecimals(void)
{
	return mcMaxDecimals;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::CleanLeft(void)
{
	int16	iPosition;

	iPosition = PrivateFindNonZero(true);
	if (iPosition < 0)
	{
		PrivateNoDigits();
		return;
	}
	SetFirstNonZerotDigit(PositionDigit(iPosition));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::CleanRight(void)
{
	int16	iPosition;

	iPosition = PrivateFindNonZero(false);
	if (iPosition < 0)
	{
		PrivateNoDigits();
		return;
	}
	SetLastNonZeroDigit(PositionDigit(iPosition));
}


//////////////////////////////////////////////////////////////////////////
//
// Every digit is 0: zero's first and last digit and sign.
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateNoDigits(void)
{
	SetFirstNonZerotDigit(1);
	SetLastNonZeroDigit(1);
	SetSign(1);
}


//////////////////////////////////////////////////////////////////////////
//
// The position of the first digit that isn't 0 between the first and last
// non zero digits (the biggest whole number and smallest decimal at most):
// the highest (bDown) or the lowest.  A word that is 0 is passed over at
// once.  -1 if all are 0.
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::PrivateFindNonZero(bool bDown)
{
	int16	iFirst;
	int16	iLast;
	int16	iHigh;
	int16	iLow;
	int16	i;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();
	if (iFirst > mcMaxWholeNumbers)
	{
		iFirst = mcMaxWholeNumbers;
	}
	if (iLast < -mcMaxDecimals)
	{
		iLast = -mcMaxDecimals;
	}
	if (iFirst < iLast)
	{
		return -1;
	}
	iHigh = DigitPosition(iFirst);
	iLow = DigitPosition(iLast);
	if (bDown)
	{
		for (i = iHigh; i >= iLow;)
		{
			if (((i & 3) == 3) && (i - 3 >= iLow) && (mauiDigits[i >> 2] == 0))
			{
				i = (int16)(i - 4);
			}
			else if (GetNibble(i) != 0)
			{
				return i;
			}
			else
			{
				i--;
			}
		}
	}
	else
	{
		for (i = iLow; i <= iHigh;)
		{
			if (((i & 3) == 0) && (i + 3 <= iHigh) && (mauiDigits[i >> 2] == 0))
			{
				i = (int16)(i + 4);
			}
			else if (GetNibble(i) != 0)
			{
				return i;
			}
			else
			{
				i++;
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The digit in the nibble iPosition (DigitPosition's inverse).
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::PositionDigit(int16 iPosition)
{
	iPosition = (int16)(iPosition - NUMBER_WORD_DIGITS(mcMaxDecimals));
	return (int16)((iPosition >= 0) ? (iPosition + 1) : iPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Clean(void)
{
	PrivateFindEnds();
	PrivateZeroEnds();
}


//////////////////////////////////////////////////////////////////////////
//
// The first and last non zero digits found again (between the ones set).
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateFindEnds(void)
{
	CleanLeft();
	CleanRight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::OffsetDigit(int16 iCurrentDigit, int16 iOffset)
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
size CNumber::GetDigitsBetween(int16 iFirst, int16 iLast)
{
	if (iFirst < iLast)
	{
		return 0;
	}
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
void CNumber::RoundSignificant(int16 iSignificantDigits)
{
	int16		iDigits;
	int16		iDifference;
	int16		iLast;
	int16		iNewLast;
	char		cBeyondDigit;
	CNumber*	pcRound;
	int16		iBeyondLast;

	if (iSignificantDigits <= 0)
	{
		return;
	}

	iDigits = (int16)GetDigitsBetween();
	if (iDigits > iSignificantDigits)
	{
		iLast = GetLastNonZeroDigit();
		iDifference = iDigits - iSignificantDigits;
		iNewLast = OffsetDigit(iLast, iDifference);
		iBeyondLast = OffsetDigit(iNewLast, -1);
		cBeyondDigit = GetDigitUnsafe(iBeyondLast);
		if (cBeyondDigit >= 5)
		{
			pcRound = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
			pcRound->Digit(iNewLast, 1 * GetSign(), mcMaxWholeNumbers, mcMaxDecimals);
			Add(pcRound);
			gcNumberControl.Remove();
		}
		SetLastNonZeroDigit(iNewLast);
		PrivateZeroDigits(iBeyondLast, iLast);
		Clean();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateZeroDigits(int16 iFirst, int16 iLast)
{
	if (GetDigitsBetween(iFirst, iLast) != 0)
	{
		PrivateZeroPositions(DigitPosition(iFirst), DigitPosition(iLast));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The digits in the nibbles from iHigh down to iLow (DigitPosition) are 0.
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateZeroPositions(int16 iHigh, int16 iLow)
{
	int16	iLowWord;
	int16	iHighWord;
	uint16	i;

	if (iHigh < iLow)
	{
		return;
	}
	iLowWord = (int16)(iLow >> 2);
	iHighWord = (int16)(iHigh >> 2);
	if (iLowWord == iHighWord)
	{
		mauiDigits[iLowWord] &= (uint16)~(gauiNumberNibblesFrom[iLow & 3] & gauiNumberNibblesTo[iHigh & 3]);
		return;
	}
	mauiDigits[iLowWord] &= (uint16)~gauiNumberNibblesFrom[iLow & 3];
	mauiDigits[iHighWord] &= (uint16)~gauiNumberNibblesTo[iHigh & 3];
	for (i = (uint16)(iLowWord + 1); i < (uint16)iHighWord; i++)
	{
		mauiDigits[i] = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateZeroEnds(void)
{
	int16	iFirst;
	int16	iLast;

	iFirst = OffsetDigit(GetFirstNonZeroDigit(), 1);
	if (iFirst <= mcMaxWholeNumbers)
	{
		PrivateZeroPositions(DigitPosition(mcMaxWholeNumbers), DigitPosition(iFirst));
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
bool CNumber::IsInteger(void)
{
	int16		iLast;

	iLast = GetLastNonZeroDigit();
	if (iLast > 0)
	{
		return true;
	}
	return false;
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
CNumber* CNumber::Shift(int16 iOffset)
{
	int16	iFirst;
	int16	iLast;
	int16	iOldFirst;
	int16	iOldLast;

	if (iOffset == 0)
	{
		return this;
	}

	if (IsOverflow() || IsNAN())
	{
		return this;
	}

	if (IsZero())
	{
		return this;
	}

	iOldFirst = GetFirstNonZeroDigit();
	iOldLast = GetLastNonZeroDigit();

	iFirst = OffsetDigit(iOldFirst, iOffset);
	if (iFirst > mcMaxWholeNumbers)
	{
		return Overflow(GetSign());
	}
	if (iFirst < -mcMaxDecimals)
	{
		//Every digit is shifted past the smallest decimal.
		Init(mcMaxWholeNumbers, mcMaxDecimals);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		return this;
	}

	iLast = OffsetDigit(iOldLast, iOffset);
	if (iLast < -mcMaxDecimals)
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		iLast = -mcMaxDecimals;
	}
	PrivateShiftNibbles(iOffset, iOldFirst, iOldLast);
	SetFirstNonZerotDigit(iFirst);
	SetLastNonZeroDigit(iLast);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Truncate(int16 iDigitToTruncateInclusive)
{
	int16		iLast;

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
CNumber* CNumber::TruncateHigh(int16 iDigit)
{
	int16		iFirst;

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
bool CNumber::IsClean(void)
{
	CNumber*	pcTest;
	bool		bResult;

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
int16 CNumber::GetWholeNumbers(void)
{
	int16	iFirst;

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
size CNumber::GetDigitsBetween(void)
{
	int16	iLast;
	int16	iFirst;

	iFirst = GetFirstNonZeroDigit();
	iLast = GetLastNonZeroDigit();

	return GetDigitsBetween(iFirst, iLast);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsNAN(void)
{
	return mcFlags & NUMBER_FLAGS_NAN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsUnderflow(void)
{
	return mcFlags & NUMBER_FLAGS_UNDERFLOW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsOverflow(void)
{
	return mcFlags & NUMBER_FLAGS_OVERFLOW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsNegative(void)
{
	return FixBool(mcFlags & NUMBER_FLAGS_NEGATIVE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsPositive(void)
{
	return !IsNegative() && !IsZero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::IsPositiveOrZero(void)
{
	return !IsNegative();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateEquals(CNumber* pcNumber, int16 iDecimals)
{
	int16	iThisLastDigit;
	int16	iOtherLastDigit;
	int16	iThisFirstDigit;
	int16	iOtherFirstDigit;
	int32	iResult;

	if (IsError())
	{
		return false;
	}

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	if (IsNegative() != pcNumber->IsNegative())
	{
		return false;
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
		return false;
	}

	iResult = PrivateCompareDigits(pcNumber, iThisFirstDigit, iOtherFirstDigit, GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	return iResult == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::Equals(CNumber* pcNumber)
{
	return PrivateEquals(pcNumber, mcMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::GreaterThan(CNumber* pcNumber)
{
	bool	bThisNegative;
	bool	bOtherNegative;
	bool	bThisZero;
	bool	bOtherZero;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();
	bThisZero = IsZero();
	bOtherZero = pcNumber->IsZero();

	if (bThisZero)
	{
		if (bOtherZero)
		{
			return false;
		}
		else if (bOtherNegative)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (bThisNegative)
	{
		if (bOtherZero)
		{
			return false;
		}
		else if (bOtherNegative)
		{
			return !PrivateGreaterThanOrEquals(pcNumber);
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (bOtherZero)
		{
			return true;
		}
		else if (bOtherNegative)
		{
			return true;
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
bool CNumber::GreaterThanOrEquals(CNumber* pcNumber)
{
	bool	bThisNegative;
	bool	bOtherNegative;
	bool	bThisZero;
	bool	bOtherZero;

	bThisNegative = IsNegative();
	bOtherNegative = pcNumber->IsNegative();
	bThisZero = IsZero();
	bOtherZero = pcNumber->IsZero();

	if (bThisZero)
	{
		if (bOtherZero)
		{
			return true;
		}
		else if (bOtherNegative)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (bThisNegative)
	{
		if (bOtherZero)
		{
			return false;
		}
		else if (bOtherNegative)
		{
			return !PrivateGreaterThan(pcNumber);
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (bOtherZero)
		{
			return true;
		}
		else if (bOtherNegative)
		{
			return true;
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
bool CNumber::LessThan(CNumber* pcNumber)
{
	return !GreaterThanOrEquals(pcNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::LessThanOrEquals(CNumber* pcNumber)
{
	return !GreaterThan(pcNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateGreaterThan(CNumber* pcNumber)
{
	int16	iThisLastDigit;
	int16	iOtherLastDigit;
	int16	iThisFirstDigit;
	int16	iOtherFirstDigit;
	int32	iResult;

	//Zero's first digit is the units digit so it would look bigger than any number below one.
	if (IsZero())
	{
		return false;
	}
	if (pcNumber->IsZero())
	{
		return true;
	}

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	if (iOtherLastDigit > iThisLastDigit)
	{
	}

	if (iThisFirstDigit > iOtherFirstDigit)
	{
		return true;
	}
	else if (iOtherFirstDigit > iThisFirstDigit)
	{
		return false;
	}

	iResult = PrivateCompareDigits(pcNumber, iThisFirstDigit, iThisFirstDigit, GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	if (iResult > 0)
	{
		return true;
	}
	else if (iResult < 0)
	{
		return false;
	}
	else if (iThisLastDigit < iOtherLastDigit)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateGreaterThanOrEquals(CNumber* pcNumber)
{
	int16		iThisLastDigit;
	int16		iOtherLastDigit;
	int16		iThisFirstDigit;
	int16		iOtherFirstDigit;
	int32		iResult;

	//Zero's first digit is the units digit so it would look bigger than any number below one.
	if (pcNumber->IsZero())
	{
		return true;
	}
	if (IsZero())
	{
		return false;
	}

	iThisFirstDigit = GetFirstNonZeroDigit();
	iThisLastDigit = GetLastNonZeroDigit();

	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();
	iOtherLastDigit = pcNumber->GetLastNonZeroDigit();

	if (iOtherLastDigit > iThisLastDigit)
	{
	}

	if (iThisFirstDigit > iOtherFirstDigit)
	{
		return true;
	}
	else if (iOtherFirstDigit > iThisFirstDigit)
	{
		return false;
	}

	iResult = PrivateCompareDigits(pcNumber, iThisFirstDigit, iThisFirstDigit, GetDigitsBetween(iThisFirstDigit, iThisLastDigit));
	if (iResult > 0)
	{
		return true;
	}
	else if (iResult < 0)
	{
		return false;
	}
	else if (iThisLastDigit > iOtherLastDigit)
	{
		return false;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Add(CNumber* pcNumber)
{
	bool		bThisNegative;
	bool		bOtherNegative;
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
	bool		bThisNegative;
	bool		bOtherNegative;
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
bool CNumber::PrivateError(CNumber* pcNumber)
{
	if (PrivateNAN(pcNumber))
	{
		return true;
	}
	if (PrivateOverflow(pcNumber))
	{
		return true;
	}
	if (PrivateDivisionByZero(pcNumber))
	{
		return true;
	}
	PrivateUnderflow(pcNumber);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateAdd(CNumber* pcNumber)
{
	int16	iLast;
	int16	iFirst;
	int16	iThisFirstDigit;
	int16	iOtherFirstDigit;

	if (PrivateError(pcNumber)) return;

	iLast = PrivateLowestLast(pcNumber);

	iThisFirstDigit = GetFirstNonZeroDigit();
	iOtherFirstDigit = pcNumber->GetFirstNonZeroDigit();

	iFirst = iThisFirstDigit;
	if (iThisFirstDigit < iOtherFirstDigit)
	{
		iFirst = iOtherFirstDigit;
	}
	if (iFirst > mcMaxWholeNumbers)
	{
		Overflow(GetSign());
		return;
	}

	//A carry out of the top word, or into the nibbles above the biggest whole number, is too big.
	if ((PrivateAddWords(pcNumber, iFirst, iLast, false) != 0) || PrivateAboveMaximum())
	{
		Overflow(GetSign());
		return;
	}

	if (iFirst < mcMaxWholeNumbers)
	{
		iFirst = OffsetDigit(iFirst, 1);
	}
	SetFirstNonZerotDigit(iFirst);

	//The digits outside first and last are still 0: only first and last are found again.
	PrivateFindEnds();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds (or subtracts, bSubtract) pcNumber's digits from iFirst down to
// iLast to this one's, a packed BCD word at a time, and returns the carry
// (or borrow) out of this one's top word.  The words of the two line up
// (the decimal point is on a word boundary in both); pcNumber's digits
// below this one's smallest decimal are left out.  The digits of both
// outside their first and last non zero digits must be 0.
//
//////////////////////////////////////////////////////////////////////////
uint16 CNumber::PrivateAddWords(CNumber* pcNumber, int16 iFirst, int16 iLast, bool bSubtract)
{
	int16	iLow;
	int16	iHigh;
	int16	iDelta;
	int16	iMinimum;
	uint16	uiCarry;
	uint16	uiWord;

	iLow = (int16)(DigitPosition(iLast) >> 2);
	iHigh = (int16)(DigitPosition(iFirst) >> 2);
	iDelta = (int16)((NUMBER_WORD_DIGITS(mcMaxDecimals) - NUMBER_WORD_DIGITS(pcNumber->mcMaxDecimals)) >> 2);

	//The words pcNumber has.
	if (iLow < iDelta)
	{
		iLow = iDelta;
	}
	if (iHigh > iDelta + pcNumber->NumWords() - 1)
	{
		iHigh = iDelta + pcNumber->NumWords() - 1;
	}
	if (iLow > iHigh)
	{
		return 0;
	}

	uiCarry = 0;
	iMinimum = MinimumPosition();
	if ((iLow == (iMinimum >> 2)) && ((iMinimum & 3) != 0))
	{
		uiWord = (uint16)(pcNumber->mauiDigits[iLow - iDelta] & gauiNumberNibblesFrom[iMinimum & 3]);
		uiCarry = NumberBCD(&mauiDigits[iLow], &uiWord, 1, 0, bSubtract);
		iLow++;
	}
	if (iLow <= iHigh)
	{
		uiCarry = NumberBCD(&mauiDigits[iLow], &pcNumber->mauiDigits[iLow - iDelta], iHigh - iLow + 1, uiCarry, bSubtract);
	}

	return PrivateRipple((int16)(iHigh + 1), uiCarry, bSubtract);
}


//////////////////////////////////////////////////////////////////////////
//
// A carry (or a borrow, bSubtract) of uiCarry into the word iWord, on up
// the words while it goes on: a word of 9999 carries out to 0, a 0 borrows
// to 9999; the first word that doesn't takes the 1.  Returns the carry (or
// borrow) out of the top word.
//
//////////////////////////////////////////////////////////////////////////
uint16 CNumber::PrivateRipple(int16 iWord, uint16 uiCarry, bool bSubtract)
{
	int16	iWords;
	uint16	uiOne;
	uint16*	puiWord;

	if (uiCarry == 0)
	{
		return 0;
	}
	iWords = NumWords();
	puiWord = &mauiDigits[iWord];
	for (; iWord < iWords; iWord++)
	{
		if (*puiWord != (bSubtract ? 0 : 0x9999))
		{
			uiOne = 1;
			return NumberBCD(puiWord, &uiOne, 1, 0, bSubtract);
		}
		*puiWord = bSubtract ? 0x9999 : 0;
		puiWord++;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// This is at least pcNumber (both without their signs).
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateSubtract(CNumber* pcNumber)
{
	int16	iLast;

	if (PrivateError(pcNumber)) return;

	iLast = PrivateLowestLast(pcNumber);
	PrivateAddWords(pcNumber, GetFirstNonZeroDigit(), iLast, true);

	//The digits outside first and last are still 0: only first and last are found again.
	PrivateFindEnds();
}


//////////////////////////////////////////////////////////////////////////
//
// The lower of this number's last non zero digit and pcNumber's, which
// becomes this one's last.  pcNumber's digits below this one's smallest
// decimal are lost (underflow).
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::PrivateLowestLast(CNumber* pcNumber)
{
	int16	iLast;

	iLast = pcNumber->GetLastNonZeroDigit();
	if (iLast < -mcMaxDecimals)
	{
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		iLast = -mcMaxDecimals;
	}
	if (iLast < GetLastNonZeroDigit())
	{
		SetLastNonZeroDigit(iLast);
	}
	return GetLastNonZeroDigit();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a digit above the biggest whole number (in the top word's spare
// nibbles) isn't 0: a carry went past it.
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateAboveMaximum(void)
{
	int16	i;

	for (i = (int16)(DigitPosition(mcMaxWholeNumbers) + 1); i < NumWords() * 4; i++)
	{
		if (GetNibble(i) != 0)
		{
			return true;
		}
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::Multiply(CNumber* pcMultiplicand)
{
	int16		iThisLastDigit;
	int16		iMultiplicandLastDigit;
	int16		iThisFirstDigit;
	int16		iMultiplicandFirstDigit;
	char		cOtherDigit;
	char		cMost;
	int16		j;
	int16		iOtherDigit;
	int16		iThisWords;
	int16		iShift;
	CNumber*	pcResult;
	CNumber*	pcPlaced;
	CNumber*	pcMultiples;
	uint16*		pauiMultiple;
	int16		iDecimals;

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

	//The result has three more decimals than this.  The multiples of this (by 1 to 9, each a word longer than this
	//to take the carries) are made one from the one before as the multiplicand's digits need them.
	iThisWords = NumWords();
	pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	if (!pcResult)
	{
		return NULL;
	}
	pcPlaced = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcMultiples = gcNumberControl.Add((int16)(9 * 4 * (iThisWords + 1)), 0);
	pcResult->Init(mcMaxWholeNumbers, iDecimals);
	memset(pcMultiples->mauiDigits, 0, (size_t)(iThisWords + 1) * sizeof(uint16));
	memcpy(pcMultiples->mauiDigits, mauiDigits, (size_t)iThisWords * sizeof(uint16));
	cMost = 1;

	for (j = iMultiplicandLastDigit; j <= iMultiplicandFirstDigit; j++)
	{
		if (j == 0)
		{
			continue;
		}
		cOtherDigit = pcMultiplicand->GetDigitUnsafe(j);
		if (cOtherDigit == 0)
		{
			continue;
		}
		iOtherDigit = (j > 0) ? (j - 1) : j;

		//The line is this times the digit, moved up by the digit's power of ten.  Its first digit past the biggest
		//whole number overflows; when its last is past the result's smallest decimal the digits beyond are lost.
		if (OffsetDigit(iThisFirstDigit, iOtherDigit) > mcMaxWholeNumbers)
		{
			gcNumberControl.Remove(3);
			return Overflow(GetSign() * pcMultiplicand->GetSign());
		}
		if (((iThisLastDigit > 0) ? (iThisLastDigit - 1) : iThisLastDigit) + iOtherDigit < -iDecimals)
		{
			pcResult->SetFlag(NUMBER_FLAGS_UNDERFLOW);
		}

		for (; cMost < cOtherDigit; cMost++)
		{
			pauiMultiple = &pcMultiples->mauiDigits[cMost * (iThisWords + 1)];
			memcpy(pauiMultiple, &pcMultiples->mauiDigits[(cMost - 1) * (iThisWords + 1)], (size_t)(iThisWords + 1) * sizeof(uint16));
			BCDAdd(pauiMultiple, pcMultiples->mauiDigits, iThisWords + 1, 0);
		}

		//A nibble of this at position p is at p + iShift in the result.
		iShift = (int16)(NUMBER_WORD_DIGITS(iDecimals) - NUMBER_WORD_DIGITS(mcMaxDecimals) + iOtherDigit);
		pauiMultiple = &pcMultiples->mauiDigits[(cOtherDigit - 1) * (iThisWords + 1)];
		if (!pcResult->PrivateAddPlaced(pauiMultiple, iThisWords + 1, iShift, pcPlaced->mauiDigits))
		{
			gcNumberControl.Remove(3);
			return Overflow(GetSign() * pcMultiplicand->GetSign());
		}
	}

	pcResult->SetFirstNonZerotDigit(mcMaxWholeNumbers);
	pcResult->SetLastNonZeroDigit(-iDecimals);
	pcResult->SetSign(GetSign() * pcMultiplicand->GetSign());
	pcResult->PrivateFindEnds();
	Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(3);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the packed BCD words pauiSource, moved up iShift nibbles (it may be
// negative), to this number's.  The nibbles that land below the smallest
// decimal are left out.  The moved words are put together in pauiPlaced
// (as many words as this).  False if a digit lands above the biggest
// whole number, or a carry goes past it.
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateAddPlaced(uint16* pauiSource, int16 iSourceWords, int16 iShift, uint16* pauiPlaced)
{
	uint16*	puiSource;
	uint16*	puiPlaced;
	int16	iWords;
	int16	iNibbles;
	int16	iWordShift;
	int16	iMinimum;
	int16	iLow;
	int16	iHigh;
	int16	i;
	int16	iSource;
	uint16	uiLow;
	uint16	uiHigh;
	uint16	uiWord;
	uint16	uiCarry;

	iWords = NumWords();
	iNibbles = (int16)(iShift & 3);
	iWordShift = (int16)((iShift - iNibbles) >> 2);
	iMinimum = MinimumPosition();

	//The words the source reaches: from its first word's, to its last word's (and the one its top nibbles go to).
	iLow = iWordShift;
	if (iLow < (iMinimum >> 2))
	{
		iLow = (int16)(iMinimum >> 2);
	}
	iHigh = (int16)(iWordShift + iSourceWords - ((iNibbles == 0) ? 1 : 0));

	//The source words from iLow's on (iLow is at least iWordShift, so the first is in the source).  Words past this
	//number's top must be 0.
	iSource = (int16)(iLow - iWordShift);
	puiSource = &pauiSource[iSource];
	puiPlaced = &pauiPlaced[iLow];
	uiLow = (iSource >= 1) ? puiSource[-1] : 0;
	for (i = iLow; i <= iHigh; i++)
	{
		//This word's high source is the next one's low.
		uiHigh = (iSource < iSourceWords) ? *puiSource : 0;
		uiWord = NumberCombineNibbles(uiHigh, uiLow, iNibbles);
		uiLow = uiHigh;
		puiSource++;
		iSource++;
		if (i < iWords)
		{
			*puiPlaced = uiWord;
			puiPlaced++;
		}
		else if (uiWord != 0)
		{
			return false;
		}
	}
	if (iLow == (iMinimum >> 2))
	{
		pauiPlaced[iLow] &= gauiNumberNibblesFrom[iMinimum & 3];
	}
	if (iHigh >= iWords)
	{
		iHigh = (int16)(iWords - 1);
	}
	if (iLow > iHigh)
	{
		return true;
	}

	uiCarry = BCDAdd(&mauiDigits[iLow], &pauiPlaced[iLow], iHigh - iLow + 1, 0);
	uiCarry = PrivateRipple((int16)(iHigh + 1), uiCarry, false);
	return (uiCarry == 0) && !PrivateAboveMaximum();
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
	int16		iDividendDigit;
	int16		iQuotientDigit;
	bool		bExact;
	int16		iLastDigit;
	int16		iLastDivisorDigit;
	int16		iDecimals;

	if (PrivateError(pcDivisorIn)) return this;
	if (pcDivisorIn->IsZero()) return DivisionByZero();

	iDecimals = mcMaxDecimals + 3;

	pcDivisor = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
	pcPartialDividend = gcNumberControl.Add(mcMaxWholeNumbers + 1, iDecimals);
	pcQuotient = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

	pcDivisor->Init(pcDivisorIn, mcMaxWholeNumbers, iDecimals);
	pcDivisor->Absolute();
	iLastDivisorDigit = pcDivisor->GetLastNonZeroDigit();

	iDividendDigit = GetFirstNonZeroDigit();
	iLastDigit = GetLastNonZeroDigit();
	cThisDigit = GetDigitUnsafe(iDividendDigit);
	pcPartialDividend->Digit(iLastDivisorDigit, cThisDigit, mcMaxWholeNumbers + 1, iDecimals);

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

		cThisDigit = GetDigitUnsafe(iDividendDigit);
		if (bExact)
		{
			if (iDividendDigit < iLastDigit)
			{
				break;
			}
			pcPartialDividend->Digit(iLastDivisorDigit, cThisDigit, mcMaxWholeNumbers + 1, iDecimals);
		}
		else
		{
			pcPartialDividend->Shift(1);
			pcPartialDividend->SetDigit(iLastDivisorDigit, cThisDigit);
		}
	}

	pcQuotient->SetSign(GetSign() * pcDivisorIn->GetSign());
	pcQuotient->Clean();
	Init(pcQuotient, mcMaxWholeNumbers, mcMaxDecimals);
	gcNumberControl.Remove(3);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::PrivateDivide(CNumber* pcDivisor, bool* pbExact)
{
	char	cDigit;
	int16	iFirst;
	int16	iLast;

	//"this" will contain the remainder.  The divisor is taken off until that borrows (this was smaller), and then put
	//back once: a borrow wraps the words to its top, and adding back carries out of it again.  Both are positive,
	//with the same decimals.
	cDigit = 0;
	if (!IsZero())
	{
		iFirst = GetFirstNonZeroDigit();
		if (pcDivisor->GetFirstNonZeroDigit() > iFirst)
		{
			iFirst = pcDivisor->GetFirstNonZeroDigit();
		}
		iLast = GetLastNonZeroDigit();
		if (pcDivisor->GetLastNonZeroDigit() < iLast)
		{
			iLast = pcDivisor->GetLastNonZeroDigit();
		}
		SetFirstNonZerotDigit(iFirst);
		SetLastNonZeroDigit(iLast);
		while (PrivateAddWords(pcDivisor, iFirst, iLast, true) == 0)
		{
			cDigit++;
		}
		PrivateAddWords(pcDivisor, iFirst, iLast, false);
		PrivateFindEnds();
	}
	*pbExact = IsZero();
	return cDigit;
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
bool CNumber::PrivateNAN(CNumber* pcNumber)
{
	if (pcNumber->IsNAN())
	{
		NotANumber();
		return true;
	}
	else if (IsNAN())
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateDivisionByZero(CNumber* pcNumber)
{
	if (pcNumber->IsDivisionByZero())
	{
		DivisionByZero();
		return true;
	}
	else if (IsDivisionByZero())
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrivateOverflow(CNumber* pcNumber)
{
	if (pcNumber->IsOverflow() && IsOverflow())
	{
		if (IsNegative() != pcNumber->IsNegative())
		{
			NotANumber();
		}
		return true;
	}
	else if (pcNumber->IsOverflow())
	{
		Overflow(pcNumber->GetSign());
		return true;
	}
	else if (IsOverflow())
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateIntegerFactorial(int16 iN, int16 cMaxWholeNumbers, int16 cMaxDecimals)
{
	CNumber*	pcI;
	uint16		i;

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
	for (i = 1; i <= (uint16)iN; i++)
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
	int32		iIterations;
	int32		iDecimals;
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
	int16	iThisPosition;
	int16	iOtherPosition;
	size	i;
	int16	iOtherFirst;
	int16	iOtherLast;
	size	iLength;

	//This function assumes that pcNumber is a different size to this.
	mcMaxWholeNumbers = cMaxWholeNumbers;
	mcMaxDecimals = cMaxDecimals;
	mcFlags = 0;
	memset(mauiDigits, 0, (size_t)NumWords() * sizeof(uint16));
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

	if (iOtherFirst < -(mcMaxDecimals))
	{
		//Every digit is smaller than this number can hold.
		Init(cMaxWholeNumbers, cMaxDecimals);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
		return;
	}

	if (iOtherLast < -(mcMaxDecimals))
	{
		iOtherLast = -(mcMaxDecimals);
		SetFlag(NUMBER_FLAGS_UNDERFLOW);
	}

	iLength = GetDigitsBetween(iOtherFirst, iOtherLast);
	iThisPosition = DigitPosition(iOtherFirst);
	iOtherPosition = pcNumber->DigitPosition(iOtherFirst);
	for (i = 0; i < iLength; i++)
	{
		SetNibble(iThisPosition - (int16)i, pcNumber->GetNibble(iOtherPosition - (int16)i));
	}

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
void CNumber::PrivateIntegerExponent(int16 iExponent)
{
	CNumber*	pcResult;
	CNumber*	pcBase;
	int32		iDecimals;

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
		pcBase = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

		pcResult->One(mcMaxWholeNumbers, iDecimals);
		pcBase->Init(this, mcMaxWholeNumbers, iDecimals);
		for (;;)
		{
			if (iExponent & 1)
			{
				pcResult->Multiply(pcBase);
			}
			iExponent >>= 1;
			if (iExponent == 0)
			{
				break;
			}
			pcBase->Squared();
		}
		Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
		gcNumberControl.Remove(1 + 1);
	}
	else if (iExponent == -1)
	{
		Inverse();
	}
	else if (iExponent == -2)
	{
		InverseSquared();
	}
	else if (iExponent == -3)
	{
		InverseCubed();
	}
	else if (iExponent < 0)
	{
		iExponent = -iExponent;
		iDecimals = mcMaxDecimals + 3;

		pcResult = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);
		pcBase = gcNumberControl.Add(mcMaxWholeNumbers, iDecimals);

		pcResult->One(mcMaxWholeNumbers, iDecimals);
		pcBase->Init(this, mcMaxWholeNumbers, iDecimals);
		for (;;)
		{
			if (iExponent & 1)
			{
				pcResult->Multiply(pcBase);
			}
			iExponent >>= 1;
			if (iExponent == 0)
			{
				break;
			}
			pcBase->Squared();
		}
		Init(pcResult, mcMaxWholeNumbers, mcMaxDecimals);
		gcNumberControl.Remove(1 + 1);
		Inverse();
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
	int32		n;

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
CNumber* CNumber::InverseSquared(void)
{
	Squared();
	Inverse();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber* CNumber::InverseCubed(void)
{
	Cubed();
	Inverse();
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
		Zero(mcMaxWholeNumbers, mcMaxDecimals);
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
	bool		bLeftZero;
	bool		bRightZero;

	pcZero = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	bLeftZero = Equals(pcZero->Zero());
	bRightZero = pcRight->Equals(pcZero);
	if (bLeftZero && bRightZero)
	{
		Zero(mcMaxWholeNumbers, mcMaxDecimals);
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
	bool		bLeftZero;
	bool		bRightZero;

	pcZero = gcNumberControl.Add(mcMaxWholeNumbers, mcMaxDecimals);
	bLeftZero = Equals(pcZero->Zero());
	bRightZero = pcRight->Equals(pcZero);
	if (bLeftZero || bRightZero)
	{
		Zero(mcMaxWholeNumbers, mcMaxDecimals);
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
	int32		iDecimals;
	CNumber*	pcExponent;

	if (pcExponentIn->IsInteger())
	{
		PrivateIntegerExponent(pcExponentIn);
		return this;
	}

	iDecimals = (int32)mcMaxDecimals*2;
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
	int32		iDecimals;

	if (pcRootIn->IsInteger())
	{
		PrivateIntegerRoot(pcRootIn);
		return this;
	}

	iDecimals = (int32)mcMaxDecimals*2;
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
	int32		iDecimals;
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
	uint32		i;
	CNumber*	pcFractional;
	CNumber*	pcIntegral;
	CNumber*	pcOne;
	CNumber*	pcPart;
	CNumber*	pcNumber;
	CNumber*	pcE;
	CNumber*	pcResult;
	int32		iDecimals;

	//e^integral can have up to mcMaxWholeNumbers digits before the point so mcMaxDecimals decimal places
	//are not enough precision for the last decimal place of the result to be correct.
	iDecimals = mcMaxDecimals + mcMaxWholeNumbers + 3;
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
	for (i = (uint32)iDecimals; i > 0; i--)
	{
		pcPart->Init(pcFractional, mcMaxWholeNumbers, iDecimals);
		pcNumber->Init((int32)i, mcMaxWholeNumbers, iDecimals);
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
	int32		iInteger;
	CNumber*	pcInverseBase;
	CNumber*	pcDecimal;
	CNumber*	pcPartial;
	CNumber*	pcPointFive;
	CNumber*	pcResult;
	int32		i;
	int32		iSign;
	int32		iDecimals;

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
	pcOne->One(mcMaxWholeNumbers, iDecimals);
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
	int32		iDecimals;
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
int32 CNumber::IntValue(void)
{
	int16	iFirst;
	uint32	i;
	int32	iTimes;
	char	iValue;
	int32	iResult;

	iFirst = GetFirstNonZeroDigit();

	if ((iFirst < 0) || (IsZero()))
	{
		return 0;
	}

	iTimes = 1;
	iResult = 0;

	for (i = 1; i <= (uint32)iFirst; i++)
	{
		iValue = GetDigitUnsafe(i);
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
uint64 CNumber::ULongValue(void)
{
	int16	iFirst;
	uint32	i;
	uint64	iTimes;
	char	iValue;
	uint64	iResult;

	iFirst = GetFirstNonZeroDigit();

	if ((iFirst < 0) || (IsZero()))
	{
		return 0;
	}

	iTimes = 1;
	iResult = 0;

	for (i = 1; i <= (uint32)iFirst; i++)
	{
		iValue = GetDigitUnsafe(i);
		iResult += (iValue * iTimes);
		iTimes *= 10;
	}
	iResult *= GetSign();

	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CNumber::DecimalValue(uint16* puiNumDecimals)
{
	int16	iLast;
	int32	i;
	uint64	iTimes;
	char	iValue;
	uint64	iResult;
	uint16	uiNumDecimals;

	iLast = GetLastNonZeroDigit();

	if ((iLast > 0) || (IsZero()))
	{
		SafeAssign(puiNumDecimals, (uint16)0);
		return 0;
	}

	iTimes = 1;
	iResult = 0;

	for (i = iLast; i <= -1; i++)
	{
		iValue = GetDigitUnsafe(i);
		iResult += (iValue * iTimes);
		iTimes *= 10;
	}

	if (iLast <= -1)
	{
		uiNumDecimals = (uint16)-iLast;
	}
	else
	{
		uiNumDecimals = 0;
	}
	
	SafeAssign(puiNumDecimals, uiNumDecimals);

	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CNumber::FloatValue(void)
{
	float32		fValue;
	uint64		uiWholeNumbers;
	uint64		uiDecimalNumbers;
	uint16		uiNumDecimals;

	//ULongValue includes the sign; the conversion needs the magnitude.
	uiWholeNumbers = ULongValue();
	if (IsNegative())
	{
		uiWholeNumbers = (uint64)0 - uiWholeNumbers;
	}
	uiDecimalNumbers = DecimalValue(&uiNumDecimals);

	fValue = (float32)ConvertDecimalFixedPointToLongDouble(uiWholeNumbers, uiDecimalNumbers, uiNumDecimals, 0);

	return IsNegative() ? -fValue : fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CNumber::DoubleValue(void)
{
	float64		fValue;
	uint64		uiWholeNumbers;
	uint64		uiDecimalNumbers;
	uint16		uiNumDecimals;

	//ULongValue includes the sign; the conversion needs the magnitude.
	uiWholeNumbers = ULongValue();
	if (IsNegative())
	{
		uiWholeNumbers = (uint64)0 - uiWholeNumbers;
	}
	uiDecimalNumbers = DecimalValue(&uiNumDecimals);

	fValue = (float64)ConvertDecimalFixedPointToLongDouble(uiWholeNumbers, uiDecimalNumbers, uiNumDecimals, 0);

	return IsNegative() ? -fValue : fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::SafeGetDigit(int16 iDigit)
{
	if (iDigit <= mcFirstDigit && iDigit >= mcLastDigit)
	{
		if (((iDigit > 0) && (iDigit <= mcMaxWholeNumbers)) || ((iDigit >= -mcMaxDecimals) && (iDigit < 0)))
		{
			return GetNibble(DigitPosition(iDigit));
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CNumber::GetDigitUnsafe(int16 iDigit)
{
	if (((iDigit > 0) && (iDigit <= mcMaxWholeNumbers)) || ((iDigit >= -mcMaxDecimals) && (iDigit < 0)))
	{
		return GetNibble(DigitPosition(iDigit));
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
void CNumber::SetDigit(int16 iDigit, char cValue)
{
	int16	iFirstDigit;
	int16	iLastDigit;
	bool	bWasZero;

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
		if (cValue != DOT)
		{
			SetFlag(NUMBER_FLAGS_NAN);
		}
	}
	else
	{
		if ((cValue >= 1) && (cValue <= 9))
		{
			bWasZero = IsZero();
			SetNibble(DigitPosition(iDigit), cValue);

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
			SetNibble(DigitPosition(iDigit), cValue);

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
void CNumber::SetDigitUnsafe(int16 iDigit, char cValue)
{
	if (iDigit != 0)
	{
		SetNibble(DigitPosition(iDigit), cValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The nibble iDigit is in (not 0): the decimals rounded up to whole words
// are below the point (NUMBER_WORD_DIGITS), the whole numbers above it.
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::DigitPosition(int16 iDigit)
{
	if (iDigit > 0)
	{
		return (int16)(NUMBER_WORD_DIGITS(mcMaxDecimals) + iDigit - 1);
	}
	return (int16)(NUMBER_WORD_DIGITS(mcMaxDecimals) + iDigit);
}


//////////////////////////////////////////////////////////////////////////
//
// The digits from iFirst down to iLast (the others are 0) moved up iShift
// nibbles (down when it is negative), a word at a time: each word is put
// together from the two it gets its nibbles from, from the top down when
// moving up (the bottom up when moving down) so no word is written before
// it is read.  Zeros come in behind them; digits moved below the smallest
// decimal are lost.  The caller checks for overflow first.
//
//////////////////////////////////////////////////////////////////////////
void CNumber::PrivateShiftNibbles(int16 iShift, int16 iFirst, int16 iLast)
{
	uint16*	puiSource;
	uint16*	puiDest;
	int16	iWords;
	int16	iNibbles;
	int16	iWordShift;
	int16	iMinimum;
	int16	iLow;
	int16	iHigh;
	int16	i;
	int16	iSource;
	uint16	uiLow;
	uint16	uiHigh;

	iWords = NumWords();
	iNibbles = (int16)(iShift & 3);
	iWordShift = (int16)((iShift - iNibbles) >> 2);

	//The words the digits from iFirst to iLast are in, before and after they move.
	iLow = (int16)(DigitPosition(iLast) >> 2);
	iHigh = (int16)(DigitPosition(iFirst) >> 2);
	if (iShift > 0)
	{
		iHigh = (int16)(iHigh + iWordShift + ((iNibbles != 0) ? 1 : 0));
	}
	else
	{
		iLow = (int16)(iLow + iWordShift);
	}
	if (iLow < 0)
	{
		iLow = 0;
	}
	if (iHigh >= iWords)
	{
		iHigh = (int16)(iWords - 1);
	}

	//Pointers walk the words (an index would be scaled each time); the counters say when a source word is outside
	//the number (0 then).
	iSource = (int16)(((iShift > 0) ? iHigh : iLow) - iWordShift);
	puiSource = &mauiDigits[iSource];
	if (iShift > 0)
	{
		//Down from the top: this word's low source is the next one's high.
		puiDest = &mauiDigits[iHigh];
		uiHigh = ((iSource >= 0) && (iSource < iWords)) ? *puiSource : 0;
		for (i = iHigh; i >= iLow; i--)
		{
			iSource--;
			puiSource--;
			uiLow = ((iSource >= 0) && (iSource < iWords)) ? *puiSource : 0;
			*puiDest = NumberCombineNibbles(uiHigh, uiLow, iNibbles);
			puiDest--;
			uiHigh = uiLow;
		}
	}
	else
	{
		//Up from the bottom: this word's high source is the next one's low.
		puiDest = &mauiDigits[iLow];
		uiLow = ((iSource >= 1) && (iSource <= iWords)) ? puiSource[-1] : 0;
		for (i = iLow; i <= iHigh; i++)
		{
			uiHigh = ((iSource >= 0) && (iSource < iWords)) ? *puiSource : 0;
			*puiDest = NumberCombineNibbles(uiHigh, uiLow, iNibbles);
			puiDest++;
			puiSource++;
			iSource++;
			uiLow = uiHigh;
		}
	}
	iMinimum = MinimumPosition();
	if (iLow * 4 < iMinimum)
	{
		PrivateZeroPositions((int16)(iMinimum - 1), (int16)(iLow * 4));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The digit in the nibble iPosition: the low nibble of byte iPosition / 2
// for an even position, the high nibble for an odd one.
//
//////////////////////////////////////////////////////////////////////////
char CNumber::GetNibble(int16 iPosition)
{
	uint8	ucByte;

	ucByte = ((uint8*)mauiDigits)[iPosition >> 1];
	return (char)((iPosition & 1) ? (ucByte >> 4) : (ucByte & 0xF));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::SetNibble(int16 iPosition, char cValue)
{
	uint8*	pucByte;

	pucByte = &((uint8*)mauiDigits)[iPosition >> 1];
	if (iPosition & 1)
	{
		*pucByte = (uint8)((*pucByte & 0x0F) | (cValue << 4));
	}
	else
	{
		*pucByte = (uint8)((*pucByte & 0xF0) | cValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::NumWords(void)
{
	return NUMBER_WORDS(mcMaxWholeNumbers, mcMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
// The position of the smallest decimal (the nibbles below it are 0).
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::MinimumPosition(void)
{
	return (int16)(NUMBER_WORD_DIGITS(mcMaxDecimals) - mcMaxDecimals);
}


//////////////////////////////////////////////////////////////////////////
//
// As memcmp of uiDigits digits from this one's iFirst and pcOther's
// iOtherFirst down: the sign of the first that differs, else 0.
//
//////////////////////////////////////////////////////////////////////////
int16 CNumber::PrivateCompareDigits(CNumber* pcOther, int16 iFirst, int16 iOtherFirst, size uiDigits)
{
	int16	iThis;
	int16	iOther;
	size	i;
	char	cThis;
	char	cOther;

	iThis = DigitPosition(iFirst);
	iOther = pcOther->DigitPosition(iOtherFirst);
	for (i = 0; i < uiDigits; i++)
	{
		cThis = GetNibble(iThis - (int16)i);
		cOther = pcOther->GetNibble(iOther - (int16)i);
		if (cThis != cOther)
		{
			return (cThis > cOther) ? 1 : -1;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNumber::PrintFloating(CChars* pcChars)
{
	int32	iFirstDigit;
	int32	iLastDigit;
	int32	i;
	char	c;
	int32	iStart;
	int32	iStop;

	if (IsNAN())
	{
		pcChars->Append(NUMBER_NOT_A_NUMBER);
		return false;
	}
	if (IsDivisionByZero())
	{
		pcChars->Append(NUMBER_NOT_A_NUMBER);
		return false;
	}

	if (IsNegative())
	{
		pcChars->Append("-");
	}

	if (IsOverflow())
	{
		pcChars->Append(NUMBER_INFINITE_STRING);
		return false;
	}

	iFirstDigit = GetFirstNonZeroDigit();
	iLastDigit = GetLastNonZeroDigit();

	iStart = iFirstDigit;
	iStop = iLastDigit;

	if ((iStart >= 1) && (iStop <= -1))
	{
		for (i = iStart; i >= iStop; i--)
		{
			if (i != 0)
			{
				c = GetDigitUnsafe(i) + ZERO;
				pcChars->Append(c);
			}
			else
			{
				pcChars->Append(DOT);
			}
		}
	}
	else if ((iStart >= 1) && (iStop == 1))
	{
		for (i = iStart; i >= iStop; i--)
		{
				c = GetDigitUnsafe(i) + ZERO;
				pcChars->Append(c);
		}
		pcChars->Append(DOT);
	}
	else if ((iStart >= 1)  && (iStart <= 8) && (iStop >= 1) && (iStop <= 4))
	{
		for (i = iStart; i >= iStop; i--)
		{
			c = GetDigitUnsafe(i) + ZERO;
			pcChars->Append(c);
		}
		pcChars->Append(ZERO, iStop - 1);
		pcChars->Append(DOT);
	}
	else if ((iStart >= 1) && (iStop >= 1))
	{
		for (i = iStart; i >= iStop; i--)
		{
			c = GetDigitUnsafe(i) + ZERO;
			pcChars->Append(c);
			if (i == iStart)
			{
				pcChars->Append(DOT);
			}
		}
		pcChars->Append('e');
		pcChars->Append('+');
		pcChars->Append(iStart - 1);
	}
	else if ((iStart <= -1) && (iStart >= -4)  && (iStop <= -1))
	{
		pcChars->Append(ZERO);
		pcChars->Append(DOT);
		pcChars->Append(ZERO, (-iStart) -1);

		for (i = iStart; i >= iStop; i--)
		{
			c = GetDigitUnsafe(i) + ZERO;
			pcChars->Append(c);
		}
	}
	else if ((iStart < 0) && (iStop < 0))
	{
		for (i = iStart; i >= iStop; i--)
		{
			c = GetDigitUnsafe(i) + ZERO;
			pcChars->Append(c);
			if (i == iStart)
			{
				pcChars->Append(DOT);
			}
		}
		pcChars->Append('e');
		pcChars->Append(iStart);
	}

	if (!IsClean())
	{
		pcChars->Append(" (Unclean!)");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Print(CChars* pcChars)
{
	int32	iFirstDigit;
	int32	iLastDigit;
	int32	i;
	char	c;
	int32	iStart;
	int32	iStop;

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
			c = SafeGetDigit(i) + ZERO;
			pcChars->Append(c);
		}
		else
		{
			pcChars->Append(DOT);
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
	Dump(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNumber::Dump(bool bNewLine)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNumber::ByteSize(void)
{
	return NUMBER_SIZE(mcMaxWholeNumbers, mcMaxDecimals);
}

