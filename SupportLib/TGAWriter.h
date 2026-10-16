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
#ifndef __TGA_WRITER_H__
#define __TGA_WRITER_H__
#include "Image.h"


//Writes RGB images as 24 bit, RGBA as 32 bit, grey as 8 bit and grey with alpha as 16 bit Targa images, optionally RLE compressed.
bool SaveTGA(Ptr<CImage> pcImage, char* szPathName, bool bRLE = false);


#endif // __TGA_WRITER_H__
