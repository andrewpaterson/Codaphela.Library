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
#include "ImageBlitterFormat.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"
#include "ImageBlitterContext.h"
#include "ImageRowBlitter.h"


class CImageRowBlitterCache;
class CImageBlitter : public CObject
{
CONSTRUCTABLE(CImageBlitter);
DESTRUCTABLE(CImageBlitter);
protected:
	CArrayImageRowBlitter		macRowBlitters;

	Ptr<CImageCel>				mpSourceCel;
	Ptr<CImage>					mpDestImage;

	CImageBlitterFormat			mcFormat;
	CImageBlitterContext		mcContext;

public:
	bool			Init(Ptr<CImageCel> pSource, Ptr<CImage> pcDest, CImageRowBlitterCache* pcBlitterCache);
	void			Free(void);

	void			Class(void);
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			Copy(int32 iDestX, int32 iDestY);

protected:
	EColourOrder	GetColourOrder(Ptr<CImage> pImage);
	EColourFormat	GetColourFormat(Ptr<CImage> pImage);
	ERGBColourBits	GetColourBits(Ptr<CImage> pImage);
	ERGBAlphaBits	GetAlphaBits(Ptr<CImage> pImage);

	bool			InitColourInfo(CImageBlitterFormat* pcFormat);
	bool			InitOpacityInfo(CImageBlitterFormat* pcFormat);
	bool			InitRowBlitters(CImageBlitterFormat* pcFormat, CImageRowBlitterCache* pcBlitterCache);
	void			InitContext(CImageBlitterContext* pcContext);
	bool			CreateImageRowBlitterContiguous(CImageRowBlitterCache* pcBlitterCache);
	bool			CreateImageRowBlitterByteAlignedOpaque(CImageBlitterFormat* pcFormat, CImageRowBlitterCache* pcBlitterCache);
	bool			CreateImageRowBlitterRGBByteAlphaByteTranslucent(CImageBlitterFormat* pcFormat, CImageRowBlitterCache* pcBlitterCache);

	void			AddBlitter(CBaseImageRowBlitter* pcBlitter, size xStart, size xEnd, size yOffset);
};


#endif // __IMAGE_BLITTER_H__

