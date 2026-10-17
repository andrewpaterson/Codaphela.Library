#ifndef __GRID_CONTAINER_H__
#define __GRID_CONTAINER_H__
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
#include "BaseLib/ArrayInt.h"
#include "GridCell.h"
#include "Container.h"


//Lays its components out in columns and rows.  Each column is as wide as the widest component that sits only in it and
//each row is as tall as the tallest.  A column's weight is the largest weight of the components sitting only in it.
//A component spanning several columns adds what they still lack, shared out by their weights or, if none of them has a
//weight, given to the last of them.  Space left over goes to the weighted columns in proportion to their weights; with no
//weighted columns the grid keeps its required size and is centred.  When there is too little space the weighted columns
//give it up first, then every column shrinks in proportion to its size.  Rows work the same way.
//
//A component with a desired size of -1 on an axis fills its cell on that axis, otherwise it keeps its size and is centred
//in its cell.  Cells may overlap and are drawn in the order they were added.
class CGridContainer : public CContainer
{
CONSTRUCTABLE(CGridContainer);
DESTRUCTABLE(CGridContainer);
protected:
	CArrayGridCells		macCells;  //The cell for the component at the same index in maChildren.

public:
	void		Init(Ptr<CWindow> pWindow);
	void		Free(void) override;
	void		Class(void) override;

	bool		AddComponent(Ptr<CComponent> pComponent, int iColumn, int iRow, int iColumnSpan = 1, int iRowSpan = 1, int iWidthWeight = 0, int iHeightWeight = 0);
	bool		AddComponent(Ptr<CComponent> pComponent) override;  //Fails, a component must be added with a cell.
	bool		RemoveComponent(Ptr<CComponent> pComponent) override;
	bool		RemoveAllComponents(void) override;

	void		SetRequiredSize(void) override;
	void		Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;

protected:
	CGridCell*	GetCell(size uiIndex);
	size		NumTracks(bool bColumns);
	void		CalculateTracks(bool bColumns, CArrayInt* paiSizes, CArrayInt* paiWeights);
	void		LayoutTracks(bool bColumns, int iStart, int iAvailable, CArrayInt* paiStarts, CArrayInt* paiSizes);
};


#endif // __GRID_CONTAINER_H__

