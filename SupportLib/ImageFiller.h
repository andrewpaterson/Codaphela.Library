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
#ifndef __IMAGE_FILLER_H__
#define __IMAGE_FILLER_H__
#include "Image.h"
#include "ImageAccessorCreator.h"
#include "FillRectangle.h"
#include "SubImage.h"


#define FILL_NONE	0
#define FILL_UP		1
#define FILL_DOWN	2
#define FILL_LEFT	4
#define FILL_RIGHT	8
#define FILL_ALL	(FILL_UP | FILL_DOWN | FILL_LEFT | FILL_RIGHT)


struct SImageFill
{
	int x;
	int y;
	int iFill;
};


typedef CArrayTemplate<SImageFill> CArrayImageFill;


class CImageFiller
{
public:
	CArrayImageFill			mcStack;
	void*					mpvBorderColour;
	CImageAccessor*			mpcSourceAccessor;
	CImage*					mpcDestImage;
	CFillRectangle*			mpcDestSubImage;
	CImageAccessor*			mpcDestMaskAccessor;
	short					miMask;

	void Init(void* pvBorderColour, CImageAccessor* pcSource, CImage* pcDestImage);
	void Kill(void);
	
	BOOL Fill(int x, int y, CFillRectangle* pcDestSubImage, short iMask);
	BOOL Push(int x, int y, int iRemove);
};


#endif // __IMAGE_FILLER_H__

