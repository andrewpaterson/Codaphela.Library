/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __IMAGE_DIVIDER_H__
#define __IMAGE_DIVIDER_H__
#include "StandardLib/SetUnknown.h"
#include "Image.h"
#include "ImageCel.h"
#include "ImageFiller.h"
#include "ImageDividerNumbers.h"


class CImageDivider : public CObject
{
CONSTRUCTABLE(CImageDivider);
DESTRUCTABLE(CImageDivider);
protected:
	Ptr<CImage>		mpcImage;

	bool			mbIgnoreEmpty;
	SImageColour	msTransparentColour;
	bool			mbUseTransparentColour;
	bool			mbCropTransparentBorders;

	CArrayImageCel	mcDestImageCels;

public:
	void 					Init(Ptr<CImage> pcImage, SImageColour* psTransparentColour = NULL, bool bIgnoreEmpty = true, bool bCropTransparentBorders = true);
	void					Class(void);
	void 					Free(void);

	bool					Save(CObjectWriter* pcFile);
	bool					Load(CObjectReader* pcFile);

	void		 			GenerateFromBorder(Ptr<CImage> pcFillMask);  //Use the pixel colour RGB (+A if available) in the top left corner to mask out rectangles.
	void 					GenerateFromNumbers(CImageDividerNumbers* pcNumbers);
	void 					GenerateFromRectangles(CArrayRectangle* pacRectangles);

	void 					CopyCellsTo(Ptr<CArrayImageCel> pcImageCels);

	Ptr<CArrayImageCel>		GetDestImageCels(void);

private:
	void	CropTransparentBorders(void);
	void	AddImageCel(CFillRectangle* pcRect, Ptr<CImage> pcFillMask);
	void	AddImageCel(CRectangle* pcRect);
};


#endif // __IMAGE_DIVIDER_H__

