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
#include <math.h>
#include "BaseLib/UTF8.h"
#include "ImageCel.h"
#include "TrueTypeFontFactory.h"


//Transparent pixels between glyphs in the image.
#define TRUE_TYPE_FONT_GLYPH_SPACING	1


//////////////////////////////////////////////////////////////////////////
//
// fPixelSize is the height of one em in pixels.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFontFactory::Init(CTrueTypeFont* pcFont, float32 fPixelSize)
{
	mpcFont = pcFont;
	mcRasteriser.Init(pcFont, fPixelSize);
	macCharacters.Init();
	miAscent = (int)ceilf(mcRasteriser.GetAscent());
	miDescent = (int)ceilf(mcRasteriser.GetDescent());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFontFactory::Kill(void)
{
	macCharacters.Kill();
	mcRasteriser.Kill();
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises each character (once, control characters are ignored) into
// an image iImageWidth wide and as high as needed.  Returns NULL if a
// glyph is wider than the image.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeFontFactory::Rasterise(char* szUTF8Characters, CImageColourRGB* pcColour, int iImageWidth)
{
	int						iImageHeight;
	Ptr<CImage>				pImage;
	size					i;
	STrueTypeFontCharacter*	psCharacter;

	macCharacters.ReInit();
	if (!AddCharacters(szUTF8Characters) || !Pack(iImageWidth, &iImageHeight))
	{
		return NULL;
	}

	pImage = OMalloc<CImage>(iImageWidth, iImageHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	pImage->Black();
	for (i = 0; i < macCharacters.NumElements(); i++)
	{
		psCharacter = macCharacters.Get(i);
		if (psCharacter->iImageWidth > 0)
		{
			CopyGlyph(pImage, psCharacter, pcColour);
		}
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes a font from the characters last rasterised into pImage.  Each
// glyph's full size is its advance by the ascent plus descent, with the
// top of the line at the top.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTrueTypeFontFactory::Generate(char* szName, Ptr<CImage> pImage)
{
	Ptr<CFont>				pFont;
	Ptr<CImageCel>			pCel;
	size					i;
	STrueTypeFontCharacter*	psCharacter;
	int						iSpaceWidth;
	int						iLineHeight;
	int						iTopOffset;

	if (pImage.IsNull())
	{
		return NULL;
	}

	iSpaceWidth = (int)floorf(mcRasteriser.GetAdvance(mpcFont->GetGlyphIndex(' ')) + 0.5f);
	iLineHeight = miAscent + miDescent;
	pFont = OMalloc<CFont>(szName, (int16)iSpaceWidth, (int16)miAscent, (int16)miDescent);

	for (i = 0; i < macCharacters.NumElements(); i++)
	{
		psCharacter = macCharacters.Get(i);
		if (psCharacter->iImageWidth > 0)
		{
			//Offsets are negative where a glyph overhangs its advance or the line.
			iTopOffset = miAscent + psCharacter->iTop;
			pCel = OMalloc<CImageCel>(pImage,
									  psCharacter->iImageLeft,
									  psCharacter->iImageTop,
									  psCharacter->iImageLeft + psCharacter->iImageWidth,
									  psCharacter->iImageTop + psCharacter->iImageHeight,
									  psCharacter->iBearing,
									  iTopOffset,
									  psCharacter->iAdvance - psCharacter->iBearing - psCharacter->iImageWidth,
									  iLineHeight - iTopOffset - psCharacter->iImageHeight);
		}
		else
		{
			pCel = OMalloc<CImageCel>(pImage, 0, 0, 0, 0, psCharacter->iAdvance, 0, 0, iLineHeight);
		}
		pFont->PutGlyph(psCharacter->uiCodePoint, psCharacter->uiCodePointLength, pCel, (int16)psCharacter->iAdvance);
	}
	pFont->SetImage(pImage);
	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTrueTypeFontFactory::NumCharacters(void)
{
	return macCharacters.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels above the baseline, rounded up.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFontFactory::GetAscent(void)
{
	return miAscent;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels below the baseline, rounded up.
//
//////////////////////////////////////////////////////////////////////////
int CTrueTypeFontFactory::GetDescent(void)
{
	return miDescent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFontFactory::AddCharacters(char* szUTF8Characters)
{
	CUTF8					cUTF8;
	uint32					uiCodePoint;
	STrueTypeFontCharacter*	psCharacter;
	STrueTypePixelBounds	sBounds;

	cUTF8.Init(szUTF8Characters);
	for (;;)
	{
		uiCodePoint = cUTF8.GetCodePointUint32();
		if (uiCodePoint == 0)
		{
			break;
		}
		if (cUTF8.IsError(uiCodePoint))
		{
			cUTF8.Kill();
			return false;
		}
		if ((uiCodePoint < 0x20) || ((uiCodePoint >= 0x7F) && (uiCodePoint < 0xA0)) || HasCharacter(uiCodePoint))
		{
			continue;
		}

		psCharacter = macCharacters.Add();
		psCharacter->uiCodePoint = uiCodePoint;
		psCharacter->uiCodePointLength = cUTF8.GetUnicodeCodePointLength(uiCodePoint);
		psCharacter->iGlyph = mpcFont->GetGlyphIndex(uiCodePoint);
		psCharacter->iAdvance = (int)floorf(mcRasteriser.GetAdvance(psCharacter->iGlyph) + 0.5f);
		psCharacter->iImageLeft = 0;
		psCharacter->iImageTop = 0;
		if (mcRasteriser.GetGlyphBounds(psCharacter->iGlyph, 0.0f, &sBounds))
		{
			psCharacter->iBearing = sBounds.iLeft;
			psCharacter->iTop = sBounds.iTop;
			psCharacter->iImageWidth = sBounds.iRight - sBounds.iLeft;
			psCharacter->iImageHeight = sBounds.iBottom - sBounds.iTop;
		}
		else
		{
			psCharacter->iBearing = 0;
			psCharacter->iTop = 0;
			psCharacter->iImageWidth = 0;
			psCharacter->iImageHeight = 0;
		}
	}
	cUTF8.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFontFactory::HasCharacter(uint32 uiCodePoint)
{
	size	i;

	for (i = 0; i < macCharacters.NumElements(); i++)
	{
		if (macCharacters.Get(i)->uiCodePoint == uiCodePoint)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Places the glyphs left to right in rows, in the order of the
// characters.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueTypeFontFactory::Pack(int iImageWidth, int* piImageHeight)
{
	size					i;
	STrueTypeFontCharacter*	psCharacter;
	int						x;
	int						y;
	int						iRowHeight;

	x = 0;
	y = 0;
	iRowHeight = 0;
	for (i = 0; i < macCharacters.NumElements(); i++)
	{
		psCharacter = macCharacters.Get(i);
		if (psCharacter->iImageWidth == 0)
		{
			continue;
		}
		if (psCharacter->iImageWidth > iImageWidth)
		{
			return false;
		}

		if (x + psCharacter->iImageWidth > iImageWidth)
		{
			x = 0;
			y += iRowHeight + TRUE_TYPE_FONT_GLYPH_SPACING;
			iRowHeight = 0;
		}
		psCharacter->iImageLeft = x;
		psCharacter->iImageTop = y;
		x += psCharacter->iImageWidth + TRUE_TYPE_FONT_GLYPH_SPACING;
		if (psCharacter->iImageHeight > iRowHeight)
		{
			iRowHeight = psCharacter->iImageHeight;
		}
	}

	*piImageHeight = y + iRowHeight;
	if (*piImageHeight == 0)
	{
		*piImageHeight = 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises a glyph into its place in the image.  The coverage is the
// opacity and every pixel is the colour.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFontFactory::CopyGlyph(Ptr<CImage> pImage, STrueTypeFontCharacter* psCharacter, CImageColourRGB* pcColour)
{
	Ptr<CImage>		pGlyphImage;
	int				iLeft;
	int				iTop;
	uint8*			puiGlyph;
	uint8*			puiDest;
	int				x;
	int				y;
	uint8			uiRed;
	uint8			uiGreen;
	uint8			uiBlue;

	pGlyphImage = mcRasteriser.RasteriseGlyph(psCharacter->iGlyph, &iLeft, &iTop);
	if (pGlyphImage.IsNull())
	{
		return;
	}

	uiRed = (uint8)(pcColour->r * 255.0f + 0.5f);
	uiGreen = (uint8)(pcColour->g * 255.0f + 0.5f);
	uiBlue = (uint8)(pcColour->b * 255.0f + 0.5f);

	//Opacity, red, green and blue bytes.
	puiGlyph = (uint8*)pGlyphImage->GetData();
	for (y = 0; y < psCharacter->iImageHeight; y++)
	{
		puiDest = &((uint8*)pImage->GetData())[((psCharacter->iImageTop + y) * pImage->GetWidth() + psCharacter->iImageLeft) * 4];
		for (x = 0; x < psCharacter->iImageWidth; x++)
		{
			puiDest[0] = puiGlyph[y * psCharacter->iImageWidth + x];
			puiDest[1] = uiRed;
			puiDest[2] = uiGreen;
			puiDest[3] = uiBlue;
			puiDest += 4;
		}
	}
}
