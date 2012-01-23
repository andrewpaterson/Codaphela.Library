/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "GridCell.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridCell::Init(int iTop, int iLeft, int iWidth, int iHeight, int iWidthFill, int iHeightFill)
{
	msTopLeft.Init(iTop, iLeft);
	msSize.Init(iWidth, iHeight);
	sWidthParams.bUseDesiredSize = FALSE;
	sWidthParams.u.sFill.iWeight = iWidthFill;
	sHeightParams.bUseDesiredSize = FALSE;
	sHeightParams.u.sFill.iWeight = iHeightFill;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridCell::Init(int iTop, int iLeft, int iWidth, int iHeight, int iWidthFill, EGridUseDesiredSizeAlignment eUseDesiredHeight)
{
	msTopLeft.Init(iTop, iLeft);
	msSize.Init(iWidth, iHeight);
	sWidthParams.bUseDesiredSize = FALSE;
	sWidthParams.u.sFill.iWeight = iWidthFill;
	sHeightParams.bUseDesiredSize = TRUE;
	sHeightParams.u.sDesiredSize.eAlign = eUseDesiredHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridCell::Init(int iTop, int iLeft, int iWidth, int iHeight, EGridUseDesiredSizeAlignment eUseDesiredWidth, int iHeightFill)
{
	msTopLeft.Init(iTop, iLeft);
	msSize.Init(iWidth, iHeight);
	sWidthParams.bUseDesiredSize = TRUE;
	sWidthParams.u.sDesiredSize.eAlign = eUseDesiredWidth;
	sHeightParams.bUseDesiredSize = FALSE;
	sHeightParams.u.sFill.iWeight = iHeightFill;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridCell::Init(int iTop, int iLeft, int iWidth, int iHeight, EGridUseDesiredSizeAlignment eUseDesiredWidth, EGridUseDesiredSizeAlignment eUseDesiredHeight)
{
	msTopLeft.Init(iTop, iLeft);
	msSize.Init(iWidth, iHeight);
	sWidthParams.bUseDesiredSize = TRUE;
	sWidthParams.u.sDesiredSize.eAlign = eUseDesiredWidth;
	sHeightParams.bUseDesiredSize = TRUE;
	sHeightParams.u.sDesiredSize.eAlign = eUseDesiredHeight;
}

