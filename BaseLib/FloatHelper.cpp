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
#include "Numbers.h"
#include "FloatHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 minf(float32 f1, float32 f2)
{
	if (f1 < f2)
	{
		return f1;
	}
	return f2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 maxf(float32 f1, float32 f2)
{
	if (f1 > f2)
	{
		return f1;
	}
	return f2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatEqual(float32 f1, float32 f2, float32 fTolerance)
{
	// f1 == f2
	if ((f2 - fTolerance > f1) || (f2 + fTolerance < f1))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatEqual(float32 f1, float32 f2)
{
	return FloatEqual(f1, f2, SMALL_NUMBER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatGreaterThanOrEqual(float32 f1, float32 f2)
{
	//f1 >= f2
	if (f2 - SMALL_NUMBER > f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatLessThanOrEqual(float32 f1, float32 f2)
{
	//f1 <= f2
	if (f2 + SMALL_NUMBER < f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatGreaterThan(float32 f1, float32 f2)
{
	//f1 > f2
	if (f2 + SMALL_NUMBER >= f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatLessThan(float32 f1, float32 f2)
{
	//f1 < f2
	if (f2 - SMALL_NUMBER <= f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 TruncateFloat(float32 fInput, int iBinaryExponent)
{
	//Everything below the binary exponent is truncated.

	float32	fReturn;
	uint32	iTruncated;
	int		iFloatExponent;
	int		iExponentDifference;
	uint32	uiFractionMask;
	int		iMaskedBits;

	//Remember that an exponent of 0x7f is 2**0 = 1.  0x80 is 2**1 = 2.  0x81 is 2**2 = 4 etc..

	iFloatExponent = *((int*)((void*)&fInput));  //Get the bits from the float32 into and int without doing a (float32) cast.
	iFloatExponent >>= 23;  //Remove the fractions part of the float32 leaving only the exponent and sign.
	iFloatExponent &= 0xff;  //Get rid of the sign

	iBinaryExponent += 0x7f;  //Get the ieee 754 exponent representation to truncate to.

	iExponentDifference = iFloatExponent - iBinaryExponent;  //The difference between the floats exponent and the wanted exponent.

	//If the exponent difference is >= 0 then our float32 has some digits which are not trunacted.
	if (iExponentDifference >= 0)
	{
		iMaskedBits = 22 - (iExponentDifference - 1);  //Why -1?.  I can't remember.  Clearly it made sense at the time.
		if (iMaskedBits > 0)
		{
			//Create a mask for the bits of float32 that are not truncated.
			uiFractionMask = 0xffffffff;
			uiFractionMask >>= iMaskedBits;
			uiFractionMask <<= iMaskedBits;

			iTruncated = *((int*)((void*)&fInput));
			iTruncated &= uiFractionMask;
			fReturn = *((float32*)((void*)&iTruncated));
			return fReturn;
		}
		else
		{
			return fInput;
		}
	}

	//Otherwise the float32 we're tuncating is smaller than the 2**exponent we're truncating it too which gives 0.
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 RoundFloat(float32 fInput, int iBinaryExponent)
{
	float32	fValue;
	int		iAdjustedExponent;

	iAdjustedExponent = iBinaryExponent - 1 + 0x7f;
	iAdjustedExponent <<= 23;
	fValue = *((float32*)((void*)&iAdjustedExponent));

	if (((fValue < 0.0f) && (fInput > 0.0f)) || ((fValue > 0.0f) && (fInput < 0.0f)))
	{
		fInput -= fValue;
	}
	else
	{
		fInput += fValue;
	}
	return TruncateFloat(fInput, iBinaryExponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 NormaliseFloat(float32 fSize, float32 fPos)
{
	return ((fPos*2.0f) / fSize) -1.0f;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 NormaliseFloat(float32 fMax, float32 fMin, float32 fPos)
{
	if (fMax != fMin)
	{
		return (fPos - fMin) / (fMax - fMin);
	}
	else
	{
		if (FloatEqual(fMax, fPos))
		{
			return fMax;
		}

		//In this case the result is undefined.
		return 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 InterpolateFloat(float32 f1, float32 f2, float32 fPos)
{
	return (f1 * fPos) + (f2 * (1.0f - fPos));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpolateFloat(float32* fDest, float32 f1, float32 f2, float32 fPos)
{
	*fDest = InterpolateFloat(f1, f2, fPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 FloatToleranceForDecimals(int iDecimals)
{
	float64 fTolerance;

	fTolerance = pow(10.0, iDecimals);
	fTolerance = 1.0/fTolerance;
	return (float32)fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FloatWholeNumbers(float32 f)
{
	float32	fCmp;
	size	iWholes;

	fCmp = 1.0f;
	f = fabsf(f);
	for (iWholes = 0; iWholes < 8; iWholes++)
	{
		if (f < fCmp)
		{
			return iWholes;
		}
		fCmp *= 10.0f;
	}
	return iWholes;
}


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleEqual(float64 f1, float64 f2)
{
	return DoubleEqual(f1, f2, SMALL_NUMBER);
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleEqual(float64 f1, float64 f2, float64 fTolerance)
{
	// f1 == f2
	if ((f2 - fTolerance > f1) || (f2 + fTolerance < f1))
	{
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LongDoubleEqual(float128 f1, float128 f2)
{
	return LongDoubleEqual(f1, f2, SMALL_NUMBER);
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LongDoubleEqual(float128 f1, float128 f2, float128 fTolerance)
{
	// f1 == f2
	if ((f2 - fTolerance > f1) || (f2 + fTolerance < f1))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleGreaterThanOrEqual(float64 f1, float64 f2)
{
	//f1 >= f2
	if (f2 - SMALL_NUMBER > f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleLessThanOrEqual(float64 f1, float64 f2)
{
	//f1 <= f2
	if (f2 + SMALL_NUMBER < f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleGreaterThan(float64 f1, float64 f2)
{
	//f1 > f2
	if (f2 + SMALL_NUMBER >= f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleLessThan(float64 f1, float64 f2)
{
	//f1 < f2
	if (f2 - SMALL_NUMBER <= f1)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 TruncateDouble(float64 fInput, int64 iBinaryExponent)
{
	//Everything below the binary exponent is truncated.

	float64			fReturn;
	uint64			iTruncated;
	int64			iDoubleExponent;
	int64			iExponentDifference;
	uint64			uiFractionMask;
	int64			iMaskedBits;

	//Remember that an exponent of 0x7f is 2**0 = 1.  0x80 is 2**1 = 2.  0x81 is 2**2 = 4 etc..

	iDoubleExponent = *((int64*)((void*)&fInput));  //Get the bits from the float64 into and int without doing a (float64) cast.
	iDoubleExponent >>= 52;  //Remove the fractions part of the float64 leaving only the exponent and sign.
	iDoubleExponent &= 0xfffLL;  //Get rid of the sign

	iBinaryExponent += 0x3ffLL;  //Get the ieee 754 exponent representation to truncate to.

	iExponentDifference = iDoubleExponent - iBinaryExponent;  //The difference between the floats exponent and the wanted exponent.

	//If the exponent difference is >= 0 then our float64 has some digits which are not trunacted.
	if (iExponentDifference >= 0)
	{
		iMaskedBits = 51 - (iExponentDifference - 1);  //Why -1?.  I can't remember.  Clearly it made sense at the time.
		if (iMaskedBits > 0)
		{
			//Create a mask for the bits of float64 that are not truncated.
			uiFractionMask = 0xffffffffffffffffLL;
			uiFractionMask >>= iMaskedBits;
			uiFractionMask <<= iMaskedBits;

			iTruncated = *((int64*)((void*)&fInput));
			iTruncated &= uiFractionMask;
			fReturn = *((float64*)((void*)&iTruncated));
			return fReturn;
		}
		else
		{
			return fInput;
		}
	}

	//Otherwise the float64 we're tuncating is smaller than the 2**exponent we're truncating it too which gives 0.
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 RoundDouble(float64 fInput, int64 iBinaryExponent)
{
	float64	fValue;
	int64		iAdjustedExponent;

	iAdjustedExponent = iBinaryExponent - 1 + 0x3ff;
	iAdjustedExponent <<= 52;
	fValue = *((float64*)((void*)&iAdjustedExponent));

	if (((fValue < 0.0f) && (fInput > 0.0f)) || ((fValue > 0.0f) && (fInput < 0.0f)))
	{
		fInput -= fValue;
	}
	else
	{
		fInput += fValue;
	}
	return TruncateDouble(fInput, iBinaryExponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 NormaliseDouble(float64 fSize, float64 fPos)
{
	return ((fPos * 2.0f) / fSize) - 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 NormaliseDouble(float64 fMax, float64 fMin, float64 fPos)
{
	if (fMax != fMin)
	{
		return (fPos - fMin) / (fMax - fMin);
	}
	else
	{
		if (DoubleEqual(fMax, fPos))
		{
			return fMax;
		}

		//In this case the result is undefined.
		return 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 InterpolateDouble(float64 f1, float64 f2, float64 fPos)
{
	return (f1 * fPos) + (f2 * (1.0f - fPos));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpolateDouble(float64* fDest, float64 f1, float64 f2, float64 fPos)
{
	*fDest = InterpolateDouble(f1, f2, fPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 DoubleToleranceForDecimals(int iDecimals)
{
	float128 fTolerance;

	fTolerance = powl(10.0, (float128)iDecimals);
	fTolerance = 1.0f/fTolerance;
	return fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size DoubleWholeNumbers(float64 f)
{
	float64	fCmp;
	size	iWholes;

	fCmp = 1.0f;
	f = fabs(f);
	for (iWholes = 0; iWholes < 8; iWholes++)
	{
		if (f < fCmp)
		{
			return iWholes;
		}
		fCmp *= 10.0f;
	}
	return iWholes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapFloat(float32* f1, float32* f2)
{
	float32 temp;

	temp = *f2;
	*f2 = *f1;
	*f1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 Float32NotANumber(void)
{
	uint32	i = 0xffffffff;
	float32 f = *((float32*)((void*)&i));
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 Float64NotANumber(void)
{
	uint64	i = 0xffffffffffffffffLL;
	float64 f = *((float64*)((void*)&i));
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsNotANumber(float32* pf)
{
	return *((uint32*)((void*)pf)) == 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float128 ConvertDecimalFixedPointToLongDouble(uint64 ulliWholeNumber, uint64 ulliDecimalNumber, uint16 iNumDecimalDigits, int64 lliExponentNumber)
{
	float128		ldf;
	float128		ldfPow;
	float128		ldfExp;

	ldf = (float128)ulliWholeNumber;
	ldfPow = 1.l / powl(10, iNumDecimalDigits);
	ldf += ulliDecimalNumber * ldfPow;
	if (lliExponentNumber > 0)
	{
		ldfExp = powl(10, (float128)lliExponentNumber);
		ldf *= ldfExp;
	}
	else if (lliExponentNumber < 0)
	{
		ldfExp = 1.l / powl(10, -((float128)lliExponentNumber));
		ldf *= ldfExp;
	}
	return ldf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 det3x3(float32 a1, float32 a2, float32 a3, float32 b1, float32 b2, float32 b3, float32 c1, float32 c2, float32 c3)
{
	float32 ans;

	ans = a1 * det2x2(b2, b3, c2, c3)
		- b1 * det2x2(a2, a3, c2, c3)
		+ c1 * det2x2(a2, a3, b2, b3);
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 det2x2(float32 a, float32 b, float32 c, float32 d)
{
	float32 ans;

	ans = a * d - b * c;
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float64 det3x3(float64 a1, float64 a2, float64 a3, float64 b1, float64 b2, float64 b3, float64 c1, float64 c2, float64 c3)
{
	float64 ans;

	ans = a1 * det2x2(b2, b3, c2, c3)
		- b1 * det2x2(a2, a3, c2, c3)
		+ c1 * det2x2(a2, a3, b2, b3);
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float64 det2x2(float64 a, float64 b, float64 c, float64 d)
{
	float64 ans;

	ans = a * d - b * c;
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 SquareRoot(float32 f)
{
	return sqrtf(f);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float64 SquareRoot(float64 f)
{
	return sqrt(f);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float128 SquareRoot(float128 f)
{
	return sqrtl(f);
}
//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float32 Square(float32 f)
{
	return f*f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float64 Square(float64 f)
{
	return f*f;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float128 Square(float128 f)
{
	return f*f;
}


float32 gf32Nan = Float32NotANumber();
float64 gf64Nan = Float64NotANumber();

