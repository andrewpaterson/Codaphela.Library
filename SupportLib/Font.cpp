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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Free(void)
{
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
// A new cel over the description's pixels in this font's image.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CFont::CreateCel(SGlyphDescription* psDescription)
{
	Ptr<CImageCel>	pCel;

	pCel = OMalloc<CImageCel>(mpImage, 0, 0, 0, 0);
	pCel->SetSubImage(&psDescription->cSubImage);
	return pCel;
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
size CFont::NumGlyphs(void) { return mpDescription->NumGlyphs(); }

