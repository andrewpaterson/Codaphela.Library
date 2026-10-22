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
#ifndef __PNG_READER_H__
#define __PNG_READER_H__
#include "Image.h"


//Reads grey, grey with alpha, RGB and RGBA PNGs of any bit depth, and palette PNGs (alpha from tRNS), interlaced or not.
//1, 2 and 4 bit grey is read as 8 bit.  Palettes that are all grey are read as grey.  Opacity is the first channel.
Ptr<CImage> LoadPNG(char* szFilename, bool bAddDebug = false);


#endif // __PNG_READER_H__

