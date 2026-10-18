#ifndef __IMAGE_ROW_BLITTER_RGB_BYTE_ALPHA_BYTE_TRANSLUCENT_H__
#define __IMAGE_ROW_BLITTER_RGB_BYTE_ALPHA_BYTE_TRANSLUCENT_H__
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
#include "BaseImageRowBlitter.h"


//Blends the colour of one source pixel over one destination pixel by the opacity of the source pixel.  The destination is
//treated as opaque, which is only exact when it is.
void	BlendRGBByteAlphaByte(CImageBlitterContext* pcContext, void* pvSourcePixel, void* pvDestPixel);

//Combines the opacity of one source pixel with the opacity of the destination pixel it is drawn over.
void	BlendAlphaByte(CImageBlitterContext* pcContext, void* pvSourcePixel, void* pvDestPixel);


class CImageRowBlitterRGBByteAlphaByteTranslucent : public CBaseImageRowBlitter
{
public:
	void							Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY) override;
	CBaseImageRowSizingBlitter*		CreateRowSizingBlitter(CImageRowBlitterFactory* pcFactory) override;
};


class CImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha : public CBaseImageRowBlitter
{
public:
	void							Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestX, size iDestY, size iSourceXLeft, size iSourceXRight, size iSourceY) override;
	CBaseImageRowSizingBlitter*		CreateRowSizingBlitter(CImageRowBlitterFactory* pcFactory) override;
};


#endif // __IMAGE_ROW_BLITTER_RGB_BYTE_ALPHA_BYTE_TRANSLUCENT_H__

