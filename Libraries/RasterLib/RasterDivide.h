/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_DIVIDE_H__
#define __RASTER_DIVIDE_H__
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif
#include "BaseLib/Define.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/PrimitiveTypes.h"


#if defined(_MSC_VER) && defined(_M_X64)
#define RASTER_MULTIPLY_HIGH(a, b)	__umulh(a, b)
#elif defined(__SIZEOF_INT128__)
#define RASTER_MULTIPLY_HIGH(a, b)	((uint64)(((unsigned __int128)(a) * (b)) >> 64))
#endif


//////////////////////////////////////////////////////////////////////////
//
// A vertex divides three numbers by the same distance.  One real division
// makes this, and RasterDivideFloor makes each of them from it.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE uint64 RasterDivideInverse(uint64 uiDivisor)
{
#ifdef RASTER_MULTIPLY_HIGH
	return ~(uint64)0 / uiDivisor;
#else
	return 0;
#endif
}


//////////////////////////////////////////////////////////////////////////
//
// Exactly what DivideFloor gives, for a divisor above 0.  Multiplying by the
// inverse can only come out a little low, and the remainder corrects that.
// Where there's no multiply that gives the high 64 bits this just divides.
//
//////////////////////////////////////////////////////////////////////////
static FORCE_INLINE int64 RasterDivideFloor(int64 iNumerator, uint64 uiDivisor, uint64 uiInverse)
{
#ifdef RASTER_MULTIPLY_HIGH
	uint64	uiNumerator;
	uint64	uiQuotient;
	uint64	uiRemainder;

	uiNumerator = (iNumerator < 0) ? (uint64)0 - (uint64)iNumerator : (uint64)iNumerator;
	uiQuotient = RASTER_MULTIPLY_HIGH(uiNumerator, uiInverse);
	uiRemainder = uiNumerator - uiQuotient * uiDivisor;
	while (uiRemainder >= uiDivisor)
	{
		uiQuotient++;
		uiRemainder -= uiDivisor;
	}
	if (iNumerator >= 0)
	{
		return (int64)uiQuotient;
	}
	return -(int64)uiQuotient - ((uiRemainder != 0) ? 1 : 0);
#else
	(void)uiInverse;
	return DivideFloor(iNumerator, (int64)uiDivisor);
#endif
}


#endif // __RASTER_DIVIDE_H__

