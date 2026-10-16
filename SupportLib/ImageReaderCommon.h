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
#ifndef __IMAGE_READER_COMMON_H__
#define __IMAGE_READER_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"
#include "Image.h"


//Copies decoded pixels (red, green, blue, alpha; 4 bytes per pixel, top row first) into pcImage, keeping only the channels pcImage has.
//If bGrey the red byte is the grey value.
bool	CopyRGBAIntoImage(uint8* puiRGBA, bool bGrey, Ptr<CImage> pcImage);


#endif // __IMAGE_READER_COMMON_H__
