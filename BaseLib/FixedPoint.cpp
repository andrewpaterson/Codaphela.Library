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
#include <math.h>
#include "IntegerHelper.h"
#include "FixedPoint.h"


//////////////////////////////////////////////////////////////////////////
//
// Divides to the nearest integer, halves away from zero (7 / 2 is 4,
// -7 / 2 is -4).
//
//////////////////////////////////////////////////////////////////////////
int64 DivideRound(int64 iNumerator, int64 iDenominator)
{
	int64	iAbsoluteNumerator;
	int64	iAbsoluteDenominator;
	int64	iQuotient;

	iAbsoluteNumerator = (iNumerator < 0) ? -iNumerator : iNumerator;
	iAbsoluteDenominator = (iDenominator < 0) ? -iDenominator : iDenominator;
	iQuotient = (iAbsoluteNumerator + iAbsoluteDenominator / 2) / iAbsoluteDenominator;
	if ((iNumerator < 0) != (iDenominator < 0))
	{
		return -iQuotient;
	}
	return iQuotient;
}


//////////////////////////////////////////////////////////////////////////
//
// Divides rounding towards negative infinity (-7 / 2 is -4).  C++
// division rounds towards zero.
//
//////////////////////////////////////////////////////////////////////////
int64 DivideFloor(int64 iNumerator, int64 iDenominator)
{
	int64	iQuotient;

	iQuotient = iNumerator / iDenominator;
	if (((iNumerator % iDenominator) != 0) && ((iNumerator < 0) != (iDenominator < 0)))
	{
		iQuotient--;
	}
	return iQuotient;
}


//////////////////////////////////////////////////////////////////////////
//
// Divides rounding towards positive infinity (7 / 2 is 4).
//
//////////////////////////////////////////////////////////////////////////
int64 DivideCeiling(int64 iNumerator, int64 iDenominator)
{
	int64	iQuotient;

	iQuotient = iNumerator / iDenominator;
	if (((iNumerator % iDenominator) != 0) && ((iNumerator < 0) == (iDenominator < 0)))
	{
		iQuotient++;
	}
	return iQuotient;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FixedFromInt(int32 i, int iFractionBits)
{
	return (int32)((int64)i * ((int64)1 << iFractionBits));
}


//////////////////////////////////////////////////////////////////////////
//
// The float is widened to a double, where multiplying by a power of two
// and adding a half are exact, so the same float gives the same fixed
// point number in every build.  Halves round up.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedFromFloat(float32 f, int iFractionBits)
{
	float64	fScaled;

	fScaled = (float64)f * (float64)((int64)1 << iFractionBits);
	return (int32)floor(fScaled + 0.5);
}


//////////////////////////////////////////////////////////////////////////
//
// Rounds f * iScale (0.5 * 255 is 128).  A float has 24 significant bits
// so, for scales below 2^29, the product is exact as a double and the
// result is the same in every build.  Halves round up.
//
//////////////////////////////////////////////////////////////////////////
int32 FloatToIntScaled(float32 f, int32 iScale)
{
	float64	fScaled;

	fScaled = (float64)f * (float64)iScale;
	return (int32)floor(fScaled + 0.5);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 FixedToFloat(int32 iFixed, int iFractionBits)
{
	return (float32)((float64)iFixed / (float64)((int64)1 << iFractionBits));
}


//////////////////////////////////////////////////////////////////////////
//
// Changes the number of fraction bits, rounding if bits are lost.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedConvert(int32 iFixed, int iFromFractionBits, int iToFractionBits)
{
	if (iToFractionBits >= iFromFractionBits)
	{
		return (int32)((int64)iFixed * ((int64)1 << (iToFractionBits - iFromFractionBits)));
	}
	return (int32)DivideRound(iFixed, (int64)1 << (iFromFractionBits - iToFractionBits));
}


//////////////////////////////////////////////////////////////////////////
//
// The largest integer not greater than the number.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedFloor(int32 iFixed, int iFractionBits)
{
	return (int32)DivideFloor(iFixed, (int64)1 << iFractionBits);
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest integer not less than the number.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedCeiling(int32 iFixed, int iFractionBits)
{
	return (int32)DivideCeiling(iFixed, (int64)1 << iFractionBits);
}


//////////////////////////////////////////////////////////////////////////
//
// The nearest integer, halves away from zero.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedRound(int32 iFixed, int iFractionBits)
{
	return (int32)DivideRound(iFixed, (int64)1 << iFractionBits);
}


//////////////////////////////////////////////////////////////////////////
//
// The part above the floor, always positive (-1.25 has a fraction of
// 0.75).
//
//////////////////////////////////////////////////////////////////////////
int32 FixedFraction(int32 iFixed, int iFractionBits)
{
	return iFixed - FixedFromInt(FixedFloor(iFixed, iFractionBits), iFractionBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FixedMultiply(int32 iA, int32 iB, int iFractionBits)
{
	return (int32)DivideRound((int64)iA * iB, (int64)1 << iFractionBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FixedDivide(int32 iA, int32 iB, int iFractionBits)
{
	return (int32)DivideRound((int64)iA * ((int64)1 << iFractionBits), iB);
}


//////////////////////////////////////////////////////////////////////////
//
// iA * iB / iC with a 64 bit product; any number of fraction bits as long
// as iB and iC have the same number.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedMultiplyDivide(int32 iA, int32 iB, int32 iC)
{
	return (int32)DivideRound((int64)iA * iB, iC);
}


//////////////////////////////////////////////////////////////////////////
//
// Rounded down.  Returns -1 for negative numbers.
//
//////////////////////////////////////////////////////////////////////////
int32 FixedSquareRoot(int32 iFixed, int iFractionBits)
{
	if (iFixed < 0)
	{
		return -1;
	}
	//sqrt(v * 2^f) * 2^(f/2) = sqrt(v) * 2^f.
	return (int32)SquareRoot((int64)iFixed * ((int64)1 << iFractionBits));
}

