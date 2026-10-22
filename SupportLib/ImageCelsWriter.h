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
#ifndef __IMAGE_CELS_WRITER_H__
#define __IMAGE_CELS_WRITER_H__
#include "ImageCel.h"


//Writes a text descriptor of the cels.  All the cels must share one source image.
//szImagePathName is where that image is on disk, relative to the working directory.  The image itself is not written.
bool CelsWriter(Ptr<CArrayImageCel> paCels, char* szImagePathName, char* szPathName);

//Writing shared with other text descriptors written in the same style.
void AppendImageCelsIntegers(CChars* szText, char* szIdentifier, int32* aiValues, size uiCount);
void AppendImageCelsPath(CChars* szText, char* szIdentifier, char* szPathName);


#endif // __IMAGE_CELS_WRITER_H__

