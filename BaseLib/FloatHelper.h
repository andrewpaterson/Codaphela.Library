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
#ifndef __FLOAT_HELPER_H__
#define __FLOAT_HELPER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


float	sqf(float f);
float	minf(float f1, float f2);
float	maxf(float f1, float f2);
bool	FloatEqual(float f1, float f2);
bool	FloatEqual(float f1, float f2, float fTolerance);
bool	FloatGreaterThanOrEqual(float f1, float f2);
bool	FloatLessThanOrEqual(float f1, float f2);
bool	FloatGreaterThan(float f1, float f2);
bool	FloatLessThan(float f1, float f2);
float 	RoundFloat(float fInput, int iBinaryExponent);
float 	TruncateFloat(float fInput, int iBinaryExponent);
float 	NormaliseFloat(float fSize, float fPos);  //returns 1 .. -1
float 	NormaliseFloat(float fMax, float fMin, float fPos);  //returns 1 .. 0
float	InterpolateFloat(float f1, float f2, float fPos);
void	InterpolateFloat(float* fDest, float f1, float f2, float fPos);
float	FloatToleranceForDecimals(int iDecimals);
int		FloatWholeNumbers(float f);
double	sq(double f);
bool	DoubleEqual(double f1, double f2);
bool	DoubleEqual(double f1, double f2, double fTolerance);
bool	DoubleGreaterThanOrEqual(double f1, double f2);
bool	DoubleLessThanOrEqual(double f1, double f2);
bool	DoubleGreaterThan(double f1, double f2);
bool	DoubleLessThan(double f1, double f2);
double 	RoundDouble(double fInput, int64 iBinaryExponent);
double 	TruncateDouble(double fInput, int64 iBinaryExponent);
double 	NormaliseDouble(double fSize, double fPos);  //returns 1 .. -1
double 	NormaliseDouble(double fMax, double fMin, double fPos);  //returns 1 .. 0
double	InterpolateDouble(double f1, double f2, double fPos);
void	InterpolateDouble(double* fDest, double f1, double f2, double fPos);
double	DoubleToleranceForDecimals(int iDecimals);
int		DoubleWholeNumbers(double f);
bool	LongDoubleEqual(float96 f1, float96 f2);
bool	LongDoubleEqual(float96 f1, float96 f2, float96 fTolerance);
void	Swap(float* f1, float* f2);
float	NotANumber(void);
bool	IsNotANumber(float* pf);
float	det2x2(float a, float b, float c, float d);
float	det3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3);
double	det2x2(double a, double b, double c, double d);
double	det3x3(double a1, double a2, double a3, double b1, double b2, double b3, double c1, double c2, double c3);


extern float gfNaN;


#endif // __FLOAT_HELPER_H__

