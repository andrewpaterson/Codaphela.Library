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


//Integer blends of 8 bit values where 0 is none and 255 is all.  Results are rounded to the nearest.
uint8	BlendOverByte(uint32 uiDest, uint32 uiSource);
uint8	BlendEraseByte(uint32 uiDest, uint32 uiCoverage);
uint8	BlendLerpByte(uint32 uiDest, uint32 uiSource, uint32 uiCoverage);
uint8	BlendPremultipliedByte(uint32 uiSource, uint32 uiDest, uint32 uiInverseAlpha);
void	BlendStraightOverPixel(uint8* puiDest, uint8* puiSource, int iCoverage);


#endif // __COLOUR_BLEND_H__

