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
#ifndef __FONT_H__
#define __FONT_H__
#include "StandardLib/Object.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "Image.h"
#include "Glyph.h"


#define FIRST_LETTER	32
#define NUM_LETTERS		96


class CFont : public CObject
{
CONSTRUCTABLE(CFont);
DESTRUCTABLE(CFont);
protected:
	CCharsImmutable		mszName;
	Ptr<CImage>			pcImage;
	size				miAverageWidth;
	size				miHeight;
	bool				mbFixedWidh;
	int16				miSpaceWidth;
	CArrayGlyph			macGlyphs;
	int16				miAscent;
	int16				miDescent;
	int16				miTabSpaceCount;  //tab width = miTabSpaceCount * miSpaceWidth.

public:
	Ptr<CFont>		Init(char* szName, int16 iSpaceWidth, int16 iAscent, int16 iDescent);
	void			Class(void);
	void 			Free(void);

	bool			Save(CObjectWriter* pcFile);
	bool			Load(CObjectReader* pcFile);

	void 			Done(void);
	bool			Is(char* szName);
	size 			Width(char* szText);
	size			Height(void);
	bool			IsWhitespace(uint16 c);
	CGlyph*			GetGlyph(uint16 c);
	CGlyph*			AddGlyph(CImageCel* pcCel, int16 iStep);
	Ptr<CImage>		GetImage(void);
	void			SetImage(Ptr<CImage> pcImage);
	int16			GetSpace(void);
	int16			GetAscent(void);
	int16			GetDescent(void);
	int16			GetTabSpaceCount(void);
	int16			GetSpaceWidth(void);

};


#endif // __FONT_H__

