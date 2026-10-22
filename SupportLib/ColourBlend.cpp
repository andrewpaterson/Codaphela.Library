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
#include "BaseLib/FixedPoint.h"
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
uint8 BlendOverByte(uint32 uiDest, uint32 uiSource)
{
	return (uint8)(uiSource + DivideRound((int64)uiDest * (255 - uiSource), 255));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// Erases the coverage from the destination: d (1 - c).
//
//////////////////////////////////////////////////////////////////////////
uint8 BlendEraseByte(uint32 uiDest, uint32 uiCoverage)
{
	return (uint8)DivideRound((int64)uiDest * (255 - uiCoverage), 255);
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
uint8 BlendLerpByte(uint32 uiDest, uint32 uiSource, uint32 uiCoverage)
{
	return (uint8)DivideRound((int64)uiDest * (255 - uiCoverage) + (int64)uiSource * uiCoverage, 255);
}


//////////////////////////////////////////////////////////////////////////
//
// uiSource + uiDest * (255 - uiAlpha) / 255, rounded and never more than
// 255.
//
//////////////////////////////////////////////////////////////////////////
uint8 BlendPremultipliedByte(uint32 uiSource, uint32 uiDest, uint32 uiInverseAlpha)
{
	uint32	uiResult;

	uiResult = uiSource + (uiDest * uiInverseAlpha + 127) / 255;
	if (uiResult > 255)
	{
		uiResult = 255;
	}
	return (uint8)uiResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the source colour, iCoverage 256ths of it, over the destination.
// Both are opacity, red, green then blue and neither is multiplied by its
// opacity.  The result is rounded to the nearest so drawing a colour over
// itself doesn't change it.
//
//////////////////////////////////////////////////////////////////////////
void BlendStraightOverPixel(uint8* puiDest, uint8* puiSource, int iCoverage)
{
	int64	iSourceOpacity;  //In 65280ths (255 * 256).
	int64	iDestOpacity;
	int64	iOpacity;  //In 65280ths of 65280ths.
	int		i;

	iSourceOpacity = (int64)puiSource[0] * iCoverage;
	if ((puiDest[0] == 0) || (iSourceOpacity == 65280))
	{
		puiDest[0] = (uint8)((iSourceOpacity + 128) / 256);
		for (i = 1; i < 4; i++)
		{
			puiDest[i] = puiSource[i];
		}
		return;
	}

	iDestOpacity = (int64)puiDest[0] * 256;
	iOpacity = iSourceOpacity * 65280 + iDestOpacity * (65280 - iSourceOpacity);
	if (iOpacity == 0)
	{
		return;
	}
	for (i = 1; i < 4; i++)
	{
		puiDest[i] = (uint8)(((int64)puiSource[i] * iSourceOpacity * 65280 + (int64)puiDest[i] * iDestOpacity * (65280 - iSourceOpacity) + iOpacity / 2) / iOpacity);
	}
	puiDest[0] = (uint8)((iOpacity + 65280 * 128) / (65280 * 256));
}

