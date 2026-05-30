#ifndef __BASE_IMAGE_ROW_BLITTER_H__
#define __BASE_IMAGE_ROW_BLITTER_H__
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
#include "ImageCel.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"


class CBaseImageRowBlitter : public CObject
{
CONSTRUCTABLE(CBaseImageRowBlitter);
protected:
	Ptr<CImage>			mpSource;
	Ptr<CImage>			mpDest;

	size				miSourcePixelStride;
	size				miDestPixelStride;
	size				miSourceWidth;
	size				miDestWidth;
	size				miSourceRowStride;
	size				miDestRowStride;

	size				muiUsageCount;

public:
			void			Init(Ptr<CImage> pSource, Ptr<CImage> pDest);
			void			Free(void) override;

			void			Class(void) override;
			bool			Save(CObjectWriter* pcFile) override;
			bool			Load(CObjectReader* pcFile) override;

	virtual void			Copy(int32 iDestX, int32 iDestY, int32 iSourceXLeft, int32 iSourceXRight, int32 iSourceY) =0;

			Ptr<CImage>		GetSource(void);
			Ptr<CImage>		GetDest(void);

			bool			Is(Ptr<CImage> pDestImage, const char* szBlitterClass);

			size			AddUsage(void);
			size			RemoveUsage(void);

};


#endif // __BASE_IMAGE_ROW_BLITTER_H__

