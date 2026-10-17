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
#include "BaseLib/Int32Vec2.h"


//Where a component sits in a grid container.  The weights decide how the space left over, once every column and row
//has its required size, is shared out.  A weight of zero means the cell only wants its required size.  A positive weight
//shares the left over space in proportion to the other positive weights.
class CGridCell
{
public:
	SInt32Vec2	msPosition;	//The column and row of the top left of the cell.
	SInt32Vec2	msSpan;		//The number of columns and rows the cell covers.
	SInt32Vec2	msWeight;	//The width weight and the height weight.

	void	Init(int iColumn, int iRow, int iColumnSpan, int iRowSpan, int iWidthWeight, int iHeightWeight);
};


typedef CArrayTemplate<CGridCell>	CArrayGridCells;


#endif // __GRID_CELL_H__

