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
#ifndef __FLOAT_HELPER_H__
#define __FLOAT_HELPER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


float32		minf(float32 f1, float32 f2);
float32		maxf(float32 f1, float32 f2);
bool		FloatEqual(float32 f1, float32 f2);
bool		FloatEqual(float32 f1, float32 f2, float32 fTolerance);
bool		FloatGreaterThanOrEqual(float32 f1, float32 f2);
bool		FloatLessThanOrEqual(float32 f1, float32 f2);
bool		FloatGreaterThan(float32 f1, float32 f2);
bool		FloatLessThan(float32 f1, float32 f2);
float32 	RoundFloat(float32 fInput, int iBinaryExponent);
float32 	TruncateFloat(float32 fInput, int iBinaryExponent);
float32 	NormaliseFloat(float32 fSize, float32 fPos);  //returns 1 .. -1
float32 	NormaliseFloat(float32 fMax, float32 fMin, float32 fPos);  //returns 1 .. 0
float32		InterpolateFloat(float32 f1, float32 f2, float32 fPos);
void		InterpolateFloat(float32* fDest, float32 f1, float32 f2, float32 fPos);
float32		Float32ToleranceForDecimals(int iDecimals);
size		FloatWholeNumbers(float32 f);
bool		FloatEqual(float64 f1, float64 f2);
bool		FloatEqual(float64 f1, float64 f2, float64 fTolerance);
bool		DoubleGreaterThanOrEqual(float64 f1, float64 f2);
bool		DoubleLessThanOrEqual(float64 f1, float64 f2);
bool		DoubleGreaterThan(float64 f1, float64 f2);
bool		DoubleLessThan(float64 f1, float64 f2);
float64 	RoundFloat(float64 fInput, int64 iBinaryExponent);
float64 	TruncateFloat(float64 fInput, int64 iBinaryExponent);
float64 	NormaliseDouble(float64 fSize, float64 fPos);  //returns 1 .. -1
float64 	NormaliseDouble(float64 fMax, float64 fMin, float64 fPos);  //returns 1 .. 0
float64		InterpolateDouble(float64 f1, float64 f2, float64 fPos);
void		InterpolateDouble(float64* fDest, float64 f1, float64 f2, float64 fPos);
float64		Float64ToleranceForDecimals(int iDecimals);
size		FloatWholeNumbers(float64 f);
bool		LongDoubleEqual(float128 f1, float128 f2);
bool		LongDoubleEqual(float128 f1, float128 f2, float128 fTolerance);
void		SwapFloat(float32* f1, float32* f2);
float32		Float32NotANumber(void);
float64		Float64NotANumber(void);
bool		IsNotANumber(float32* pf);
float128	ConvertDecimalFixedPointToLongDouble(uint64 ulliWholeNumber, uint64 ulliDecimalNumber, uint16 iNumDecimalDigits, int64 lliExponentNumber);
float32		det2x2(float32 a, float32 b, float32 c, float32 d);
float32		det3x3(float32 a1, float32 a2, float32 a3, float32 b1, float32 b2, float32 b3, float32 c1, float32 c2, float32 c3);
float64		det2x2(float64 a, float64 b, float64 c, float64 d);
float64		det3x3(float64 a1, float64 a2, float64 a3, float64 b1, float64 b2, float64 b3, float64 c1, float64 c2, float64 c3);
float32		SquareRoot(float32 f);
float64		SquareRoot(float64 f);
float128	SquareRoot(float128 f);
float32		Square(float32 f);
float64		Square(float64 f);
float128	Square(float128 f);


extern float32 gf32Nan;
extern float64 gf64Nan;


#endif // __FLOAT_HELPER_H__

