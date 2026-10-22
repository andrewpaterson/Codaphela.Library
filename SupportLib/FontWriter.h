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
#ifndef __FONT_WRITER_H__
#define __FONT_WRITER_H__
#include "Font.h"


//Writes a text descriptor of the font and, with CelsWriter, the cels of its glyphs.  All the glyphs must share one
//source image.  szImagePathName is where that image is on disk, relative to the working directory.  The image itself
//is not written.  Glyphs keyed with more than four bytes can't be written.
bool FontWriter(Ptr<CFont> pFont, char* szImagePathName, char* szCelsPathName, char* szPathName);


#endif // __FONT_WRITER_H__
