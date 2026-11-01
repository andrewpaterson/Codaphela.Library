#ifndef __CONTAINER_STYLE_H__
#define __CONTAINER_STYLE_H__
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
#include "BaseLib/Int32Vec2.h"


enum EContainerStyleWrap
{
	CSW_Unknown,
	CSW_Wrap,
	CSW_Continue,
};


enum EContainerStyleDirection
{
	CSD_Unknown,
	CSD_Right,
	CSD_Down,
	CSD_Up,
	CSD_Left,
};


enum EContainerStyleHorizontal
{
	CSH_Unknown,
	CSH_Stretch,
	CSH_Left,
	CSH_Center,
	CSH_Right,
};


enum EContainerStyleVertical
{
	CSV_Unknown,
	CSV_Stretch,
	CSV_Top,
	CSV_Center,
	CSV_Bottom,
};


int		AlignComponentVertical(EContainerStyleVertical eAlignment, int iComponentHeight, int iAvailableHeight);
int		AlignComponentHorizontal(EContainerStyleHorizontal eAlignment, int iComponentWidth, int iAvailableWidth);

int32	ContainerStyleMainAxis(SInt32Vec2* psVec, bool bHorizontal);  //The x of sVec if bHorizontal, otherwise the y.
int32	ContainerStyleCrossAxis(SInt32Vec2* psVec, bool bHorizontal);  //The other one.


#endif // __CONTAINER_STYLE_H__

