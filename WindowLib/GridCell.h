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
#ifndef __GRID_CELL_H__
#define __GRID_CELL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/DataTypes.h"
#include "ContainerCell.h"


enum EGridUseDesiredSizeAlignment
{
	GUDSA_LeftAlign = 0,
	GUDSA_TopAlign = 0,
	GUDSA_CenterAlign = 1,
	GUDSA_RightAlign = 2,
	GUDSA_BottomAlign = 2,
	GUDSA_Fill = 3,
};


struct SUseDesiredSizeParams
{
	EGridUseDesiredSizeAlignment eAlign;
};


struct SFillParams
{
	int iWeight;
};


struct SGridCellAxisParams
{
	BOOL	bUseDesiredSize;
	union
	{
		SUseDesiredSizeParams	sDesiredSize;
		SFillParams				sFill;
	} u;
};


class CGridCell : public CContainerCell
{
public:
	SInt2					msTopLeft;  //Position in grid.
	SInt2					msSize;  //Size in grid.
	SGridCellAxisParams		sWidthParams;
	SGridCellAxisParams		sHeightParams;

	void Init(int iTop, int iLeft, int iWidth, int iHeight, int iWidthFill, int iHeightFill);
	void Init(int iTop, int iLeft, int iWidth, int iHeight, int iWidthFill, EGridUseDesiredSizeAlignment eUseDesiredHeight);
	void Init(int iTop, int iLeft, int iWidth, int iHeight, EGridUseDesiredSizeAlignment eUseDesiredWidth, int iHeightFill);
	void Init(int iTop, int iLeft, int iWidth, int iHeight, EGridUseDesiredSizeAlignment eUseDesiredWidth, EGridUseDesiredSizeAlignment eUseDesiredHeight);
};


typedef CArrayTemplate<CGridCell>	CArrayGridCells;


#endif // __GRID_CELL_H__

