/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __IMAGE_CEL_H__
#define __IMAGE_CEL_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayType.h"
#include "Image.h"
#include "SubImage.h"
#include "PixelOpacity.h"


//Note:  CImageCel can have transparency, translucency etc... if there is an opacity channel.
//	     CImageCelTransparent and CImageCelMask allow additional trasparency through masking.


class CImageCel : public CUnknown
{
BASE_FUNCTIONS(CImageCel);
protected:
	CSubImage		mcSubImage;  //The full dimension of the cel and where it's pixels are in the source image.
	CImage*			mpcSourceImage;

public:
	void 			Init(CImage* pcSourceImage);
	void			Init(CImage* pcSourceImage, CRectangle* pcRect);
	void 			Init(CImage* pcSourceImage, int iLeft, int iTop, int iRight, int iBottom);
	void 			Init(CImage* pcSourceImage, int iLeft, int iTop, int iRight, int iBottom, int iLeftOffset, int iTopOffset, int iRightOffset, int iBottomOffset, int iAlignment = SUB_IMAGE_ALIGNMENT_LEFT | SUB_IMAGE_ALIGNMENT_TOP);
	void			Kill(void);

	void			Copy(CImageCel* pcSource);

	//Remove these from CImageCel, create a new class CDividerCel and put them there.
	virtual	void	CropTransparentBorders(void);
	void			CropTransparentBorders(CPixelOpacityBase* pcPixelOpacity);
	BOOL			IsColumnTransparent(CPixelOpacityBase* pcOpacity, int x, int y1, int y2); //X/Y in Source space
	BOOL			IsRowTransparent(CPixelOpacityBase* pcOpacity, int y, int x1, int x2);  //X/Y in Source space

	virtual	void	GetAllChannels(CArrayChannel* pasChannels);
	virtual	BOOL	MustFixDrawOpacity(void);
	virtual	void	FixDrawOpacity(CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft);
	void			FixDrawOpacity(CPixelOpacityBase* pcPixelOpacity, CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft);
	CSubImage*		GetSubImage(void);
	void			SetSubImage(CSubImage* pcSubImage);
	CImage*			GetSourceImage(void);
};


typedef CArrayType<CImageCel> CArrayImageCel;


#endif // __IMAGE_CEL_H__

