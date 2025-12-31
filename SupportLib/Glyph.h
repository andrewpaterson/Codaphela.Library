/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __GLYPH_H__
#define __GLYPH_H__
#include "ImageCel.h"


class CGlyph : public CUnknown
{
CONSTRUCTABLE(CGlyph);
protected:
	CImageCel*	mpcCel;
	int16		miStep;

public:
	void		Init(CImageCel* pcCel, int16 iStep);
	void		Kill(void);

	int			GetFullWidth(void);
	int			GetFullHeight(void);
	int			GetFullDestRight(int x);
	void		GetFullDestBounds(int x, int y, CRectangle* pcRectangle);
	void		GetImageDestBounds(int x, int y, CRectangle* pcRectangle);
	Ptr<CImage>	GetSourceImage(void);
	CSubImage*	GetSubImage(void);
	CImageCel*	GetCel(void);
	int16		GetStep(void);
};


typedef CArrayType<CGlyph> CArrayGlyph;


#endif // __GLYPH_H__

