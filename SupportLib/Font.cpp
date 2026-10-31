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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "ImageCel.h"
#include "ImageAccessorCreator.h"
#include "Font.h"


//////////////////////////////////////////////////////////////////////////
//
// pImage must be laid out as pDescription says and be at least as big as
// the image the description was made for.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFont::Init(Ptr<CFontDescription> pDescription, Ptr<CImage> pImage)
{
	PreInit();

	mpDescription = pDescription;
	mpImage = pImage;
	mapGlyphs.Init();
	mapGlyphs.GrowTo(pDescription->NumGlyphs());
	mbLazy = false;
	mabHasPixels.Init();
	mpWhite = NULL;
	msColour = 0xFFFFFFFF;

	if (pImage.IsNull() || ((int32)pImage->GetWidth() < pDescription->GetImageWidth()) || ((int32)pImage->GetHeight() < pDescription->GetImageHeight()))
	{
		gcLogger.Error2(__METHOD__, " Font [", pDescription->GetFontName(), "] image is smaller than its description's.", NULL);
	}

	PostInit();

	return Ptr<CFont>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Class(void)
{
	M_Pointer(mpDescription);
	M_Pointer(mpImage);
	M_Embedded(mapGlyphs);
	U_Bool(mbLazy);
	U_Data(CArrayBit, mabHasPixels);
	M_Pointer(mpWhite);
	U_UInt32(msColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Free(void)
{
	mabHasPixels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint16 c, size uiLength)
{
	return GetGlyph((uint8*)&c, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint32 c, size uiLength)
{
	return GetGlyph((uint8*)&c, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the font doesn't have the character.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint8* puiBuffer, size uiLength)
{
	SGlyphDescription*	psDescription;

	psDescription = mpDescription->GetGlyph(puiBuffer, uiLength);
	if (psDescription == NULL)
	{
		return NULL;
	}
	return GetGlyph(psDescription);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next character.  NULL if the font doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(CUnicodeReader* pcUTFReader)
{
	SGlyphDescription*	psDescription;

	psDescription = mpDescription->GetGlyph(pcUTFReader);
	if (psDescription == NULL)
	{
		return NULL;
	}
	return GetGlyph(psDescription);
}


//////////////////////////////////////////////////////////////////////////
//
// The glyph is generated from its description the first time it is asked
// for.  psDescription must be from this font's description.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(SGlyphDescription* psDescription)
{
	Ptr<CGlyph>		pGlyph;

	//Characters can have been put into the description since the font was made.
	if (psDescription->uiIndex >= mapGlyphs.NumElements())
	{
		mapGlyphs.GrowTo(mpDescription->NumGlyphs());
	}

	pGlyph = mapGlyphs.Get(psDescription->uiIndex);
	if (pGlyph.IsNull())
	{
		//CreateCel draws the glyph's pixels first if the font is lazy.
		pGlyph = OMalloc<CGlyph>(CreateCel(psDescription), psDescription->iStep);
		mapGlyphs.Set(psDescription->uiIndex, pGlyph);
	}
	return pGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the font doesn't have the character.
//
//////////////////////////////////////////////////////////////////////////
SGlyphDescription* CFont::GetGlyphDescription(uint8* puiBuffer, size uiLength)
{
	return mpDescription->GetGlyph(puiBuffer, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// A new cel over the description's pixels in this font's image, drawn
// first if the font is lazy and hasn't drawn them yet.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CFont::CreateCel(SGlyphDescription* psDescription)
{
	Ptr<CImageCel>	pCel;

	EnsurePixels(psDescription);
	pCel = OMalloc<CImageCel>(mpImage, 0, 0, 0, 0);
	pCel->SetSubImage(&psDescription->cSubImage);
	return pCel;
}


//////////////////////////////////////////////////////////////////////////
//
// The image is empty (see CTrueTypeFontFactory::CreateImage) and the
// description's rasteriser draws each glyph's pixels into it, in white,
// when the glyph is generated.
//
//////////////////////////////////////////////////////////////////////////
void CFont::SetLazy(void)
{
	mbLazy = true;
	mpWhite = NULL;
	msColour = 0xFFFFFFFF;
	mabHasPixels.ReInit();
	mabHasPixels.GrowTo(mpDescription->NumGlyphs());
	mabHasPixels.Zero();
}


//////////////////////////////////////////////////////////////////////////
//
// The image is empty and each glyph's pixels are pWhite's (a lazy white
// font with the same description) recoloured to sColour when the glyph
// is generated.
//
//////////////////////////////////////////////////////////////////////////
void CFont::SetLazyColour(Ptr<CFont> pWhite, ARGB32 sColour)
{
	SetLazy();
	mpWhite = pWhite;
	msColour = sColour;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the image has the glyph's pixels.  A font that isn't lazy has
// them all.
//
//////////////////////////////////////////////////////////////////////////
bool CFont::HasPixels(SGlyphDescription* psDescription)
{
	if (!mbLazy)
	{
		return true;
	}
	if (psDescription->uiIndex >= mabHasPixels.NumElements())
	{
		return false;
	}
	return mabHasPixels.Get(psDescription->uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the glyph's pixels into a lazy font's image if they aren't there.
// They are drawn before any cel of the glyph so its blitter sees them.
//
//////////////////////////////////////////////////////////////////////////
void CFont::EnsurePixels(SGlyphDescription* psDescription)
{
	Ptr<CGlyphRasteriser>	pRasteriser;
	lsize					uiNumBits;

	if (HasPixels(psDescription))
	{
		return;
	}

	if (mpWhite.IsNotNull())
	{
		mpWhite->EnsurePixels(psDescription);
		RecolourGlyph(psDescription);
	}
	else
	{
		pRasteriser = mpDescription->GetRasteriser();
		if (pRasteriser.IsNotNull())
		{
			pRasteriser->RasteriseGlyph(psDescription, mpImage);
		}
	}

	//Characters can have been put into the description since the font was made.
	if (psDescription->uiIndex >= mabHasPixels.NumElements())
	{
		uiNumBits = mabHasPixels.NumElements();
		mabHasPixels.GrowTo(mpDescription->NumGlyphs());
		while (uiNumBits < mabHasPixels.NumElements())
		{
			mabHasPixels.Set(uiNumBits, false);
			uiNumBits++;
		}
	}
	mabHasPixels.Set(psDescription->uiIndex, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the glyph's pixels from the white font, setting red, green and
// blue to the colour and scaling the opacity by its alpha, as
// CFontColours does to a whole image.
//
//////////////////////////////////////////////////////////////////////////
void CFont::RecolourGlyph(SGlyphDescription* psDescription)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcSource;
	CImageAccessor*			pcDest;
	Ptr<CImage>				pWhiteImage;
	uint8					auiPixel[4];
	uint8					uiOpacity;
	uint8					uiAlpha;
	int32					iLeft;
	int32					iTop;
	uint32					x;
	uint32					y;

	pWhiteImage = mpWhite->GetImage();
	cCreator.Init(&pWhiteImage);
	cCreator.AddAccess(IMAGE_OPACITY, PT_uint8);
	pcSource = cCreator.CreateAndKill();
	cCreator.Init(&mpImage);
	cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
	pcDest = cCreator.CreateAndKill();

	auiPixel[0] = Get8BitRedColour(msColour);
	auiPixel[1] = Get8BitGreenColour(msColour);
	auiPixel[2] = Get8BitBlueColour(msColour);
	uiAlpha = Get8BitAlphaColour(msColour);
	iLeft = psDescription->cSubImage.mcImageRect.msTopLeft.x;
	iTop = psDescription->cSubImage.mcImageRect.msTopLeft.y;
	for (y = 0; y < (uint32)psDescription->cSubImage.GetImageHeight(); y++)
	{
		for (x = 0; x < (uint32)psDescription->cSubImage.GetImageWidth(); x++)
		{
			pcSource->Get(iLeft + x, iTop + y, &uiOpacity);
			if (uiAlpha != 0xFF)
			{
				uiOpacity = (uint8)(((uint32)uiOpacity * uiAlpha + 127) / 255);
			}
			auiPixel[3] = uiOpacity;
			pcDest->Set(iLeft + x, iTop + y, auiPixel);
		}
	}
	pcDest->Kill();
	pcSource->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The number of glyphs asked for so far.
//
//////////////////////////////////////////////////////////////////////////
size CFont::NumGeneratedGlyphs(void)
{
	return mapGlyphs.NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The keys of every character, whether its glyph has been generated or
// not.
//
//////////////////////////////////////////////////////////////////////////
bool CFont::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	return mpDescription->StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	return mpDescription->Iterate(psIterator, pvKey, piKeySize, iMaxKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
// The metrics are the description's.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontDescription> CFont::GetDescription(void) { return mpDescription; }
Ptr<CImage> CFont::GetImage(void) { return mpImage; }
bool CFont::IsLazy(void) { return mbLazy; }
bool CFont::Is(char* szName) { return mpDescription->Is(szName); }
char* CFont::GetFontName(void) { return mpDescription->GetFontName(); }
size CFont::CalculateWidth(char* szText) { return mpDescription->CalculateWidth(szText); }
bool CFont::IsWhitespace(uint16 c) { return mpDescription->IsWhitespace(c); }
int16 CFont::GetSpaceWidth(void) { return mpDescription->GetSpaceWidth(); }
int16 CFont::GetAscent(void) { return mpDescription->GetAscent(); }
int16 CFont::GetDescent(void) { return mpDescription->GetDescent(); }
int16 CFont::GetLineHeight(void) { return mpDescription->GetLineHeight(); }
int16 CFont::GetTabSpaceCount(void) { return mpDescription->GetTabSpaceCount(); }
size CFont::GetAverageWidth(void) { return mpDescription->GetAverageWidth(); }
size CFont::GetAverageHeight(void) { return mpDescription->GetAverageHeight(); }
bool CFont::IsFixedWidth(void) { return mpDescription->IsFixedWidth(); }
size CFont::NumGlyphs(void) { return mpDescription->NumGlyphs(); }

