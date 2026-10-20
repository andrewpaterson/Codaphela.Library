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
#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__
#include "Define.h"
#include "PrimitiveTypes.h"


//A fixed point number is an integer holding a value multiplied by 2^iFractionBits (8 fraction bits hold 1.5 as 384).
//Everything is integer arithmetic, so results are the same in every build and on every compiler, unlike floating
//point whose last bits can change with optimisation.  Rounding is to the nearest, halves away from zero, unless a
//function says otherwise.  Intermediate results are 64 bit; results that don't fit are undefined.


int64	DivideRound(int64 iNumerator, int64 iDenominator);
int64	DivideFloor(int64 iNumerator, int64 iDenominator);
int64	DivideCeiling(int64 iNumerator, int64 iDenominator);

int32	FixedFromInt(int32 i, int iFractionBits);
int32	FixedFromFloat(float32 f, int iFractionBits);
int32	FloatToIntScaled(float32 f, int32 iScale);
float32	FixedToFloat(int32 iFixed, int iFractionBits);
int32	FixedConvert(int32 iFixed, int iFromFractionBits, int iToFractionBits);

int32	FixedFloor(int32 iFixed, int iFractionBits);
int32	FixedCeiling(int32 iFixed, int iFractionBits);
int32	FixedRound(int32 iFixed, int iFractionBits);
int32	FixedFraction(int32 iFixed, int iFractionBits);

int32	FixedMultiply(int32 iA, int32 iB, int iFractionBits);
int32	FixedDivide(int32 iA, int32 iB, int iFractionBits);
int32	FixedMultiplyDivide(int32 iA, int32 iB, int32 iC);
int32	FixedSquareRoot(int32 iFixed, int iFractionBits);


#endif // __FIXED_POINT_H__
