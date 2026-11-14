/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __IMAGE_TEXELS_H__
#define __IMAGE_TEXELS_H__
#include "BaseLib/PrimitiveTypes.h"


class CImage;


//An image's texels as every renderer textures with them: 0xAARRGGBB, its rows in the order the image has them, puiTexels having
//room for its width times its height.  A colour it hasn't a channel for is all there, as is alpha, which is its opacity.  False,
//writing nothing, for an image that isn't bytes.
bool GetImageTexels(CImage* pcImage, uint32* puiTexels);


#endif // __IMAGE_TEXELS_H__

