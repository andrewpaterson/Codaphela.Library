/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
float sqf(float f)
{
	return f*f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float minf(float f1, float f2)
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
float maxf(float f1, float f2)
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
bool FloatEqual(float f1, float f2, float fTolerance)
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
bool FloatEqual(float f1, float f2)
{
	return FloatEqual(f1, f2, SMALL_NUMBER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FloatGreaterThanOrEqual(float f1, float f2)
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
bool FloatLessThanOrEqual(float f1, float f2)
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
bool FloatGreaterThan(float f1, float f2)
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
bool FloatLessThan(float f1, float f2)
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
float TruncateFloat(float fInput, int iBinaryExponent)
{
	//Everything below the binary exponent is truncated.

	float	fReturn;
	uint32	iTruncated;
	int		iFloatExponent;
	int		iExponentDifference;
	uint32	uiFractionMask;
	int		iMaskedBits;

	//Remember that an exponent of 0x7f is 2**0 = 1.  0x80 is 2**1 = 2.  0x81 is 2**2 = 4 etc..

	iFloatExponent = *((int*)((void*)&fInput));  //Get the bits from the float into and int without doing a (float) cast.
	iFloatExponent >>= 23;  //Remove the fractions part of the float leaving only the exponent and sign.
	iFloatExponent &= 0xff;  //Get rid of the sign

	iBinaryExponent += 0x7f;  //Get the ieee 754 exponent representation to truncate to.

	iExponentDifference = iFloatExponent - iBinaryExponent;  //The difference between the floats exponent and the wanted exponent.

	//If the exponent difference is >= 0 then our float has some digits which are not trunacted.
	if (iExponentDifference >= 0)
	{
		iMaskedBits = 22 - (iExponentDifference - 1);  //Why -1?.  I can't remember.  Clearly it made sense at the time.
		if (iMaskedBits > 0)
		{
			//Create a mask for the bits of float that are not truncated.
			uiFractionMask = 0xffffffff;
			uiFractionMask >>= iMaskedBits;
			uiFractionMask <<= iMaskedBits;

			iTruncated = *((int*)((void*)&fInput));
			iTruncated &= uiFractionMask;
			fReturn = *((float*)((void*)&iTruncated));
			return fReturn;
		}
		else
		{
			return fInput;
		}
	}

	//Otherwise the float we're tuncating is smaller than the 2**exponent we're truncating it too which gives 0.
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float RoundFloat(float fInput, int iBinaryExponent)
{
	float	fValue;
	int		iAdjustedExponent;

	iAdjustedExponent = iBinaryExponent - 1 + 0x7f;
	iAdjustedExponent <<= 23;
	fValue = *((float*)((void*)&iAdjustedExponent));

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
float NormaliseFloat(float fSize, float fPos)
{
	return ((fPos*2.0f) / fSize) -1.0f;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float NormaliseFloat(float fMax, float fMin, float fPos)
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
float InterpolateFloat(float f1, float f2, float fPos)
{
	return (f1 * fPos) + (f2 * (1.0f - fPos));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpolateFloat(float* fDest, float f1, float f2, float fPos)
{
	*fDest = InterpolateFloat(f1, f2, fPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float FloatToleranceForDecimals(int iDecimals)
{
	double fTolerance;

	fTolerance = pow(10.0, iDecimals);
	fTolerance = 1.0/fTolerance;
	return (float)fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FloatWholeNumbers(float f)
{
	float	fCmp;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double sq(double f)
{
	return f * f;
}


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleEqual(double f1, double f2)
{
	return DoubleEqual(f1, f2, SMALL_NUMBER);
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DoubleEqual(double f1, double f2, double fTolerance)
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
bool LongDoubleEqual(float96 f1, float96 f2)
{
	return LongDoubleEqual(f1, f2, SMALL_NUMBER);
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LongDoubleEqual(float96 f1, float96 f2, float96 fTolerance)
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
bool DoubleGreaterThanOrEqual(double f1, double f2)
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
bool DoubleLessThanOrEqual(double f1, double f2)
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
bool DoubleGreaterThan(double f1, double f2)
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
bool DoubleLessThan(double f1, double f2)
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
double TruncateDouble(double fInput, int64 iBinaryExponent)
{
	//Everything below the binary exponent is truncated.

	double			fReturn;
	uint64			iTruncated;
	int64			iDoubleExponent;
	int64			iExponentDifference;
	uint64			uiFractionMask;
	int64			iMaskedBits;

	//Remember that an exponent of 0x7f is 2**0 = 1.  0x80 is 2**1 = 2.  0x81 is 2**2 = 4 etc..

	iDoubleExponent = *((int64*)((void*)&fInput));  //Get the bits from the double into and int without doing a (double) cast.
	iDoubleExponent >>= 52;  //Remove the fractions part of the double leaving only the exponent and sign.
	iDoubleExponent &= 0xfffLL;  //Get rid of the sign

	iBinaryExponent += 0x3ffLL;  //Get the ieee 754 exponent representation to truncate to.

	iExponentDifference = iDoubleExponent - iBinaryExponent;  //The difference between the floats exponent and the wanted exponent.

	//If the exponent difference is >= 0 then our double has some digits which are not trunacted.
	if (iExponentDifference >= 0)
	{
		iMaskedBits = 51 - (iExponentDifference - 1);  //Why -1?.  I can't remember.  Clearly it made sense at the time.
		if (iMaskedBits > 0)
		{
			//Create a mask for the bits of double that are not truncated.
			uiFractionMask = 0xffffffffffffffffLL;
			uiFractionMask >>= iMaskedBits;
			uiFractionMask <<= iMaskedBits;

			iTruncated = *((int64*)((void*)&fInput));
			iTruncated &= uiFractionMask;
			fReturn = *((double*)((void*)&iTruncated));
			return fReturn;
		}
		else
		{
			return fInput;
		}
	}

	//Otherwise the double we're tuncating is smaller than the 2**exponent we're truncating it too which gives 0.
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double RoundDouble(double fInput, int64 iBinaryExponent)
{
	double	fValue;
	int64		iAdjustedExponent;

	iAdjustedExponent = iBinaryExponent - 1 + 0x3ff;
	iAdjustedExponent <<= 52;
	fValue = *((double*)((void*)&iAdjustedExponent));

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
double NormaliseDouble(double fSize, double fPos)
{
	return ((fPos * 2.0f) / fSize) - 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double NormaliseDouble(double fMax, double fMin, double fPos)
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
double InterpolateDouble(double f1, double f2, double fPos)
{
	return (f1 * fPos) + (f2 * (1.0f - fPos));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void InterpolateDouble(double* fDest, double f1, double f2, double fPos)
{
	*fDest = InterpolateDouble(f1, f2, fPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
double DoubleToleranceForDecimals(int iDecimals)
{
	float96 fTolerance;

	fTolerance = powl(10.0, (float96)iDecimals);
	fTolerance = 1.0f/fTolerance;
	return fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size DoubleWholeNumbers(double f)
{
	double	fCmp;
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
void SwapFloat(float* f1, float* f2)
{
	float temp;

	temp = *f2;
	*f2 = *f1;
	*f1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float NotANumber(void)
{
	uint32	i = 0xffffffff;
	float f = *((float*)((void*)&i));
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsNotANumber(float* pf)
{
	return *((uint32*)((void*)pf)) == 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float96 ConvertDecimalFixedPointToLongDouble(uint64 ulliWholeNumber, uint64 ulliDecimalNumber, uint16 iNumDecimalDigits, int64 lliExponentNumber)
{
	float96		ldf;
	float96		ldfPow;
	float96		ldfExp;

	ldf = (float96)ulliWholeNumber;
	ldfPow = 1.l / powl(10, iNumDecimalDigits);
	ldf += ulliDecimalNumber * ldfPow;
	if (lliExponentNumber > 0)
	{
		ldfExp = powl(10, (float96)lliExponentNumber);
		ldf *= ldfExp;
	}
	else if (lliExponentNumber < 0)
	{
		ldfExp = 1.l / powl(10, -((float96)lliExponentNumber));
		ldf *= ldfExp;
	}
	return ldf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
	float ans;

	ans = a1 * det2x2(b2, b3, c2, c3)
		- b1 * det2x2(a2, a3, c2, c3)
		+ c1 * det2x2(a2, a3, b2, b3);
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
float det2x2(float a, float b, float c, float d)
{
	float ans;

	ans = a * d - b * c;
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3)
{
	double ans;

	ans = a1 * det2x2(b2, b3, c2, c3)
		- b1 * det2x2(a2, a3, c2, c3)
		+ c1 * det2x2(a2, a3, b2, b3);
	return ans;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
double det2x2(double a, double b, double c, double d)
{
	double ans;

	ans = a * d - b * c;
	return ans;
}


float gfNaN = NotANumber();
