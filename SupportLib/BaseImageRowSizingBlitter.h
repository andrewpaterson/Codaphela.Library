#ifndef __BASE_IMAGE_ROW_SIZING_BLITTER_H__
#define __BASE_IMAGE_ROW_SIZING_BLITTER_H__
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
#include "StandardLib/Pointer.h"
#include "ImageBlitterContext.h"


//Copies one row segment of a cel that is drawn wider or narrower than it is.  Destination pixel iDestXLeft + i is taken
//from source pixel paiSourceX[i] of row iSourceY so source pixels can be repeated (stretching) or skipped (shrinking).
class CBaseImageRowSizingBlitter
{
public:
	virtual void	Copy(CImageBlitterContext* pcContext, void* pvSource, void* pvDest, size iDestXLeft, size iDestXRight, size iDestY, int32* paiSourceX, size iSourceY) =0;
};


#endif // __BASE_IMAGE_ROW_SIZING_BLITTER_H__

