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
#include "FontDescription.h"


//////////////////////////////////////////////////////////////////////////
//
// iImageWidth and iImageHeight are the size of the image the glyphs are
// in.  Every font made from this description needs an image at least that
// big, laid out the same way.
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::Init(char* szName, int16 iSpaceWidth, int16 iAscent, int16 iDescent, int32 iImageWidth, int32 iImageHeight)
{
	PreInit();

	mszName.Init(szName);
	macGlyphs.Init();
	miImageWidth = iImageWidth;
	miImageHeight = iImageHeight;
	miAverageWidth = 0;
	miAverageHeight = iAscent + iDescent;
	mbFixedWidth = false;
	mbWidthsCalculated = false;
	miSpaceWidth = iSpaceWidth;
	miAscent = iAscent;
	miDescent = iDescent;
	miTabSpaceCount = 4;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::Class(void)
{
	U_String(mszName);
	U_Data(CIndexGlyphDescription, macGlyphs);
	U_Int32(miImageWidth);
	U_Int32(miImageHeight);
	U_Size(miAverageWidth);
	U_Size(miAverageHeight);
	U_Bool(mbFixedWidth);
	U_Bool(mbWidthsCalculated);
	U_Int16(miSpaceWidth);
	U_Int16(miAscent);
	U_Int16(miDescent);
	U_Int16(miTabSpaceCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::Free(void)
{
	macGlyphs.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Works out the average full width of the glyphs and whether they are all
// the same width.  Done when they are first asked for after glyphs are put.
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::CalculateWidths(void)
{
	SGlyphDescription*				psDescription;
	size							iTotalWidth;
	int32							iWidth;
	int32							iFirstWidth;
	SIndexTreeMemoryUnsafeIterator	sIter;
	uint8							auiKey[FONT_MAX_KEY_LENGTH];
	size							uiKeyLength;
	bool							bExists;

	iTotalWidth = 0;
	iFirstWidth = -1;
	mbFixedWidth = macGlyphs.NumElements() > 0;

	//The unsafe iterator gives the description itself; the safe one doesn't work for data that isn't a pointer.
	bExists = macGlyphs.StartIteration(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH, &psDescription);
	while (bExists)
	{
		iWidth = psDescription->cSubImage.GetFullWidth();
		iTotalWidth += iWidth;
		if (iFirstWidth == -1)
		{
			iFirstWidth = iWidth;
		}
		else if (iWidth != iFirstWidth)
		{
			mbFixedWidth = false;
		}
		bExists = macGlyphs.Iterate(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH, &psDescription);
	}

	if (macGlyphs.NumElements() > 0)
	{
		miAverageWidth = iTotalWidth / macGlyphs.NumElements();
	}
	else
	{
		miAverageWidth = 0;
	}
	mbWidthsCalculated = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::PutGlyph(uint32 c, size uiLength, CSubImage* pcSubImage, int16 iStep)
{
	PutGlyph((uint8*)&c, uiLength, pcSubImage, iStep);
}


//////////////////////////////////////////////////////////////////////////
//
// A character put again keeps its index.
//
//////////////////////////////////////////////////////////////////////////
void CFontDescription::PutGlyph(uint8* puiBuffer, size uiLength, CSubImage* pcSubImage, int16 iStep)
{
	SGlyphDescription*	psDescription;

	psDescription = macGlyphs.Get(puiBuffer, uiLength);
	if (psDescription == NULL)
	{
		psDescription = macGlyphs.Put(puiBuffer, uiLength);
		psDescription->uiIndex = macGlyphs.NumElements() - 1;
	}
	psDescription->cSubImage.Copy(pcSubImage);
	psDescription->iStep = iStep;
	mbWidthsCalculated = false;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the font doesn't have the character.
//
//////////////////////////////////////////////////////////////////////////
SGlyphDescription* CFontDescription::GetGlyph(uint8* puiBuffer, size uiLength)
{
	return macGlyphs.Get(puiBuffer, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next character.  NULL if the font doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
SGlyphDescription* CFontDescription::GetGlyph(CUnicodeReader* pcUTFReader)
{
	uint8	auiKey[FONT_MAX_KEY_LENGTH];
	size	uiKeyLength;

	uiKeyLength = ReadKey(pcUTFReader, auiKey, FONT_MAX_KEY_LENGTH);
	if (uiKeyLength == 0)
	{
		return NULL;
	}
	return GetGlyph(auiKey, uiKeyLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next character and writes the key its glyph is indexed by
// into puiKey.  Returns the length of the key or 0 if there is no
// character.
//
//////////////////////////////////////////////////////////////////////////
size CFontDescription::ReadKey(CUnicodeReader* pcUTFReader, uint8* puiKey, size uiMaxKeyLength)
{
	uint16			uiCodePoint16;
	uint32			uiCodePoint32;
	size			uiUTFLength;
	size			uiCodePointLength;

	uiUTFLength = pcUTFReader->PeekUTFBytes();
	if (uiUTFLength <= 2)
	{
		uiCodePoint16 = pcUTFReader->GetCodePointUint16();
		if (uiCodePoint16 != 0)
		{
			if (!pcUTFReader->IsTooSmall(uiCodePoint16))
			{
				uiCodePointLength = pcUTFReader->GetUnicodeCodePointLength(uiCodePoint16);
				memcpy(puiKey, &uiCodePoint16, uiCodePointLength);
				return uiCodePointLength;
			}
			else
			{
				uiCodePoint32 = pcUTFReader->GetCodePointUint32();
				if (!pcUTFReader->IsError(uiCodePoint32))
				{
					uiCodePointLength = pcUTFReader->GetUnicodeCodePointLength(uiCodePoint32);
					memcpy(puiKey, &uiCodePoint32, uiCodePointLength);
					return uiCodePointLength;
				}
			}
		}
	}
	else if (uiUTFLength <= 4)
	{
		uiCodePoint32 = pcUTFReader->GetCodePointUint32();
		if (!pcUTFReader->IsError(uiCodePoint32))
		{
			if ((uiCodePoint32 != 0xFFFD) && (uiCodePoint32 != 0xFFFF))
			{
				uiCodePointLength = pcUTFReader->GetUnicodeCodePointLength(uiCodePoint32);
				memcpy(puiKey, &uiCodePoint32, uiCodePointLength);
				return uiCodePointLength;
			}
		}
	}
	else
	{
		uiCodePointLength = pcUTFReader->GetCodePointMulti(puiKey, uiMaxKeyLength);
		if ((uiCodePointLength != 0) || (pcUTFReader->IsError((uint32)uiCodePointLength)))
		{
			return uiCodePointLength;
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFontDescription::CalculateWidth(char* szText)
{
	CUTF8				cUTF8;
	size				uiUTFElementLength;
	size				iWidth;
	SGlyphDescription*	psDescription;

	iWidth = 0;
	cUTF8.Init(szText);
	uiUTFElementLength = cUTF8.PeekUTFBytes();
	while ((uiUTFElementLength != 0) && (uiUTFElementLength != cUTF8.GetError()))
	{
		psDescription = GetGlyph(&cUTF8);
		if (psDescription != NULL)
		{
			iWidth += psDescription->cSubImage.GetFullWidth();
		}
		uiUTFElementLength = cUTF8.PeekUTFBytes();
	}
	cUTF8.Kill();
	return iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::IsWhitespace(uint16 c)
{
	//  c <=  to include the space.
	if ((c <= FIRST_LETTER) || (c >= (char)(FIRST_LETTER + macGlyphs.NumElements())))
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The keys of every character in the font.
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	return macGlyphs.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	return macGlyphs.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFontDescription::NumGlyphs(void) { return macGlyphs.NumElements(); }
char* CFontDescription::GetFontName(void) { return mszName.Text(); }
int32 CFontDescription::GetImageWidth(void) { return miImageWidth; }
int32 CFontDescription::GetImageHeight(void) { return miImageHeight; }
int16 CFontDescription::GetSpaceWidth(void) { return miSpaceWidth; }
int16 CFontDescription::GetAscent(void) { return miAscent; }
int16 CFontDescription::GetDescent(void) { return miDescent; }
int16 CFontDescription::GetLineHeight(void) { return miAscent + miDescent; }
int16 CFontDescription::GetTabSpaceCount(void) { return miTabSpaceCount; }
size CFontDescription::GetAverageHeight(void) { return miAverageHeight; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFontDescription::GetAverageWidth(void)
{
	if (!mbWidthsCalculated)
	{
		CalculateWidths();
	}
	return miAverageWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// True if every glyph's full width (its advance) is the same.
//
//////////////////////////////////////////////////////////////////////////
bool CFontDescription::IsFixedWidth(void)
{
	if (!mbWidthsCalculated)
	{
		CalculateWidths();
	}
	return mbFixedWidth;
}

