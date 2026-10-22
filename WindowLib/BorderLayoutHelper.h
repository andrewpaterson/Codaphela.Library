/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __BORDER_LAYOUT_HELPER_H__
#define __BORDER_LAYOUT_HELPER_H__
#include "GridContainer.h"


//The nine cells of a border layout, from the top left to the bottom right.
enum EBorderLayoutCell
{
	BLC_TopLeft,
	BLC_Top,
	BLC_TopRight,
	BLC_Left,
	BLC_Center,
	BLC_Right,
	BLC_BottomLeft,
	BLC_Bottom,
	BLC_BottomRight,
};


//Creates a 3 x 3 grid container that lays its components out as a border around a centre component.  The centre cell
//has a weight of 1, 1 so its column and row take all the spare space.  The outer cells have no weight so their columns
//and rows keep their required sizes.  A column or row with no components in it takes no space.  Each outer component is
//aligned towards the centre, so the top left one sits at the bottom right of its cell and the top one at the bottom of
//its cell.  The centre component keeps its own alignment.
class CBorderLayoutHelper
{
protected:
	Ptr<CGridContainer>	mpGrid;

public:
	void					Init(Ptr<CWindow> pWindow);
	void					Init(Ptr<CGridContainer> pGrid);
	void					Kill(void);

	bool					AddComponent(Ptr<CComponent> pComponent, EBorderLayoutCell eCell);
	Ptr<CGridContainer>		GetContainer(void);
};


#endif // __BORDER_LAYOUT_HELPER_H__

