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
#include "BaseLib/ArrayInt32.h"
#include "StandardLib/Pointer.h"
#include "ColourFormat.h"
#include "ImageCel.h"
#include "ImageBlitterFormat.h"
#include "Rectangle.h"
#include "ImageCopyDimension.h"
#include "ImageBlitterContext.h"
#include "ImageRowBlitter.h"
#include "ImageRowBlitterFactory.h"


class CImageBlitter : public CObject
{
CONSTRUCTABLE(CImageBlitter);
DESTRUCTABLE(CImageBlitter);
protected:
	CArrayImageRowBlitter	macRowBlitters;

	CImageCel*				mpcSourceCel;		//Not a reference.  Whoever uses this blitter must keep the cel alive.
	Ptr<CImage>				mpDestImage;

	CImageBlitterFormat		mcFormat;
	CImageBlitterContext	mcContext;

	CArrayInt32				maiRowStarts;		//The index in macRowBlitters of the first segment of each row of the cel's image, then one past the last segment.

	//Built for the last size the cel was drawn at that was not its own.
	SInt32Vec2				msSizedTo;
	CArrayInt32				maiSizedSourceX;	//For each destination column the source image column drawn into it.
	CArrayInt32				maiSizedSourceY;	//For each destination row the row of the cel's image drawn into it, outside the image in the cel's transparent borders.
	CArrayInt32				maiSizedDestX;		//For each column of the full cel the first destination column drawn from it or a column to its right.

public:
	void			Init(Ptr<CImageCel> pSource, Ptr<CImage> pcDest);
	bool			Configure(CImageRowBlitterFactory* pcBlitterCache);
	void			Free(void);
	void			Class(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			Blit(int32 iDestX, int32 iDestY, CRectangle* pcClip = NULL);
	bool			Blit(int32 iDestX, int32 iDestY, int32 iDestWidth, int32 iDestHeight, CRectangle* pcClip = NULL);
	Ptr<CImageCel>	GetCel(void);

protected:
	bool			PopulateColourFormat(CImageBlitterFormat* pcFormat);
	bool			PopulateOpacityFormat(CImageBlitterFormat* pcFormat);
	bool			InitRowBlitters(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);
	void			InitContext(CImageBlitterContext* pcContext);
	void			InitRowStarts(void);
	bool			InitSizedTables(int32 iDestWidth, int32 iDestHeight);
	void			InitSizedSamples(CArrayInt32* paiSamples, int32 iDestLength, int32 iFullLength, int32 iOffset);
	bool			ClipDestRect(CRectangle* pcClip, CRectangle* pcDestRect);

	bool			CreateImageRowBlitterContiguous(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);
	bool			CreateImageRowBlitterByteAlignedOpaque(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);
	bool			CreateImageRowBlitterByteAlignedOpaqueDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);
	bool			CreateImageRowBlitterRGBByteAlphaByteTranslucent(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);
	bool			CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache);

	bool			CreateImageRowBlitterUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreate);
	bool			CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreateRGBByteAlphaByteTranslucent, CreateImageRowBlitterFunc fCreateByteAlignedOpaque);

	void			AddBlitter(CBaseImageRowBlitter* pcBlitter, CImageRowBlitterFactory* pcBlitterFactory, size xStart, size xEnd, size yOffset);
};


typedef CArray<CImageBlitter> CArrayImageBlitter;


#endif // __IMAGE_BLITTER_H__

