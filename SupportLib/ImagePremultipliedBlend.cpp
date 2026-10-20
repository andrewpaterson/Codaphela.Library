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
#include "ImageBlitterContext.h"
#include "ImagePremultipliedBlend.h"


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
//
//////////////////////////////////////////////////////////////////////////
bool BlendPremultipliedImage(Ptr<CImage> pSource, Ptr<CImage> pDest, int32 iDestX, int32 iDestY)
{
	CColourFormatHelper		cSourceFormatHelper;
	CColourFormatHelper		cDestFormatHelper;
	CImageBlitterContext	cContext;
	bool					bDestOpacity;
	int32					iLeft;
	int32					iTop;
	int32					iRight;
	int32					iBottom;
	int32					x;
	int32					y;
	uint8*					puiSourceData;
	uint8*					puiDestData;
	uint8*					puiSourcePixel;
	uint8*					puiDestPixel;
	uint32					uiInverseAlpha;
	size					uiColour;

	if ((pSource->GetColourBits() != CRGB_24bit) || (pDest->GetColourBits() != CRGB_24bit) ||
		(pSource->GetAlphaBits() != ARGB_8bit) || !pSource->HasChannel(IMAGE_OPACITY) ||
		(pSource->GetColourOrder() != pDest->GetColourOrder()))
	{
		return false;
	}

	//An image with an ignored channel (e.g. RGBX) has alpha bits but no opacity channel.
	bDestOpacity = pDest->HasChannel(IMAGE_OPACITY);
	if (bDestOpacity && (pDest->GetAlphaBits() != ARGB_8bit))
	{
		return false;
	}

	cSourceFormatHelper.Init(pSource->GetColourFormat(), pSource->GetColourOrder(), pSource->GetColourBits(), pSource->GetAlphaBits());
	cDestFormatHelper.Init(pDest->GetColourFormat(), pDest->GetColourOrder(), pDest->GetColourBits(), pDest->GetAlphaBits());
	cContext.Init(pSource, pDest, &cSourceFormatHelper, &cDestFormatHelper);

	//The source pixels that land inside the destination.
	iLeft = (iDestX < 0) ? -iDestX : 0;
	iTop = (iDestY < 0) ? -iDestY : 0;
	iRight = (int32)pSource->GetWidth();
	if (iDestX + iRight > (int32)pDest->GetWidth())
	{
		iRight = (int32)pDest->GetWidth() - iDestX;
	}
	iBottom = (int32)pSource->GetHeight();
	if (iDestY + iBottom > (int32)pDest->GetHeight())
	{
		iBottom = (int32)pDest->GetHeight() - iDestY;
	}

	puiSourceData = (uint8*)pSource->GetData();
	puiDestData = (uint8*)pDest->GetData();
	for (y = iTop; y < iBottom; y++)
	{
		puiSourcePixel = &puiSourceData[y * cContext.miSourceRowStride + iLeft * cContext.miSourcePixelStride];
		puiDestPixel = &puiDestData[(y + iDestY) * cContext.miDestRowStride + (iLeft + iDestX) * cContext.miDestPixelStride];
		for (x = iLeft; x < iRight; x++)
		{
			uiInverseAlpha = 255 - puiSourcePixel[cContext.muiSourceAlphaOffset];
			for (uiColour = 0; uiColour < 3; uiColour++)
			{
				puiDestPixel[cContext.muiDestColourOffset + uiColour] = BlendPremultipliedByte(puiSourcePixel[cContext.muiSourceColourOffset + uiColour], puiDestPixel[cContext.muiDestColourOffset + uiColour], uiInverseAlpha);
			}
			if (bDestOpacity)
			{
				puiDestPixel[cContext.muiDestAlphaOffset] = BlendPremultipliedByte(puiSourcePixel[cContext.muiSourceAlphaOffset], puiDestPixel[cContext.muiDestAlphaOffset], uiInverseAlpha);
			}

			puiSourcePixel += cContext.miSourcePixelStride;
			puiDestPixel += cContext.miDestPixelStride;
		}
	}
	return true;
}
