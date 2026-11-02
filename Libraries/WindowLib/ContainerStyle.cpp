/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "ContainerStyle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int AlignComponentVertical(EContainerStyleVertical eAlignment, int iComponentHeight, int iAvailableHeight)
{
	if (eAlignment == CSV_Top)
	{
		return 0;
	}
	else if (eAlignment == CSV_Center)
	{
		return (iAvailableHeight - iComponentHeight) / 2;
	}
	else if (eAlignment == CSV_Bottom)
	{
		return iAvailableHeight - iComponentHeight;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int AlignComponentHorizontal(EContainerStyleHorizontal eAlignment, int iComponentWidth, int iAvailableWidth)
{
	if (eAlignment == CSH_Left)
	{
		return 0;
	}
	else if (eAlignment == CSH_Center)
	{
		return (iAvailableWidth - iComponentWidth) / 2;
	}
	else if (eAlignment == CSH_Right)
	{
		return iAvailableWidth - iComponentWidth;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 ContainerStyleMainAxis(SInt32Vec2* psVec, bool bHorizontal)
{
	return bHorizontal ? psVec->x : psVec->y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 ContainerStyleCrossAxis(SInt32Vec2* psVec, bool bHorizontal)
{
	return bHorizontal ? psVec->y : psVec->x;
}

