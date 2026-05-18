#ifndef __IMAGE_COPIER_H__
#define __IMAGE_COPIER_H__
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
#include "ImageAccessor.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"


class CImageCel;
class CImageCopier : public CUnknown
{
CONSTRUCTABLE(CImageCopier);
public:
	static void Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest, int32 iDestX, int32 iDestY, CRectangle* psSourceRect = NULL);
	static void Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest, int32 iDestX, int32 iDestY, int32 iSourceX1, int32 iSourceY1, int32 iSourceX2, int32 iSourceY2);
	static void Copy(Ptr<CImageCel> pcSource, Ptr<CImage> pcDest, int32 iDestX, int32 iDestY);
	static void Copy(Ptr<CImage> pcSource, Ptr<CImage> pcDest);

protected:
	Ptr<CImage>			mpcSource;
	Ptr<CImage>			mpcDest;
	CImageAccessor*		mpcSourceAccessor;
	CImageAccessor*		mpcDestAccessor;
	bool				mbKillAccessors;

public:
	bool Init(Ptr<CImage> pcSource, Ptr<CImage> pcDest);
	void Init(CImageAccessor* pcSourceAccessor, CImageAccessor* pcDestAccessor);
	void Kill(void);

	void Copy(int32 iDestX, int32 iDestY, Ptr<CImageCel> pcSourceCel);
	void Copy(int32 iDestX, int32 iDestY, CRectangle* psSourceRect = NULL);
	void Copy(int32 iDestX, int32 iDestY, int32 iSourceX1, int32 iSourceY1, int32 iSourceX2, int32 iSourceY2);
	void Copy(SImageCopy* psCopy);
};


#endif // __IMAGE_COPIER_H__

