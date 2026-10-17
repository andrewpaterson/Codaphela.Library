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
	meSplit = FCS_Overlay;
	miSpacing = 0;
	msInsets.Init();
	msRequiredSize.Init(-1, -1);
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
void CFillContainer::Class(void)
{
	CContainer::Class();
	U_Enum(meHorizontal);
	U_Enum(meVertical);
	U_Enum(meSplit);
	U_Int32(miSpacing);
	U_Data(SInsets, msInsets);
	U_2Int32(msRequiredSize);
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
void CFillContainer::SetSplit(EFillContainerSplit eSplit, int32 iSpacing)
{
	meSplit = eSplit;
	miSpacing = iSpacing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetRequiredSize(void)
{
	size				i;
	Ptr<CComponent>		pComponent;
	size				uiSize;
	SInt32Vec2			sSize;
	SInt32Vec2			sLargest;
	SInt32Vec2			sTotalSize;
	int32				iSpacing;

	//Every slot is the same size so the largest component decides the size of all of them.
	sLargest.Init(0, 0);

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.x > sLargest.x)
		{
			sLargest.x = sSize.x;
		}
		if (sSize.y > sLargest.y)
		{
			sLargest.y = sSize.y;
		}
	}

	sTotalSize = sLargest;
	if ((meSplit != FCS_Overlay) && (uiSize > 0))
	{
		iSpacing = miSpacing * ((int32)uiSize - 1);
		if (meSplit == FCS_Horizontal)
		{
			sTotalSize.x = sLargest.x * (int32)uiSize + iSpacing;
		}
		else if (meSplit == FCS_Vertical)
		{
			sTotalSize.y = sLargest.y * (int32)uiSize + iSpacing;
		}
	}

	sTotalSize.x += msInsets.maiWidths[BEI_Left] + msInsets.maiWidths[BEI_Right];
	sTotalSize.y += msInsets.maiWidths[BEI_Top] + msInsets.maiWidths[BEI_Bottom];
	msRequiredSize = sTotalSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CFillContainer::GetRequiredSize(void)
{
	return msRequiredSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size				i;
	SInt32Vec2			sOrigin;
	SInt32Vec2			sArea;
	SInt32Vec2			sSlotPosition;
	SInt32Vec2			sSlotSize;
	Ptr<CComponent>		pComponent;
	size				uiSize;
	int32				iLength;
	int32				iCellLength;
	int32				iExtra;
	int32				iPos;

	sOrigin = sPosition;
	sArea = sAreaSize;
	sOrigin.x += msInsets.maiWidths[BEI_Left];
	sOrigin.y += msInsets.maiWidths[BEI_Top];
	sArea.x -= (msInsets.maiWidths[BEI_Left] + msInsets.maiWidths[BEI_Right]);
	sArea.y -= (msInsets.maiWidths[BEI_Bottom] + msInsets.maiWidths[BEI_Top]);
	if (sArea.x < 0)
	{
		sArea.x = 0;
	}
	if (sArea.y < 0)
	{
		sArea.y = 0;
	}

	SetContainerBounds(sOrigin, sArea);

	uiSize = maChildren.Size();
	if ((meSplit == FCS_Overlay) || (uiSize == 0))
	{
		//Every component is laid out into the same area.
		for (i = 0; i < uiSize; i++)
		{
			pComponent = maChildren.Get(i);
			LayoutComponent(pComponent, sOrigin, sArea);
		}
		return;
	}

	//Split the area into equal slots.  Pixels that don't divide evenly go to the first slots.
	iLength = (meSplit == FCS_Horizontal) ? sArea.x : sArea.y;
	iLength -= miSpacing * ((int32)uiSize - 1);
	if (iLength < 0)
	{
		iLength = 0;
	}
	iCellLength = iLength / (int32)uiSize;
	iExtra = iLength % (int32)uiSize;

	iPos = (meSplit == FCS_Horizontal) ? sOrigin.x : sOrigin.y;
	for (i = 0; i < uiSize; i++)
	{
		sSlotPosition = sOrigin;
		sSlotSize = sArea;
		if (meSplit == FCS_Horizontal)
		{
			sSlotPosition.x = iPos;
			sSlotSize.x = iCellLength + (((int32)i < iExtra) ? 1 : 0);
			iPos += sSlotSize.x + miSpacing;
		}
		else
		{
			sSlotPosition.y = iPos;
			sSlotSize.y = iCellLength + (((int32)i < iExtra) ? 1 : 0);
			iPos += sSlotSize.y + miSpacing;
		}

		pComponent = maChildren.Get(i);
		LayoutComponent(pComponent, sSlotPosition, sSlotSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::LayoutComponent(Ptr<CComponent> pComponent, SInt32Vec2 sSlotPosition, SInt32Vec2 sSlotSize)
{
	SInt32Vec2	sSize;
	SInt32Vec2	sPosition;

	//A desired size of -1 on an axis means the component has no preference and takes the whole slot.
	sSize = pComponent->GetDesiredSize();
	if ((sSize.x < 0) || (meHorizontal == CSH_Stretch) || (meHorizontal == CSH_Unknown))
	{
		sSize.x = sSlotSize.x;
	}
	if ((sSize.y < 0) || (meVertical == CSV_Stretch) || (meVertical == CSV_Unknown))
	{
		sSize.y = sSlotSize.y;
	}

	sPosition.x = sSlotPosition.x + AlignComponentHorizontal(meHorizontal, sSize.x, sSlotSize.x);
	sPosition.y = sSlotPosition.y + AlignComponentVertical(meVertical, sSize.y, sSlotSize.y);

	pComponent->Layout(sPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFillContainer::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}

