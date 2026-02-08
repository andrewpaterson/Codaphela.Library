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
#include "Component.h"
#include "FillContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Init(Ptr<CWindow> pWindow)
{
	CContainer::Init(pWindow);
	meHorizontal = CSH_Stretch;
	meVertical = CSV_Stretch;
	msInsets.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetFillStyle(EContainerStyleHorizontal eH, EContainerStyleVertical eV)
{
	meHorizontal = eH;
	meVertical = eV;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetRequiredSize(void)
{
	size					i;
	Ptr<CComponent>	pComponent;
	size					uiSize;
	SInt2					sSize;

	//There should only be one componet, but if there are the largest size will be used.
	msRequiredSize.Init(0, 0);
	
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.x > msRequiredSize.x)
		{
			msRequiredSize.x = sSize.x;
		}
		if (sSize.y > msRequiredSize.y)
		{
			msRequiredSize.y = sSize.y;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Layout(SInt2 sPosition, SInt2 sAreaSize)
{
	SInt2					sSize;
	size					i;
	SInt2					sOrigin;
	SInt2					sArea;
	Ptr<CComponent>	pComponent;
	size					uiSize;

	sOrigin = sPosition;
	sArea = sAreaSize;
	sOrigin.x += msInsets.maiWidths[BEI_Left];
	sOrigin.y += msInsets.maiWidths[BEI_Top];
	sArea.x -= (msInsets.maiWidths[BEI_Left] + msInsets.maiWidths[BEI_Right]);
	sArea.y -= (msInsets.maiWidths[BEI_Bottom] + msInsets.maiWidths[BEI_Top]);

	SetBounds(sOrigin, sArea);

	//There should only be one componet, any more will be laid out into the same area.
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		sPosition = sOrigin;
		sAreaSize = sArea;

		pComponent = maChildren.Get(i);

		sSize = pComponent->GetDesiredSize();
		if (sSize.x == -1)
		{
			sSize = sAreaSize;
		}

		//Horizontal.
		if (meHorizontal == CSH_Stretch)
		{
		}
		else if (meHorizontal == CSH_Left)
		{
			sAreaSize.x = sSize.x;
		}
		else if (meHorizontal == CSH_Center)
		{
			sPosition.x = sPosition.x + (sAreaSize.x - sSize.x) / 2;
			sAreaSize.x = sSize.x;
		}
		else if (meHorizontal == CSH_Right)
		{
			sPosition.x = sPosition.x + sAreaSize.x - sSize.x;
			sAreaSize.x = sSize.x;
		}

		//Vertical.
		if (meVertical == CSV_Stretch)
		{
		}
		else if (meVertical == CSV_Top)
		{
			sAreaSize.y = sSize.y;
		}
		else if (meVertical == CSV_Center)
		{
			sPosition.y = sPosition.y + (sAreaSize.y - sSize.y) / 2;
			sAreaSize.y = sSize.y;
		}
		else if (meVertical == CSV_Bottom)
		{
			sPosition.y = sPosition.y + sAreaSize.y - sSize.y;
			sAreaSize.y = sSize.y;
		}

		pComponent->Layout(sPosition, sAreaSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}

