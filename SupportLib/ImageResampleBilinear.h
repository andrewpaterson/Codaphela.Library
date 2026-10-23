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

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_RESAMPLE_BILINEAR_H__
#define __IMAGE_RESAMPLE_BILINEAR_H__
#include "Image.h"


#define BILINEAR_ONE		0x10000		//1.0 in 16.16 fixed point.


//The two source pixels and the weight of the second (0 to BILINEAR_ONE) for each dest pixel along one axis.
struct SBilinearAxis
{
	int32*	paiFirst;
	int32*	paiSecond;
	int32*	paiFraction;

	void	Init(uint32 uiSourceSize, uint32 uiDestSize);
	void	Kill(void);
};


//Resamples every channel of pcSource into pcDest, which must already have the same channels and its new size.
//Pixel centres line up: dest pixel x samples source position (x + 0.5) * source width / dest width - 0.5, and positions
//past the edges use the edge pixels.  Positions are 16.16 fixed point.  Integer channels of 16 bits or fewer (including
//the bit sized types) are interpolated with integer maths only and rounded to the nearest value; other channels in float64.
//Returns false if a channel can't be resampled (reversed or not a primitive).
bool ResampleBilinear(Ptr<CImage> pcSource, Ptr<CImage> pcDest);


#endif // __IMAGE_RESAMPLE_BILINEAR_H__

