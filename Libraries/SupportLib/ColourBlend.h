/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COLOUR_BLEND_H__
#define __COLOUR_BLEND_H__
#include "BaseLib/PrimitiveTypes.h"


//Integer blends of 8 bit values where 0 is none and 255 is all.  Results are rounded to the nearest.  They are
//worked out in size, the narrowest integer that holds 255 * 255: 16 bits on the W65C816.
uint8	BlendOverByte(size uiDest, size uiSource);
uint8	BlendEraseByte(size uiDest, size uiCoverage);
uint8	BlendLerpByte(size uiDest, size uiSource, size uiCoverage);
void	BlendStraightOverPixel(uint8* puiDest, uint8* puiSource, size uiCoverage);


//uiSource + uiDest * (255 - uiAlpha) / 255, rounded and never more than 255.  Inline as whole images are blended with it.
inline uint8 BlendPremultipliedByte(size uiSource, size uiDest, size uiInverseAlpha)
{
	size	uiResult;

	uiResult = uiSource + (uiDest * uiInverseAlpha + 127) / 255;
	if (uiResult > 255)
	{
		uiResult = 255;
	}
	return (uint8)uiResult;
}


#endif // __COLOUR_BLEND_H__

