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
#include "ImageCopier.h"
#include "ImageAccessorCreator.h"
#include "Font.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CFont::Init(char* szName, int iSpaceWidth, int iAscent, int iDescent)
{
	mszName.Init(szName);
	pcImage = NULL;
	miAverageWidth = 0;
	miHeight = iAscent + iDescent;
	mbFixedWidh = FALSE;
	miSpace = iSpaceWidth;
	miAscent = iAscent;
	miDescent = iDescent;
	macGlyphs.Init();

	return Ptr<CFont>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Class(void)
{
	CObject::Class();
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
BOOL CFont::Save(CObjectSerialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFont::Load(CObjectDeserialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFont::Done(void)
{
	int			i;
	CGlyph*		pcGlyph;
	int			iTotalWidth;
	int			iLastWidth;
	int			iWidth;

	iTotalWidth = 0;
	iWidth = 0;
	mbFixedWidh = TRUE;
	for (i = 0; i < macGlyphs.NumElements(); i++)
	{
		pcGlyph = macGlyphs.Get(i);
		iLastWidth = iWidth;
		iWidth = pcGlyph->GetFullWidth();
		iTotalWidth += pcGlyph->GetFullWidth();
		if ((iLastWidth != iWidth) && (i > 1))
		{
			mbFixedWidh = FALSE;
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
BOOL CFont::Is(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFont::Width(char* szText)
{
	int		iLen;
	int		i;
	int		iChar;
	int		iWidth;

	iLen = (int)strlen(szText);
	iWidth = 0;
	for (i = 0; i < iLen; i++)
	{
		iChar = (int)szText[i];

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
int CFont::Height(void)
{
	return miHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFont::IsWhiteSpace(char c)
{
	//  c <=  to include the space.
	if ((c <= FIRST_LETTER) || (c >= (FIRST_LETTER + macGlyphs.NumElements())))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGlyph* CFont::GetGlyph(char c)
{
	int	iChar;

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
CGlyph* CFont::AddGlyph(CImageCel* pcCel, int iStep)
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
int CFont::GetSpace(void)
{
	return miSpace;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFont::GetAscent(void)
{
	return miAscent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFont::GetDescent(void)
{
	return miDescent;
}
