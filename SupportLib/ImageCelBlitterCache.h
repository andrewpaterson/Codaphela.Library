#ifndef __IMAGE_CEL_BLITTER_CACHE_H__
#define __IMAGE_CEL_BLITTER_CACHE_H__
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
#include "StandardLib/Array.h"
#include "ImageCel.h"
#include "ImageBlitter.h"
#include "ImageRowBlitterFactory.h"


typedef CArrayTemplate<CImageCel*>	CArrayImageCelPtr;


//The blitters that draw onto one destination image.  A blitter does not keep its cel alive and neither does the cache.
//When a cel is freed it tells every cache holding a blitter for it to forget it, and when a cache is freed it tells
//every cel it holds a blitter for to forget it, so whichever of the two is freed first the other is never left
//pointing at it.  A forgotten blitter is only removed the next time the cache is used, never while something is being freed.
class CImageCelBlitterCache : public CObject
{
CONSTRUCTABLE(CImageCelBlitterCache);
DESTRUCTABLE(CImageCelBlitterCache);
protected:
	CArray<CImageBlitter>		maBlitters;  //The blitter for the cel at the same index in mapcCels.
	CArrayImageCelPtr			mapcCels;    //Not references.  NULL where the cel has been freed.
	Ptr<CImage>					mpDestImage;

public:
	void				Init(Ptr<CImage> pDestImage);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CImageBlitter>	CreateImageBlitter(Ptr<CImageCel> pCel);
	Ptr<CImageBlitter>	CreateSizingImageBlitter(Ptr<CImageCel> pCel);
	Ptr<CImage>			GetDestImage(void);

	Ptr<CImageBlitter>	GetImageBlitter(Ptr<CImageCel> pCel);
	bool				HasImageBlitter(Ptr<CImageCel> pCel);
	size				NumImageBlitters(void);

	bool				Matches(Ptr<CImage> pImage);

	void				RemoveCelDuringFree(CImageCel* pcCel);  //Only called by CImageCel::Free.

protected:
	size				FindCel(CImageCel* pcCel);
	void				RemoveForgottenBlitters(void);
	Ptr<CImageBlitter>	CreateImageBlitter(Ptr<CImageCel> pCel, bool bAllowSizing);
};


#endif // __IMAGE_CEL_BLITTER_CACHE_H__

