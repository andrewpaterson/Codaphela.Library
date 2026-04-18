#ifndef __IMAGE_COPY_DIMENSION_H__
#define __IMAGE_COPY_DIMENSION_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/PrimitiveTypes.h"


struct SImageCopy
{
	uint	uiDestX;
	uint	uiDestY;
	uint	uiSourceX1;
	uint	uiSourceY1;
	uint	uiSourceX2;
	uint	uiSourceY2;
	bool	bValid;

	void	Init(int iDestX, int iDestY, int iSourceX1, int iSourceY1, int iSourceX2, int iSourceY2, int iDestWidth, int iDestHeight);
};


#endif // __IMAGE_COPY_DIMENSION_H__

