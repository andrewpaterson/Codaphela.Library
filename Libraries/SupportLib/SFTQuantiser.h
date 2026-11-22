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
#ifndef __SFT_QUANTISER_H__
#define __SFT_QUANTISER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Image.h"
#include "SFTCommon.h"


//Images made palettised for SFT (SFTCommon.h): one palette for all of them, and each image's pixels as indices into it.
//A pixel whose opacity is below half is transparent: if any is, index 0 is transparent and the colours are from index 1
//(no more than 255 of them), else from 0 (no more than 256).  An image with a transparent pixel is a transparent image.
struct SSFTIndexedImage
{
	uint16	uiWidth;
	uint16	uiHeight;
	bool	bTransparent;
	uint8*	puiPixels;	//uiWidth x uiHeight indices, a row after another.  0 in a transparent image isn't drawn.
};


class CSFTQuantised
{
public:
	SSFTColours			msColours;
	SSFTIndexedImage*	masImages;
	size				muiNumImages;

	void	Init(void);
	void	Kill(void);
};


//The images' colours: their own if there are no more than uiMaxColours of them (and no more than the palette has room
//for), else that many chosen by median cut (the colours split at the weighted median of their widest channel until
//there are enough, each palette entry the mean of its colours), with each pixel the nearest entry's index.  No
//dithering.  The images need red, green and blue channels; opacity is optional.  False if an image hasn't them or is
//more than 65535 across or down.
bool	QuantiseSFT(CArrayImage* pacImages, uint16 uiMaxColours, CSFTQuantised* pcResult);


#endif // __SFT_QUANTISER_H__
