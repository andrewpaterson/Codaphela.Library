/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ImageCopier.h"
#include "ImageAccessorCreator.h"
#include "Font.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFont::Init(char* szName, size iSpaceWidth, int iAscent, int iDescent)
{
	PreInit();

	mszName.Init(szName);
	pcImage = NULL;
	miAverageWidth = 0;
	miHeight = iAscent + iDescent;
	mbFixedWidh = false;
	miSpace = iSpaceWidth;
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
	M_Pointer(pcImage);
	U_Size(miAverageWidth);
	U_Size(miHeight);
	U_Bool(mbFixedWidh);
	U_Size(miSpace);
	U_Unknown(CArrayGlyph, macGlyphs);
	U_SInt(miAscent);
	U_SInt(miDescent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Free(void)
{
	macGlyphs.Kill();
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
	size		i;
	CGlyph*		pcGlyph;
	size		iTotalWidth;
	size		iLastWidth;
	size		iWidth;

	iTotalWidth = 0;
	iWidth = 0;
	mbFixedWidh = true;
	for (i = 0; i < macGlyphs.NumElements(); i++)
	{
		pcGlyph = macGlyphs.Get(i);
		iLastWidth = iWidth;
		iWidth = pcGlyph->GetFullWidth();
		iTotalWidth += pcGlyph->GetFullWidth();
		if ((iLastWidth != iWidth) && (i > 1))
		{
			mbFixedWidh = false;
		}
		pcGlyph->GetSubImage()->SetHorizontalAlignment(SUB_IMAGE_ALIGNMENT_LEFT);
		pcGlyph->GetSubImage()->msAlignment.y = -miAscent;
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
size CFont::Width(char* szText)
{
	size	iLen;
	size	i;
	size	iChar;
	size	iWidth;

	iLen = (size)strlen(szText);
	iWidth = 0;
	for (i = 0; i < iLen; i++)
	{
		iChar = (size)szText[i];

		if ((iChar < FIRST_LETTER) || (iChar >= FIRST_LETTER + macGlyphs.NumElements()))
		{
			iWidth += 0;
		}
		else if (iChar == FIRST_LETTER)
		{
			iWidth += miSpace;
		}
		else
		{
			iWidth += macGlyphs.Get(iChar - FIRST_LETTER)->GetFullWidth();
		}
	}
	return iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::Height(void)
{
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFont::IsWhiteSpace(char c)
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
CGlyph* CFont::GetGlyph(char c)
{
	size	iChar;

	iChar = c;
	if ((iChar < FIRST_LETTER) || (iChar >= FIRST_LETTER + macGlyphs.NumElements()))
	{
		return NULL;
	}
	return macGlyphs.Get(iChar - FIRST_LETTER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGlyph* CFont::AddGlyph(CImageCel* pcCel, size iStep)
{
	CGlyph*		pcGlyph;

	pcGlyph = macGlyphs.Add();
	pcGlyph->Init(pcCel, iStep);
	return pcGlyph;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CFont::GetImage(void)
{
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::SetImage(Ptr<CImage> pcImage)
{
	this->pcImage = pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::GetSpace(void)
{
	return miSpace;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::GetAscent(void)
{
	return miAscent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFont::GetDescent(void)
{
	return miDescent;
}

