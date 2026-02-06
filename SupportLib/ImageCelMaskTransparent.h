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
#ifndef __IMAGE_CEL_MASK_TRANSPARENT_H__
#define __IMAGE_CEL_MASK_TRANSPARENT_H__
#include "ImageColour.h"
#include "ImageCelMask.h"


class CImageCelMaskTransparent : public CImageCelMask
{
CONSTRUCTABLE(CImageCelMaskTransparent);
protected:
	SImageColour	msTransparentColour;  //Treat this colour as transparent.

public:
	void 	Init(Ptr<CImage> pcSourceImage, SImageColour* psTransparent, Ptr<CImage> pcMaskImage, CFillRectangle* pcRect);
	void 	Init(Ptr<CImage> pcSourceImage, SImageColour* psTransparent, Ptr<CImage> pcMaskImage, short iMask, bool bFilled, int iLeft, int iTop, int iRight, int iBottom);
	void	Class(void);
	void 	Free(void);

	bool	Save(CObjectWriter* pcFile);
	bool	Load(CObjectReader* pcFile);

	void	CropTransparentBorders(void);

	void	GetAllChannels(CArrayChannel* pasChannels);
	bool	MustFixDrawOpacity(void);
	void	FixDrawOpacity(CImageAccessor* pcDestOpacity, int iDestTop, int iDestLeft);
};


#endif // __IMAGE_CEL_MASK_H__

