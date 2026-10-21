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
#include "StandardLib/Objects.h"
#include "BorderLayoutHelper.h"


//Indexed by EBorderLayoutCell.  Each outer cell's component is aligned towards the centre cell.
static EContainerStyleHorizontal gaeBorderHorizontal[9] = {	CSH_Right,	CSH_Center,	CSH_Left,
																CSH_Right,	CSH_Center,	CSH_Left,
																CSH_Right,	CSH_Center,	CSH_Left };
static EContainerStyleVertical gaeBorderVertical[9] = {		CSV_Bottom,	CSV_Bottom,	CSV_Bottom,
																CSV_Center,	CSV_Center,	CSV_Center,
																CSV_Top,	CSV_Top,	CSV_Top };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderLayoutHelper::Init(Ptr<CWindow> pWindow)
{
	mpGrid = OMalloc<CGridContainer>(pWindow);
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out an existing grid, such as one a component extends.
//
//////////////////////////////////////////////////////////////////////////
void CBorderLayoutHelper::Init(Ptr<CGridContainer> pGrid)
{
	mpGrid = pGrid;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderLayoutHelper::Kill(void)
{
	mpGrid = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBorderLayoutHelper::AddComponent(Ptr<CComponent> pComponent, EBorderLayoutCell eCell)
{
	int		iColumn;
	int		iRow;
	int		iWeight;

	if ((eCell < BLC_TopLeft) || (eCell > BLC_BottomRight))
	{
		return false;
	}

	iColumn = eCell % 3;
	iRow = eCell / 3;
	if (eCell == BLC_Center)
	{
		iWeight = 1;
	}
	else
	{
		iWeight = 0;
		pComponent->SetAlignment(gaeBorderHorizontal[eCell], gaeBorderVertical[eCell]);
	}
	return mpGrid->AddComponent(pComponent, iColumn, iRow, 1, 1, iWeight, iWeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CGridContainer> CBorderLayoutHelper::GetContainer(void)
{
	return mpGrid;
}

