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
#include "Int32Rect.h"


#define HALF_TEXEL	(0.5f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInt32Rect::Init(int64 iLeft, int64 iTop, int64 iRight, int64 iBottom)
{
	msTopLeft.x = (int32)iLeft;
	msTopLeft.y = (int32)iTop;
	msBottomRight.x = (int32)iRight;
	msBottomRight.y = (int32)iBottom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInt32Rect::GrowToContain(int32 iXPos, int32 iYPos)
{
	if (iXPos < msTopLeft.x)
	{
		msTopLeft.x = iXPos;
	}
	if (iYPos < msTopLeft.y)
	{
		msTopLeft.y = iYPos;
	}

	if (iXPos >= msBottomRight.x)
	{
		msBottomRight.x = iXPos+1;
	}
	if (iYPos >= msBottomRight.y)
	{
		msBottomRight.y = iYPos+1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInt32Rect::GrowToContain(CInt32Rect* pcOther)
{
	if (pcOther)
	{
		GrowToContain(pcOther->msTopLeft.x, pcOther->msTopLeft.y);
		GrowToContain(pcOther->msBottomRight.x - 1, pcOther->msBottomRight.y - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2 CInt32Rect::GetUVCoordinatesTopLeft(int32 iTextureXSize, int32 iTextureYSize)
{
	SFloat32Vec2	sTopLeft;

	sTopLeft.x = ((float32)msTopLeft.x + HALF_TEXEL) / (float32)iTextureXSize;
	sTopLeft.y = ((float32)msTopLeft.y + HALF_TEXEL) / (float32)iTextureYSize;
	return sTopLeft;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec2 CInt32Rect::GetUVCoordinatesBottomRight(int32 iTextureXSize, int32 iTextureYSize)
{
	SFloat32Vec2 sBottomRight;

	sBottomRight.x = ((float32)msBottomRight.x + HALF_TEXEL) / (float32)iTextureXSize;
	sBottomRight.y = ((float32)msBottomRight.y + HALF_TEXEL) / (float32)iTextureYSize;
	return sBottomRight;
}

