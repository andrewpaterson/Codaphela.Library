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
#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int2.h"



//Left 0 to Right 1 gives a width of 1.  0 to 0 gives a width of 0.

class CRectangle
{
public:
	int32    miLeft;
	int32    miTop;

	int32    miRight;
	int32    miBottom;

public:
	void		Init(void);
	void 		Init(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom);
	void		Init(int32 iWidth, int32 iHeight);
	void 		Init(int64 iLeft, int64 iTop, int64 iRight, int64 iBottom);
	void 		Init(SInt2 sTopLeft, SInt2 sBottomRight);
	void 		Init(CRectangle* pcSource);
	void 		Kill(void);
	void		Copy(CRectangle* pcSource);
	int32		GetLeft(void);
	int32		GetTop(void);
	int32		GetRight(void);
	int32		GetBottom(void);
	int32 		GetWidth(void);
	int32 		GetHeight(void);
	int32 		GetArea(void);
	void		SetWidth(int32 iWidth);
	void		SetHeight(int32 iHeight);
	void		SetSize(int32 iWidth, int32 iHeight);
	void		SetPos(int32 x, int32 y);
	void		GrowToContain(int32 iXPos, int32 iYPos);
	bool		IsValid(void);
	bool		IsIn(int32 x, int32 y);
	bool		IsIn(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom);
	bool		Equals(CRectangle* pcOther);

	SFloat2		GetUVCoordinatesTopLeft(int32 iTextureXSize, int32 iTextureYSize);
	SFloat2		GetUVCoordinatesBottomRight(int32 iTextureXSize, int32 iTextureYSize);
};


typedef CArrayTemplate<CRectangle>	CArrayRectangle;


#endif // __RECTANGLE_H__

