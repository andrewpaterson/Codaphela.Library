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
#ifndef __FLOAT_HELPER_H__
#define __FLOAT_HELPER_H__
#include "Define.h"

float	sqf(float f);
float	minf(float f1, float f2);
float	maxf(float f1, float f2);
BOOL	FloatEqual(float f1, float f2);
BOOL	FloatEqual(float f1, float f2, float fTolerance);
BOOL	FloatGreaterThanOrEqual(float f1, float f2);
BOOL	FloatLessThanOrEqual(float f1, float f2);
BOOL	FloatGreaterThan(float f1, float f2);
BOOL	FloatLessThan(float f1, float f2);
float 	RoundFloat(float fInput, int iBinaryExponent);
float 	TruncateFloat(float fInput, int iBinaryExponent);
float 	NormaliseFloat(float fSize, float fPos);  //returns 1 .. -1
float 	NormaliseFloat(float fMax, float fMin, float fPos);  //returns 1 .. 0
float	InterpolateFloat(float f1, float f2, float fPos);
void	InterpolateFloat(float* fDest, float f1, float f2, float fPos);
float	FloatToleranceForDecimals(int iDecimals);
int		FloatWholeNumbers(float f);
void	Swap(float* f1, float* f2);
float	NotANumber(void);
BOOL	IsNotANumber(float* pf);


extern float gfNaN;


#endif //__FLOAT_HELPER_H__

