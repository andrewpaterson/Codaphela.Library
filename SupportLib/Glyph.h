#ifndef __GLYPH_H__
#define __GLYPH_H__
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
#include "BaseLib/IndexTemplate.h"
#include "ImageCel.h"
#include "StandardLib/Array.h"
#include "StandardLib/Index.h"


class CGlyph : public CObject
{
CONSTRUCTABLE(CGlyph);
DESTRUCTABLE(CGlyph);
protected:
	Ptr<CImageCel>	mpCel;
	int16			miStep;

public:
	void			Init(Ptr<CImageCel> pCel, int16 iStep);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	int				GetFullWidth(void);
	int				GetFullHeight(void);
	int				GetFullDestRight(int x);
	void			GetFullDestBounds(int x, int y, CRectangle* pcRectangle);
	void			GetImageDestBounds(int x, int y, CRectangle* pcRectangle);
	Ptr<CImage>		GetSourceImage(void);
	CSubImage*		GetSubImage(void);
	Ptr<CImageCel>	GetCel(void);
	int16			GetStep(void);

	void			Print(CChars* psz);
	void			Dump(void);
};


struct SGlyphPosition
{
	CGlyph*		pcGlyph;
	SInt32Vec2	sPosition;
};


//Where a glyph's pixels are in its font's image and how far it advances.  A font keeps one for every character it
//has and only makes the CGlyph (and its CImageCel) the first time the character is asked for.
struct SGlyphDescription
{
	CSubImage	cSubImage;
	int16		iStep;
};


typedef CIndex<CGlyph> CIndexGlyph;
typedef CIndexTemplate<SGlyphDescription> CIndexGlyphDescription;
typedef CArrayTemplate<SGlyphPosition>	CArrayGlyphPosition;


#endif // __GLYPH_H__

