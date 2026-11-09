/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OLD_DRAGON_IMAGE_READER_H__
#define __OLD_DRAGON_IMAGE_READER_H__
#include "OldDragonCursor.h"
#include "OldDragonScene.h"


//In 2009 and Max images, the pixels are a simple array of bytes at the very end, after channel descriptions that aren't
//decoded.  pcCursor holds what's left of the chunk and the image knows its size.  An image whose pixels can't be found
//isn't an error: it's left with no pixels.
bool	ReadOldDragonImagePixels(COldDragonCursor* pcCursor, COldDragonImage* pcImage);


#endif // __OLD_DRAGON_IMAGE_READER_H__

