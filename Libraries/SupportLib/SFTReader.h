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
#ifndef __SFT_READER_H__
#define __SFT_READER_H__
#include "Image.h"
#include "SFTCommon.h"


//An image, or a container's images combined (SFTCommon.h).  NULL if it isn't an SFT image or container of this layout
//(SFT_VERSION), or is cut short.  A pixel is its palette's colour (the container's palette before the image), or with
//none SFTColour32's.
Ptr<CImage> LoadSFT(char* szFilename, bool bAddDebug = false);

//The record at the file's position.  psColours is the palette the image's pixels are in (NULL for none); a pixel that
//isn't in it fails.
Ptr<CImage> LoadSFTOpaque(CFileBasic* pcFile, bool bAddDebug = false, const SSFTColours* psColours = NULL);
Ptr<CImage> LoadSFTTransparent(CFileBasic* pcFile, bool bAddDebug = false, const SSFTColours* psColours = NULL);
Ptr<CImage> LoadSFTContainer(CFileBasic* pcFile, bool bAddDebug = false);


#endif // __SFT_READER_H__

