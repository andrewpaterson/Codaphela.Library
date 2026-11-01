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
#include "StandardLib/ClassDefines.h"
#include "ImageAccessorCreator.h"
#include "TrueTypeRasteriser.h"
#include "FontDescription.h"
#include "TrueTypeFontFactory.h"
#include "TrueTypeGlyphRasteriser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeGlyphRasteriser::Init(Ptr<CTrueTypeFace> pFace, float32 fPixelSize, EFontTurn eTurn)
{
	PreInit();

	CGlyphRasteriser::Init();
	mpFace = pFace;
	mfPixelSize = fPixelSize;
	meTurn = eTurn;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeGlyphRasteriser::Class(void)
{
	CGlyphRasteriser::Class();
	M_Pointer(mpFace);
	U_Float32(mfPixelSize);
	U_Enum(meTurn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeGlyphRasteriser::Free(void)
{
	CGlyphRasteriser::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the glyph's coverage as the opacity, and white, into its place in
// pImage, whatever the image's channel order.  A glyph with no pixels
// (such as a space) has nothing to draw.  A turned font's glyphs are
// drawn turned (see EFontTurn).
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeGlyphRasteriser::RasteriseGlyph(SGlyphDescription* psDescription, Ptr<CImage> pImage)
{
	CTrueTypeRasteriser		cRasteriser;
	Ptr<CImage>				pGlyphImage;
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	uint8*					puiCoverage;
	uint8					auiPixel[4];
	int						iLeft;
	int						iTop;
	int32					iImageLeft;
	int32					iImageTop;
	int32					iImageWidth;
	int32					iImageHeight;
	uint32					x;
	uint32					y;
	int32					iGlyphWidth;

	iImageWidth = psDescription->cSubImage.GetImageWidth();
	iImageHeight = psDescription->cSubImage.GetImageHeight();
	if ((iImageWidth <= 0) || (iImageHeight <= 0) || (psDescription->iSourceGlyph < 0))
	{
		return true;
	}

	cRasteriser.Init(mpFace->GetFont(), mfPixelSize);
	pGlyphImage = cRasteriser.RasteriseGlyph(psDescription->iSourceGlyph, &iLeft, &iTop);
	cRasteriser.Kill();
	if (pGlyphImage.IsNull())
	{
		return false;
	}

	iImageLeft = psDescription->cSubImage.mcImageRect.msTopLeft.x;
	iImageTop = psDescription->cSubImage.mcImageRect.msTopLeft.y;
	iGlyphWidth = (meTurn == FT_Clockwise) ? iImageHeight : iImageWidth;

	cCreator.Init(&pImage);
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
	pcAccessor = cCreator.CreateAndKill();

	auiPixel[0] = 0xFF;
	auiPixel[1] = 0xFF;
	auiPixel[2] = 0xFF;
	puiCoverage = (uint8*)pGlyphImage->GetData();
	for (y = 0; y < (uint32)iImageHeight; y++)
	{
		for (x = 0; x < (uint32)iImageWidth; x++)
		{
			auiPixel[3] = puiCoverage[TurnedGlyphOffset(meTurn, x, y, iGlyphWidth, iImageWidth)];
			pcAccessor->Set(iImageLeft + x, iImageTop + y, auiPixel);
		}
	}
	pcAccessor->Kill();
	return true;
}

