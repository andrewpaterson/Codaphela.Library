#ifndef __IMAGE_CEL_H__
#define __IMAGE_CEL_H__
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
#include "Image.h"
#include "SubImage.h"
#include "ImageCopyDimension.h"
#include "PixelOpacity.h"


//Note:  CImageCel can have transparency, translucency etc... if there is an opacity channel.
//	     CImageCelTransparent and CImageCelMask allow additional trasparency through masking.


class CImageCel : public CObject
{
CONSTRUCTABLE(CImageCel);
DESTRUCTABLE(CImageCel);
protected:
	CSubImage		mcSubImage;  //The full dimension of the cel and where it's pixels are in the source image.
	Ptr<CImage>		mpSourceImage;

public:
			void 			Init(Ptr<CImage> pcSourceImage);
			void			Init(Ptr<CImage> pcSourceImage, CRectangle* pcRect);
			void 			Init(Ptr<CImage> pcSourceImage, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom);
			void 			Init(Ptr<CImage> pcSourceImage, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, int32 iLeftOffset, int32 iTopOffset, int32 iRightOffset, int32 iBottomOffset, int32 iAlignment = SUB_IMAGE_ALIGNMENT_LEFT | SUB_IMAGE_ALIGNMENT_TOP);
			void			Class(void);
			void 			Free(void);

			bool			Save(CObjectWriter* pcFile);
			bool			Load(CObjectReader* pcFile);

			void			Copy(CImageCel* pcSource);
			void			Print(CChars* psz);
			void			Dump(void);

	//Remove these from CImageCel, create a new class CDividerCel and put them there.
	virtual	void			CropTransparentBorders(void);
			void			CropTransparentBorders(CPixelOpacityBase* pcPixelOpacity);
			bool			IsColumnTransparent(CPixelOpacityBase* pcOpacity, int32 x, int32 y1, int32 y2); //X/Y in Source space
			bool			IsRowTransparent(CPixelOpacityBase* pcOpacity, int32 y, int32 x1, int32 x2);  //X/Y in Source space

	virtual	void			GetAllChannels(CArrayChannel* pasChannels);
	virtual	bool			MustFixDrawOpacity(void);
	virtual	void			FixDrawOpacity(CImageAccessor* pcDestOpacity, int32 iDestTop, int32 iDestLeft);
			void			FixDrawOpacity(CPixelOpacityBase* pcPixelOpacity, CImageAccessor* pcDestOpacity, int32 iDestTop, int32 iDestLeft);
			CSubImage*		GetSubImage(void);
			void			SetSubImage(CSubImage* pcSubImage);
			Ptr<CImage>		GetSourceImage(void);
			void			GetImageDestBounds(int32 x, int32 y, CRectangle* pcReturn);
			void			GetImageSourceBounds(CRectangle* pcReturn);

			void			CopyParam(SImageCopy* psCopy, int32 iDestX, int32 iDestY, int32 iDestWith, int32 iDestHeight);
};


typedef CArray<CImageCel> CArrayImageCel;


#endif // __IMAGE_CEL_H__

