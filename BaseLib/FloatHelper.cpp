/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
BOOL FloatEqual(float f1, float f2, float fTolerance)
{
	// f1 == f2
	if ((f2 - fTolerance > f1) || (f2 + fTolerance < f1))
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL DoubleEqual(double f1, double f2, double fTolerance)
{
	// f1 == f2
	if ((f2 - fTolerance > f1) || (f2 + fTolerance < f1))
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FloatEqual(float f1, float f2)
{
	return FloatEqual(f1, f2, SMALL_NUMBER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FloatGreaterThanOrEqual(float f1, float f2)
{
	//f1 >= f2
	if (f2 - SMALL_NUMBER > f1)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FloatLessThanOrEqual(float f1, float f2)
{
	//f1 <= f2
	if (f2 + SMALL_NUMBER < f1)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FloatGreaterThan(float f1, float f2)
{
	//f1 > f2
	if (f2 + SMALL_NUMBER >= f1)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FloatLessThan(float f1, float f2)
{
	//f1 < f2
	if (f2 - SMALL_NUMBER <= f1)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float TruncateFloat(float fInput, int iBinaryExponent)
{
	//Everything below the binary exponent is truncated.

	float			fReturn;
	unsigned int	iTruncated;
	int				iFloatExponent;
	int				iExponentDifference;
	unsigned int	uiFractionMask;
	int				iMaskedBits;

	//Remember that an exponent of 0x7f is 2**0 = 1.  0x80 is 2**1 = 2.  0x81 is 2**2 = 4 etc..

	iFloatExponent = *((int*)((void*)&fInput));  //Get the bits from the float into and int without doing a (float) cast.
	iFloatExponent >>= 23;  //Remove the fractions part of the float leaving only the exponent and sign.
	iFloatExponent &= 0xff;  //Get rid of the sign

	iBinaryExponent += 0x7f;  //Get the ieee 754 exponent representation to truncate to.

	iExponentDifference = iFloatExponent-iBinaryExponent;  //The difference between the floats exponent and the wanted exponent.

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
double DoubleToleranceForDecimals(int iDecimals)
{
	long double fTolerance;

	fTolerance = powl(10.0, (long double)iDecimals);
	fTolerance = 1.0f/fTolerance;
	return fTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int FloatWholeNumbers(float f)
{
	float fCmp = 1.0f;
	int iWholes;

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
void Swap(float* f1, float* f2)
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
	int	i = 0xffffffff;
	float f = *((float*)((void*)&i));
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL IsNotANumber(float* pf)
{
	return *((int*)((void*)pf)) == 0xffffffff;
}


float gfNaN = NotANumber();
