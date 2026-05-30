#ifndef __IMAGE_ROW_BLITTER_CACHE_H__
#define __IMAGE_ROW_BLITTER_CACHE_H__
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
#include "StandardLib/MapObject.h"
#include "StandardLib/ArrayObject.h"
#include "ColourFormat.h"
#include "ImageCel.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"
#include "ImageRowBlitterCacheValue.h"
#include "ImageRowBlitterContiguous.h"
#include "ImageRowBlitterByteAlignedOpaque.h"
#include "ImageRowBlitterRGBByteAlphaByteTranslucent.h"


class CImageRowBlitterCache : public CObject
{
CONSTRUCTABLE(CImageRowBlitterCache);
DESTRUCTABLE(CImageRowBlitterCache);
protected:
	CMapObject	mMapBlitters;

public:
	void						Init(void);
	void						Class(void);
	void						Free(void);

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	Ptr<CBaseImageRowBlitter>	CreateImageRowBlitterContiguous(Ptr<CImage> pSource, Ptr<CImage> pDest);
	Ptr<CBaseImageRowBlitter>	CreateImageRowBlitterByteAlignedOpaque(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper);
	Ptr<CBaseImageRowBlitter>	CreateImageRowBlitterRGBByteAlphaByteTranslucent(Ptr<CImage> pSource, Ptr<CImage> pDest, CColourFormatHelper* pcSourceFormatHelper, CColourFormatHelper* pcDestFormatHelper);

	bool						FreeImageRowBlitter(Ptr<CBaseImageRowBlitter> pcRowBlitter);

protected:
	Ptr<CImageRowBlitterCacheValue>		GetOrCreateImageRowBlitterCacheValue(Ptr<CImage> pSource);
};


#endif // __IMAGE_ROW_BLITTER_CACHE_H__

