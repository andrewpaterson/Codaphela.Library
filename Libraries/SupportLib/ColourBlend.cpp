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
#include "ColourBlend.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// An opacity drawn over another: s + d (1 - s).  An opaque destination
// stays opaque and a transparent source leaves the destination unchanged.
//
//////////////////////////////////////////////////////////////////////////
uint8 BlendOverByte(size uiDest, size uiSource)
{
	return (uint8)(uiSource + (uiDest * (255 - uiSource) + 127) / 255);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// Erases the coverage from the destination: d (1 - c).
//
//////////////////////////////////////////////////////////////////////////
uint8 BlendEraseByte(size uiDest, size uiCoverage)
{
	return (uint8)((uiDest * (255 - uiCoverage) + 127) / 255);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// Moves the destination towards the source by the coverage:
// d (1 - c) + s c.
//
//////////////////////////////////////////////////////////////////////////
uint8 BlendLerpByte(size uiDest, size uiSource, size uiCoverage)
{
	return (uint8)((uiDest * (255 - uiCoverage) + uiSource * uiCoverage + 127) / 255);
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the source colour, uiCoverage 255ths of it, over the destination.
// Both are opacity, red, green then blue and neither is multiplied by its
// opacity.  The destination's colour counts by the opacity of it the
// source leaves showing, so drawing a colour over itself doesn't change it.
// No sum passes 255 * 255 + 255, so it all fits in a 16 bit size.
//
//////////////////////////////////////////////////////////////////////////
void BlendStraightOverPixel(uint8* puiDest, uint8* puiSource, size uiCoverage)
{
	size	uiSourceOpacity;
	size	uiDestOpacity;  //Of the destination, showing through the source.
	size	uiOpacity;
	size	i;

	uiSourceOpacity = (puiSource[0] * uiCoverage + 127) / 255;
	if ((puiDest[0] == 0) || (uiSourceOpacity == 255))
	{
		puiDest[0] = (uint8)uiSourceOpacity;
		for (i = 1; i < 4; i++)
		{
			puiDest[i] = puiSource[i];
		}
		return;
	}

	uiDestOpacity = (puiDest[0] * (255 - uiSourceOpacity) + 127) / 255;
	uiOpacity = uiSourceOpacity + uiDestOpacity;
	if (uiOpacity == 0)
	{
		return;
	}
	for (i = 1; i < 4; i++)
	{
		puiDest[i] = (uint8)((puiSource[i] * uiSourceOpacity + puiDest[i] * uiDestOpacity + uiOpacity / 2) / uiOpacity);
	}
	puiDest[0] = (uint8)uiOpacity;
}
