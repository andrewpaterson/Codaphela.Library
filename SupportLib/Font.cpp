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
#include "BaseLib/UTF8.h"
#include "ImageCopier.h"
#include "ImageAccessorCreator.h"
#include "Font.h"


#define FONT_MAX_KEY_LENGTH		64


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFont::Init(char* szName, int16 iSpaceWidth, int16 iAscent, int16 iDescent)
{
	PreInit();

	mszName.Init(szName);
	mpImage = NULL;
	miAverageWidth = 0;
	miTabSpaceCount = 4;
	miAverageHeight = iAscent + iDescent;
	miLineHeight = iAscent + iDescent;
	mbFixedWidh = false;
	miSpaceWidth = iSpaceWidth;
	miAscent = iAscent;
	miDescent = iDescent;
	macGlyphs.Init();
	macDescriptions.Init();

	PostInit();

	return Ptr<CFont>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Class(void)
{
	U_String(mszName);
	M_Pointer(mpImage);
	U_Size(miAverageWidth);
	U_Size(miAverageHeight);
	U_Size(miLineHeight);
	U_Bool(mbFixedWidh);
	U_Int16(miSpaceWidth);
	M_Embedded(macGlyphs);
	U_Data(CIndexGlyphDescription, macDescriptions);
	U_Int16(miAscent);
	U_Int16(miDescent);
	U_Int16(miTabSpaceCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Free(void)
{
	macDescriptions.Kill();
	mszName.Kill();
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
void CFont::Done(void)
{
	size							i;
	SGlyphDescription*				psDescription;
	size							iTotalWidth;
	size							iLastWidth;
	size							iWidth;
	SIndexTreeMemoryUnsafeIterator	sIter;
	uint8							auiKey[FONT_MAX_KEY_LENGTH];
	size							uiKeyLength;
	bool							bExists;

	iTotalWidth = 0;
	iWidth = 0;
	mbFixedWidh = true;

	//Glyphs copy their description when they are generated so only glyphs generated after Done are aligned.
	i = 0;
	bExists = macDescriptions.StartIteration(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH, &psDescription);
	while (bExists)
	{
		iLastWidth = iWidth;
		iWidth = psDescription->cSubImage.GetFullWidth();
		iTotalWidth += iWidth;
		if ((iLastWidth != iWidth) && (i > 1))
		{
			mbFixedWidh = false;
		}
		psDescription->cSubImage.SetHorizontalAlignment(SUB_IMAGE_ALIGNMENT_LEFT);
		psDescription->cSubImage.msAlignment.y = -miAscent;

		i++;
		bExists = macDescriptions.Iterate(&sIter, auiKey, &uiKeyLength, FONT_MAX_KEY_LENGTH, &psDescription);
	}

	miAverageWidth = iTotalWidth / macDescriptions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFont::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(CUnicodeReader* pcUTFReader)
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
size CFont::ReadKey(CUnicodeReader* pcUTFReader, uint8* puiKey, size uiMaxKeyLength)
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
size CFont::CalculateWidth(char* szText)
{
	CUTF8				cUTF8;
	size				uiUTFElementLength;
	size				iWidth;
	SGlyphDescription*	psDescription;

	//Measured from the descriptions so no glyphs are generated.
	iWidth = 0;
	cUTF8.Init(szText);
	uiUTFElementLength = cUTF8.PeekUTFBytes();
	while ((uiUTFElementLength != 0) && (uiUTFElementLength != cUTF8.GetError()))
	{
		psDescription = GetGlyphDescription(&cUTF8);
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
bool CFont::IsWhitespace(uint16 c)
{
	//  c <=  to include the space.
	if ((c <= FIRST_LETTER) || (c >= (char)(FIRST_LETTER + macDescriptions.NumElements())))
	{
		return true;
	}
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
// The glyph is generated from its description the first time it is
// asked for.  NULL if the font doesn't have the character.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint8* puiBuffer, size uiLength)
{
	Ptr<CGlyph>			pGlyph;
	SGlyphDescription*	psDescription;

	pGlyph = macGlyphs.Get(puiBuffer, uiLength);
	if (pGlyph.IsNull())
	{
		psDescription = GetGlyphDescription(puiBuffer, uiLength);
		if (psDescription != NULL)
		{
			pGlyph = OMalloc<CGlyph>(CreateCel(psDescription), psDescription->iStep);
			macGlyphs.Put(puiBuffer, uiLength, pGlyph);
		}
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
	return macDescriptions.Get(puiBuffer, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next character.  NULL if the font doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
SGlyphDescription* CFont::GetGlyphDescription(CUnicodeReader* pcUTFReader)
{
	uint8	auiKey[FONT_MAX_KEY_LENGTH];
	size	uiKeyLength;

	uiKeyLength = ReadKey(pcUTFReader, auiKey, FONT_MAX_KEY_LENGTH);
	if (uiKeyLength == 0)
	{
		return NULL;
	}
	return GetGlyphDescription(auiKey, uiKeyLength);
}


//////////////////////////////////////////////////////////////////////////
//
// A new cel over the description's pixels in the font's image.
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
//
//////////////////////////////////////////////////////////////////////////
void CFont::PutGlyph(uint32 c, size uiLength, CSubImage* pcSubImage, int16 iStep)
{
	PutGlyph((uint8*)&c, uiLength, pcSubImage, iStep);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::PutGlyph(uint8* puiBuffer, size uiLength, CSubImage* pcSubImage, int16 iStep)
{
	SGlyphDescription*	psDescription;

	psDescription = macDescriptions.Put(puiBuffer, uiLength);
	psDescription->cSubImage.Copy(pcSubImage);
	psDescription->iStep = iStep;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::SetImage(Ptr<CImage> pImage)
{
	mpImage = pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::NumGlyphs(void)
{
	return macDescriptions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The number of glyphs asked for so far.
//
//////////////////////////////////////////////////////////////////////////
size CFont::NumGeneratedGlyphs(void)
{
	return macGlyphs.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	//The keys of every character, whether its glyph has been generated or not.
	return macDescriptions.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	return macDescriptions.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CFont::GetImage(void) { return mpImage; }
int16 CFont::GetSpaceWidth(void) { return miSpaceWidth; }
int16 CFont::GetAscent(void) { return miAscent; }
int16 CFont::GetDescent(void) { return miDescent; }
int16 CFont::GetLineHeight(void) { return miAscent + miDescent; }
int16 CFont::GetTabSpaceCount(void) { return miTabSpaceCount; }
size CFont::GetAverageWidth(void) { return miAverageWidth; }
size CFont::GetAverageHeight(void) { return miAverageHeight; }

