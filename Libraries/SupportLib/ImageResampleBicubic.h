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
#ifndef __IMAGE_RESAMPLE_BICUBIC_H__
#define __IMAGE_RESAMPLE_BICUBIC_H__
#include "Image.h"


//Resamples every channel of pcSource into pcDest, which must already have the same channels and its new size.
//Uses the Catmull-Rom cubic (Keys, a = -0.5) along x then along y, in float64.  Pixel centres line up as they do for
//bilinear and pixels past the edges are the edge pixels.  Integer channels are rounded to the nearest value and clamped
//to their range (the cubic overshoots at hard edges); float channels are not clamped.
//Returns false if a channel can't be resampled (reversed or not a primitive).
bool ResampleBicubic(Ptr<CImage> pcSource, Ptr<CImage> pcDest);


#endif // __IMAGE_RESAMPLE_BICUBIC_H__

