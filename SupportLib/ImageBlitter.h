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
#include "Int32Rect.h"
#include "ImageCopyDimension.h"
#include "ImageBlitterContext.h"
#include "OffsetImageRowBlitter.h"
#include "SizingImageBlitterSize.h"
#include "ImageRowBlitterFactory.h"


class CImageBlitter : public CObject
{
CONSTRUCTABLE(CImageBlitter);
DESTRUCTABLE(CImageBlitter);
protected:
	CArrayOffsetImageRowBlitter		macRowBlitters;

	CImageCel*						mpcSourceCel;		//Not a reference.  Whoever uses this blitter must keep the cel alive.
	Ptr<CImage>						mpDestImage;

	CImageBlitterFormat				mcFormat;
	CImageBlitterContext			mcContext;

	CArrayInt32						maiRowStarts;		//The index in macRowBlitters of the first segment of each row of the cel's image, then one past the last segment.

	CSizingImageBlitterSize*		mpcSizing;

public:
	void			Init(Ptr<CImageCel> pSource, Ptr<CImage> pcDest);
	bool			Configure(CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	void			Free(void);
	void			Class(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			Blit(int32 iDestX, int32 iDestY, CInt32Rect* pcClip = NULL);
	bool			Blit(int32 iDestX, int32 iDestY, int32 iDestWidth, int32 iDestHeight, CInt32Rect* pcClip = NULL);
	Ptr<CImageCel>	GetCel(void);
	bool			AllowsSizing(void);
	void			EnableSizing(CImageRowBlitterFactory* pcBlitterCache);

protected:
	bool			PopulateColourFormat(CImageBlitterFormat* pcFormat);
	bool			PopulateOpacityFormat(CImageBlitterFormat* pcFormat);
	bool			CreateImageRowBlitters(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	void			PopulateContext(CImageBlitterContext* pcContext);
	void			PopulateRowStarts(void);
	bool			PopulateSizingTables(int32 iDestWidth, int32 iDestHeight);
	bool			ClipDestRect(CInt32Rect* pcClip, CInt32Rect* pcDestRect);

	bool			CreateImageRowBlitterContiguous(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	bool			CreateImageRowBlitterByteAlignedOpaque(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	bool			CreateImageRowBlitterByteAlignedOpaqueDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	bool			CreateImageRowBlitterRGBByteAlphaByteTranslucent(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);
	bool			CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, bool bAllowSizing);

	bool			CreateImageRowBlitterUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreate, bool bAllowSizing);
	bool			CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreateRGBByteAlphaByteTranslucent, CreateImageRowBlitterFunc fCreateByteAlignedOpaque, bool bAllowSizing);

	void			AddBlitter(CBaseImageRowBlitter* pcBlitter, CImageRowBlitterFactory* pcBlitterFactory, size xStart, size xEnd, size yOffset, bool bAllowSizing);
};


typedef CArray<CImageBlitter> CArrayImageBlitter;


#endif // __IMAGE_BLITTER_H__

