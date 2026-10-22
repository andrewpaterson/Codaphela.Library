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
#include "BaseLib/UTF8.h"
#include "BaseLib/FixedPoint.h"
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
	miPixelSize64 = (int32)(fPixelSize * 64.0f + 0.5f);
	macCharacters.Init();
	miAscent = FixedCeiling(mcRasteriser.GetAscentSubpixels(), TRUE_TYPE_SUBPIXEL_BITS);
	miDescent = FixedCeiling(mcRasteriser.GetDescentSubpixels(), TRUE_TYPE_SUBPIXEL_BITS);
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
// Measures each character (once, control characters are ignored) and
// packs them into the layout of an image iImageWidth wide and as high as
// needed.  Each glyph's full size is its advance by the ascent plus
// descent, with the top of the line at the top.  Returns NULL if a glyph
// is wider than the image.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontDescription> CTrueTypeFontFactory::Describe(char* szName, char* szUTF8Characters, int iImageWidth)
{
	Ptr<CFontDescription>	pDescription;
	CSubImage				cSubImage;
	size					i;
	STrueTypeFontCharacter*	psCharacter;
	int						iImageHeight;
	int						iSpaceWidth;
	int						iLineHeight;
	int						iTopOffset;

	macCharacters.ReInit();
	if (!AddCharacters(szUTF8Characters) || !Pack(iImageWidth, &iImageHeight))
	{
		return NULL;
	}

	iSpaceWidth = FixedRound(mcRasteriser.GetAdvanceSubpixels(mpcFont->GetGlyphIndex(' ')), TRUE_TYPE_SUBPIXEL_BITS);
	iLineHeight = miAscent + miDescent;
	pDescription = OMalloc<CFontDescription>(szName, (int16)iSpaceWidth, (int16)miAscent, (int16)miDescent, iImageWidth, iImageHeight);

	for (i = 0; i < macCharacters.NumElements(); i++)
	{
		psCharacter = macCharacters.Get(i);
		if (psCharacter->iImageWidth > 0)
		{
			//Offsets are negative where a glyph overhangs its advance or the line.
			iTopOffset = miAscent + psCharacter->iTop;
			cSubImage.Init(psCharacter->iImageLeft,
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
			cSubImage.Init(0, 0, 0, 0, psCharacter->iAdvance, 0, 0, iLineHeight);
		}
		pDescription->PutGlyph(psCharacter->uiCodePoint, psCharacter->uiCodePointLength, &cSubImage, (int16)psCharacter->iAdvance, psCharacter->iGlyph);
	}
	DescribeScriptAndLines(pDescription);
	return pDescription;
}


//////////////////////////////////////////////////////////////////////////
//
// The underline from the font's post table and the strikeout, superscript
// and subscript from its OS/2 table.  Without a table the description's
// own estimates are kept.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFontFactory::DescribeScriptAndLines(Ptr<CFontDescription> pDescription)
{
	int16	iThickness;
	int		iUnitsPerEm;

	if (mpcFont->HasPost())
	{
		iThickness = ScaleFontUnits(mpcFont->GetUnderlineThickness());
		pDescription->SetUnderline(ScaleFontUnits(-mpcFont->GetUnderlinePosition()), (iThickness < 1) ? 1 : iThickness);
	}
	if (mpcFont->HasOS2())
	{
		iThickness = ScaleFontUnits(mpcFont->GetStrikeoutSize());
		pDescription->SetStrikeout(ScaleFontUnits(-mpcFont->GetStrikeoutPosition()), (iThickness < 1) ? 1 : iThickness);

		iUnitsPerEm = mpcFont->GetUnitsPerEm();
		if (mpcFont->GetSuperscriptYSize() > 0)
		{
			pDescription->SetSuperscript((float32)mpcFont->GetSuperscriptYSize() / (float32)iUnitsPerEm, ScaleFontUnits(mpcFont->GetSuperscriptYOffset()));
		}
		if (mpcFont->GetSubscriptYSize() > 0)
		{
			pDescription->SetSubscript((float32)mpcFont->GetSubscriptYSize() / (float32)iUnitsPerEm, ScaleFontUnits(mpcFont->GetSubscriptYOffset()));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Font units as whole pixels at the factory's size, rounded half away from
// zero.  The arithmetic is integer so every build gets the same pixels.
//
//////////////////////////////////////////////////////////////////////////
int16 CTrueTypeFontFactory::ScaleFontUnits(int iUnits)
{
	int64	iScaled;
	int64	iDivisor;

	iScaled = (int64)iUnits * miPixelSize64;
	iDivisor = (int64)mpcFont->GetUnitsPerEm() * 64;
	if (iScaled >= 0)
	{
		return (int16)((iScaled + iDivisor / 2) / iDivisor);
	}
	return (int16)(-((-iScaled + iDivisor / 2) / iDivisor));
}


//////////////////////////////////////////////////////////////////////////
//
// An empty image (no opacity anywhere) the size of pDescription's, with
// opacity then red, green and blue in eColourOrder (CCO_Unknown is red,
// green, blue), as Rasterise and then CImageColourConverter make them.
// For fonts that draw their glyphs as they are asked for.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeFontFactory::CreateImage(Ptr<CFontDescription> pDescription, EColourOrder eColourOrder)
{
	CColourFormatHelper		cHelper;
	CArrayChannel			asChannels;
	SChannel*				psChannel;
	size					i;
	Ptr<CImage>				pImage;

	if (eColourOrder == CCO_Unknown)
	{
		eColourOrder = CCO_RGB;
	}
	cHelper.Init(CFT_RGB, eColourOrder, CRGB_Unknown, ARGB_Unknown);

	asChannels.Init();
	psChannel = asChannels.Add();
	psChannel->eType = PT_uint8;
	psChannel->iChannel = IMAGE_OPACITY;
	psChannel->bReverse = false;
	for (i = 0; i < 3; i++)
	{
		psChannel = asChannels.Add();
		psChannel->eType = PT_uint8;
		psChannel->iChannel = (size)cHelper.GetColourChannel(i);
		psChannel->bReverse = false;
	}
	pImage = OMalloc<CImage>(pDescription->GetImageWidth(), pDescription->GetImageHeight(), &asChannels);
	asChannels.Kill();
	pImage->Black();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises every character of pDescription into a new image in a
// colour, each glyph where the description says it is.  pDescription must
// have been described from the same TrueType font and pixel size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTrueTypeFontFactory::Rasterise(Ptr<CFontDescription> pDescription, CImageColourRGB* pcColour)
{
	Ptr<CImage>					pImage;
	SIndexTreeMemoryIterator	sIter;
	uint8						auiKey[FONT_MAX_KEY_LENGTH];
	size						uiKeyLength;
	bool						bExists;
	uint32						uiCodePoint;
	size						i;
	SGlyphDescription*			psDescription;

	if (pDescription.IsNull())
	{
		return NULL;
	}

	pImage = OMalloc<CImage>(pDescription->GetImageWidth(), pDescription->GetImageHeight(), PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	pImage->Black();

	bExists = pDescription->StartIteration(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH);
	while (bExists)
	{
		psDescription = pDescription->GetGlyph(auiKey, uiKeyLength);
		if (psDescription->cSubImage.GetImageWidth() > 0)
		{
			//The key is the code point's bytes, little endian.
			uiCodePoint = 0;
			for (i = 0; i < uiKeyLength; i++)
			{
				uiCodePoint |= ((uint32)auiKey[i]) << (i * 8);
			}
			CopyGlyph(pImage, mpcFont->GetGlyphIndex(uiCodePoint), &psDescription->cSubImage, pcColour);
		}
		bExists = pDescription->Iterate(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH);
	}
	return pImage;
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
		psCharacter->iAdvance = FixedRound(mcRasteriser.GetAdvanceSubpixels(psCharacter->iGlyph), TRUE_TYPE_SUBPIXEL_BITS);
		psCharacter->iImageLeft = 0;
		psCharacter->iImageTop = 0;
		if (mcRasteriser.GetGlyphBounds(psCharacter->iGlyph, 0, &sBounds))
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
// A colour channel from 0.0 to 1.0 as a byte.
//
//////////////////////////////////////////////////////////////////////////
uint8 CTrueTypeFontFactory::ColourByte(float32 fChannel)
{
	int32	iChannel;

	iChannel = FloatToIntScaled(fChannel, 255);
	return (uint8)((iChannel < 0) ? 0 : ((iChannel > 255) ? 255 : iChannel));
}


//////////////////////////////////////////////////////////////////////////
//
// Rasterises a glyph into its place in the image.  The coverage is the
// opacity and every pixel is the colour.
//
//////////////////////////////////////////////////////////////////////////
void CTrueTypeFontFactory::CopyGlyph(Ptr<CImage> pImage, int iGlyph, CSubImage* pcSubImage, CImageColourRGB* pcColour)
{
	Ptr<CImage>		pGlyphImage;
	int				iLeft;
	int				iTop;
	int				iImageLeft;
	int				iImageTop;
	int				iImageWidth;
	int				iImageHeight;
	uint8*			puiGlyph;
	uint8*			puiDest;
	int				x;
	int				y;
	uint8			uiRed;
	uint8			uiGreen;
	uint8			uiBlue;

	pGlyphImage = mcRasteriser.RasteriseGlyph(iGlyph, &iLeft, &iTop);
	if (pGlyphImage.IsNull())
	{
		return;
	}

	iImageLeft = pcSubImage->mcImageRect.msTopLeft.x;
	iImageTop = pcSubImage->mcImageRect.msTopLeft.y;
	iImageWidth = pcSubImage->GetImageWidth();
	iImageHeight = pcSubImage->GetImageHeight();

	uiRed = ColourByte(pcColour->r);
	uiGreen = ColourByte(pcColour->g);
	uiBlue = ColourByte(pcColour->b);

	//Opacity, red, green and blue bytes.
	puiGlyph = (uint8*)pGlyphImage->GetData();
	for (y = 0; y < iImageHeight; y++)
	{
		puiDest = &((uint8*)pImage->GetData())[((iImageTop + y) * pImage->GetWidth() + iImageLeft) * 4];
		for (x = 0; x < iImageWidth; x++)
		{
			puiDest[0] = puiGlyph[y * iImageWidth + x];
			puiDest[1] = uiRed;
			puiDest[2] = uiGreen;
			puiDest[3] = uiBlue;
			puiDest += 4;
		}
	}
}
