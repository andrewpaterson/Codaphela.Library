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
#ifndef __IMAGE_PREMULTIPLIED_BLEND_H__
#define __IMAGE_PREMULTIPLIED_BLEND_H__
#include "Image.h"
#include "Rectangle.h"


//Blends pSource over pDest with its top left at iDestX, iDestY, clipped to pDest.  The source's colours must already be
//multiplied by its opacity, so each channel becomes source + dest * (1 - source opacity).  If pDest has an opacity
//channel it is blended the same way.  Both images must have 8 bit colour channels in the same order and the source an
//8 bit opacity channel.  Every pixel is blended; nothing about the source is assumed from what it held before.  If
//pcClip is not NULL only the destination pixels inside it are blended.
bool BlendPremultipliedImage(Ptr<CImage> pSource, Ptr<CImage> pDest, int32 iDestX, int32 iDestY, CRectangle* pcClip = NULL);


#endif // __IMAGE_PREMULTIPLIED_BLEND_H__
