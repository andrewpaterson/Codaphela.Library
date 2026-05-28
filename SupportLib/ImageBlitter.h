#ifndef __IMAGE_BLITTER_H__
#define __IMAGE_BLITTER_H__
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
#include "StandardLib/Pointer.h"
#include "ColourFormat.h"
#include "ImageCel.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"
#include "ImageRowBlitters.h"


class CImageRowBlitterCache;
class CImageBlitter : public CUnknown
{
CONSTRUCTABLE(CImageBlitter);
protected:
	EColourOrder			meColourOrder;
	EColourFormat			meSourceColourFormat;
	EColourFormat			meDestColourFormat;
	ERGBColourBits			meColourBits;
	ERGBAlphaBits			meSourceAlphaBits;
	ERGBAlphaBits			meDestAlphaBits;

	CImageRowBlitterCache*	mpcBlitterCache;
	CImageRowBlitters		mcBlitters;
	EColourOpacity			meSourceOpacity;

public:
	bool			Init(Ptr<CImageCel> pSource, Ptr<CImage> pcDest, Ptr<CImageRowBlitterCache> pBlitterCache);
	bool			InitColourInfo(Ptr<CImage> pSource, Ptr<CImage> pDest);
	bool			InitOpacityInfo(Ptr<CImageCel> pSourceCel, Ptr<CImage> pDest);
	void			Kill(void);

	EColourOrder	GetColourOrder(Ptr<CImage> pImage);
	EColourFormat	GetColourFormat(Ptr<CImage> pImage);
	ERGBColourBits	GetColourBits(Ptr<CImage> pImage);
	ERGBAlphaBits	GetAlphaBits(Ptr<CImage> pImage);

	void			Copy(int32 iDestX, int32 iDestY);
	void			Copy(SImageCopy* psCopy);
};


#endif // __IMAGE_BLITTER_H__

