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
	size						i;
	Ptr<CGlyph>					pGlyph;
	size						iTotalWidth;
	size						iLastWidth;
	size						iWidth;
	SIndexTreeMemoryIterator	sIter;

	iTotalWidth = 0;
	iWidth = 0;
	mbFixedWidh = true;

	i = 0;
	pGlyph = macGlyphs.StartIterationPointer(&sIter);
	while (pGlyph.IsNotNull())
	{
		iLastWidth = iWidth;
		iWidth = pGlyph->GetFullWidth();
		iTotalWidth += pGlyph->GetFullWidth();
		if ((iLastWidth != iWidth) && (i > 1))
		{
			mbFixedWidh = false;
		}
		pGlyph->GetSubImage()->SetHorizontalAlignment(SUB_IMAGE_ALIGNMENT_LEFT);
		pGlyph->GetSubImage()->msAlignment.y = -miAscent;

		i++;
		pGlyph = macGlyphs.IteratePointer(&sIter);
	}

	miAverageWidth = iTotalWidth / macGlyphs.NumElements();
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
Ptr<CGlyph> CFont::GetGlyph(CUnicodeReader* pcUTFReader)
{
	uint16			uiCodePoint16;
	uint32			uiCodePoint32;
	uint8			auiBuffer[64];
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
				return GetGlyph(uiCodePoint16, uiCodePointLength);
			}
			else
			{
				uiCodePoint32 = pcUTFReader->GetCodePointUint32();
				if (!pcUTFReader->IsError(uiCodePoint32))
				{
					uiCodePointLength = pcUTFReader->GetUnicodeCodePointLength(uiCodePoint32);
					return GetGlyph(uiCodePoint32, uiCodePointLength);
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
				return GetGlyph(uiCodePoint32, uiCodePointLength);
			}
		}
	}
	else
	{
		uiCodePointLength = pcUTFReader->GetCodePointMulti(auiBuffer, 64);
		if ((uiCodePointLength != 0) || (pcUTFReader->IsError(uiCodePointLength)))
		{
			return GetGlyph(auiBuffer, uiCodePointLength);
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::CalculateWidth(char* szText)
{
	CUTF8			cUTF8;
	size			uiUTFElementLength;
	size			ui;
	size			iWidth;
	Ptr<CGlyph>		pGlyph;

	ui = 0;
	iWidth = 0;
	cUTF8.Init(szText);
	uiUTFElementLength = cUTF8.PeekUTFBytes();
	while ((uiUTFElementLength != 0) && (uiUTFElementLength != cUTF8.GetError()))
	{
		pGlyph = GetGlyph(&cUTF8);

		if (pGlyph.IsNotNull())
		{
			iWidth += pGlyph->GetFullWidth();
			ui++;
		}
		uiUTFElementLength = cUTF8.PeekUTFBytes();
	}
	return iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::IsWhitespace(uint16 c)
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
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint16 c, size uiLength)
{
	return macGlyphs.Get((uint8*)&c, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint32 c, size uiLength)
{
	return macGlyphs.Get((uint8*)&c, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint8* puiBuffer, size uiLength)
{
	return macGlyphs.Get(puiBuffer, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::PutGlyph(uint8 c, Ptr<CImageCel> pCel, int16 iStep)
{
	Ptr<CGlyph>		pGlyph;

	pGlyph = OMalloc<CGlyph>(pCel, iStep);
	macGlyphs.Put((uint8*)&c, sizeof(uint8), pGlyph);
	return pGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::PutGlyph(uint16 c, Ptr<CImageCel> pCel, int16 iStep)
{
	Ptr<CGlyph>		pGlyph;

	pGlyph = OMalloc<CGlyph>(pCel, iStep);
	macGlyphs.Put((uint8*)&c, sizeof(uint16), pGlyph);
	return pGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::PutGlyph(uint32 c, size uiLength, Ptr<CImageCel> pCel, int16 iStep)
{
	Ptr<CGlyph>		pGlyph;

	pGlyph = OMalloc<CGlyph>(pCel, iStep);
	macGlyphs.Put((uint8*)&c, uiLength, pGlyph);
	return pGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::PutGlyph(uint8* puiBuffer, size uiLength, Ptr<CImageCel> pCel, int16 iStep)
{
	Ptr<CGlyph>		pGlyph;

	pGlyph = OMalloc<CGlyph>(pCel, iStep);
	macGlyphs.Put(puiBuffer, uiLength, pGlyph);
	return pGlyph;
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
	return macGlyphs.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	bool	bExists;

	macGlyphs.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, &bExists);

	return bExists;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize)
{
	bool	bExists;

	macGlyphs.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, &bExists);

	return bExists;
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

