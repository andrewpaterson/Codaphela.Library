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
#include "GridContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Init(Ptr<CWindow> pWindow)
{
	CContainer::Init(pWindow);
	macCells.Init();
	maiCellIndices.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Free(void)
{
	macCells.Kill();
	maiCellIndices.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::SetRequiredSize(void)
{
	Ptr<CBasicComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	SInt2					sTotalSize;

	msRequiredSize.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGridContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	Ptr<CBasicComponent>	pComponent;
	size					uiSize;
	size					i;
	SInt2					sSize;
	int						fXPos;
	int						fYPos;
	SInt2					sOffset;

	SetBounds(sPosition, sAreaSize);

	fXPos = 0;
	fYPos = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();

		pComponent->Layout(sOffset, sSize);
	}
}

