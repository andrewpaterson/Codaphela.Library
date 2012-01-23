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
#include "Image.h"
#include "Glyph.h"


#define FIRST_LETTER	32
#define NUM_LETTERS		96


class CFont : public CUnknown
{
BASE_FUNCTIONS(CFont);
protected:
	CChars			mszName;
	CImage			mcImage;
	int				miAverageWidth;
	int				miHeight;
	BOOL			mbFixedWidh;
	int				miSpace;
	CArrayGlyph		macGlyphs;
	int				miAscent;
	int				miDescent;

public:
	void		Init(char* szName, int iSpaceWidth, int iAscent, int iDescent);
	void 		Kill(void);
	void 		Done(void);
	BOOL		Is(char* szName);
	int 		Width(char* szText);
	int			Height(void);
	void		FreeImage(void);
	BOOL		IsWhiteSpace(char c);
	CGlyph*		GetGlyph(char c);
	CGlyph*		AddGlyph(CImageCel* pcCel, int iStep);
	CImage*		GetImage(void);
	int			GetSpace(void);
	int			GetAscent(void);
	int			GetDescent(void);
};


#endif //__FONT_H__

