/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "BaseLib/UTF-8.h"
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
	miHeight = iAscent + iDescent;
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
	U_Size(miHeight);
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
Ptr<CGlyph> CFont::GetGlyph(CUTF8* pcUTF8)
{
	uint16			c16;
	uint32			c32;
	uint8			auiBuffer[64];
	size			uiElementLength;

	uiElementLength = pcUTF8->Peek();
	if (uiElementLength <= 2)
	{
		c16 = pcUTF8->GetCodePointUint16();
		if ((c16 != 0xFFFD) && (c16 != 0xFFFF))
		{
			return GetGlyph(c16);
		}
	}
	else if (uiElementLength <= 4)
	{
		c32 = pcUTF8->GetCodePointUint32();
		if ((c32 != 0xFFFD) && (c32 != 0xFFFF))
		{
			return GetGlyph(c32, uiElementLength);
		}
	}
	else
	{
		uiElementLength = pcUTF8->GetCodePointMulti(auiBuffer, 64);
		if ((uiElementLength != 0) || (uiElementLength != pcUTF8->GetError()))
		{
			return GetGlyph(auiBuffer, uiElementLength);
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::Width(char* szText)
{
	CUTF8			cUTF8;
	size			uiUTFElementLength;
	size			ui;
	size			iWidth;
	Ptr<CGlyph>		pGlyph;

	ui = 0;
	iWidth = 0;
	cUTF8.Init(szText);
	uiUTFElementLength = cUTF8.Peek();
	while ((uiUTFElementLength != 0) && (uiUTFElementLength != cUTF8.GetError()))
	{
		pGlyph = GetGlyph(&cUTF8);

		if (pGlyph.IsNotNull())
		{
			iWidth += pGlyph->GetFullWidth();
			ui++;
		}
		uiUTFElementLength = cUTF8.Peek();
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
Ptr<CGlyph> CFont::GetGlyph(uint16 c)
{
	return macGlyphs.Get((uint8*)&c, sizeof(uint16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGlyph> CFont::GetGlyph(uint8 c)
{
	return macGlyphs.Get((uint8*)&c, sizeof(uint8));
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
int16 CFont::GetTabSpaceCount(void) { return miTabSpaceCount; }
size CFont::Height(void) { return miHeight; }

