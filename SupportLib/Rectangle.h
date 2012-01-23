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


//Left 0 to Right 1 gives a width of 1 pixel.  0 to 0 gives a width of 0.

class CRectangle
{
public:
	int    miLeft;
	int    miTop;

	int    miRight;
	int    miBottom;

	void	Init(void);
	void 	Init(int iLeft, int iTop, int iRight, int iBottom);
	void 	Init(SInt2 sTopLeft, SInt2 sBottomRight);
	void 	Init(CRectangle* pcSource);
	void 	Kill(void);
	void	Copy(CRectangle* pcSource);
	int		GetLeft(void);
	int		GetTop(void);
	int		GetRight(void);
	int		GetBottom(void);
	int 	GetWidth(void);
	int 	GetHeight(void);
	int 	GetArea(void);
	void	SetWidth(int iWidth);
	void	SetHeight(int iHeight);
	void	SetSize(int iWidth, int iHeight);
	void	SetPos(int x, int y);
	void	GrowToContain(int iXPos, int iYPos);
	BOOL	IsValid(void);
	BOOL	IsIn(int x, int y);
	BOOL	IsIn(int iLeft, int iTop, int iRight, int iBottom);
	SFloat2	GetUVCoordinatesTopLeft(int iTextureXSize, int iTextureYSize);
	SFloat2	GetUVCoordinatesBottomRight(int iTextureXSize, int iTextureYSize);
};


typedef CArrayTemplate<CRectangle>	CArrayRectangle;


#endif // __RECTANGLE_H__

