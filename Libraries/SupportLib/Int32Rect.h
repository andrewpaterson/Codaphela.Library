/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __INT32_RECT_H__
#define __INT32_RECT_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Float32Vec2.h"
#include "Rectangle.h"


//An integer rectangle, usually of pixels.  The pixel methods treat a point as the one pixel square below and right of it.
class CInt32Rect : public CRectangle<SInt32Vec2, int32, int64>
{
public:
	void			GrowToContain(int32 iXPos, int32 iYPos);
	void			GrowToContain(CInt32Rect* pcOther);

	SFloat32Vec2	GetUVCoordinatesTopLeft(int32 iTextureXSize, int32 iTextureYSize);
	SFloat32Vec2	GetUVCoordinatesBottomRight(int32 iTextureXSize, int32 iTextureYSize);
};


typedef CArrayTemplate<CInt32Rect>	CArrayInt32Rect;


#endif // __INT32_RECT_H__

