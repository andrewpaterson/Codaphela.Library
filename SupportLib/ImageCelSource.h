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
#ifndef __IMAGE_CEL_SOURCE_H__
#define __IMAGE_CEL_SOURCE_H__
#include "StandardLib/Unknown.h"
#include "Image.h"
#include "ImageDivider.h"
#include "ImageCel.h"


//Image Cel Sources can be used multiple times on different images.  They cannot carry the results of their divide.
class CImageCelSource : public CUnknown
{
CONSTRUCTABLE(CImageCelSource);
protected:
	bool			mbIgnoreEmpty;
	SImageColour*	mpsTransparentColour;
	bool			mbCropTransparentBorders;

public:
	void			Init(SImageColour* psTransparentColour, bool bIgnoreEmpty, bool bCropTransparentBorders);
	virtual void	Divide(CImage* pcImage, Ptr<CArrayImageCel> pcDestImageCels, CImage* pcMask) =0;
	virtual bool	NeedsMask(void);
	void			Kill(void);
};


#endif // __IMAGE_CEL_SOURCE_H__

