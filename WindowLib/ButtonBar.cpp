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
#include "StandardLib/ClassDefines.h"
#include "ButtonBar.h"


//////////////////////////////////////////////////////////////////////////
//
// The bar is 8 in from each edge with 8 between buttons, aligned right.
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	CContainer::Init(pWindow);
	SetInsets(8, 8, 8, 8);
	miGap = 8;
	meButtonAlignment = CSH_Right;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::Class(void)
{
	CContainer::Class();

	U_Data(SInsets, msInsets);
	U_Int32(miGap);
	U_Enum(meButtonAlignment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::Free(void)
{
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButtonBar::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButtonBar::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::SetInsets(SInsets* psInsets)
{
	memcpy(&msInsets, psInsets, sizeof(SInsets));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::SetInsets(int iTop, int iRight, int iBottom, int iLeft)
{
	msInsets.maiWidths[BEI_Top] = iTop;
	msInsets.maiWidths[BEI_Right] = iRight;
	msInsets.maiWidths[BEI_Bottom] = iBottom;
	msInsets.maiWidths[BEI_Left] = iLeft;
}


//////////////////////////////////////////////////////////////////////////
//
// Like a button the bar keeps the size its buttons need unless a desired
// size is set on an axis.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CButtonBar::GetDesiredSize(void)
{
	SInt32Vec2	sSize;
	SInt32Vec2	sRequired;

	sSize = CContainer::GetDesiredSize();
	if ((sSize.x < 0) || (sSize.y < 0))
	{
		SetRequiredSize();
		sRequired = GetRequiredSize();
		if (sSize.x < 0)
		{
			sSize.x = sRequired.x;
		}
		if (sSize.y < 0)
		{
			sSize.y = sRequired.y;
		}
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The row of buttons and the gaps between them, plus the insets.
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::SetRequiredSize(void)
{
	size				i;
	size				uiSize;
	Ptr<CComponent>		pComponent;
	SInt32Vec2			sSize;
	int32				iHeight;

	iHeight = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.y > iHeight)
		{
			iHeight = sSize.y;
		}
	}

	msRequiredSize.x = GetRowWidth() + msInsets.GetLeftAndRight();
	msRequiredSize.y = iHeight + msInsets.GetTopAndBottom();
}


//////////////////////////////////////////////////////////////////////////
//
// The row is placed in the area inside the insets by the button alignment.
// Each button gets its own width and the row's height to sit in.
//
//////////////////////////////////////////////////////////////////////////
void CButtonBar::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size				i;
	size				uiSize;
	SInt32Vec2			sOrigin;
	SInt32Vec2			sArea;
	SInt32Vec2			sButtonPosition;
	SInt32Vec2			sButtonArea;
	SInt32Vec2			sSize;
	Ptr<CComponent>		pComponent;

	sOrigin = sPosition;
	sArea = sAreaSize;
	sOrigin.x += msInsets.maiWidths[BEI_Left];
	sOrigin.y += msInsets.maiWidths[BEI_Top];
	sArea.x -= msInsets.GetLeftAndRight();
	sArea.y -= msInsets.GetTopAndBottom();
	if (sArea.x < 0)
	{
		sArea.x = 0;
	}
	if (sArea.y < 0)
	{
		sArea.y = 0;
	}

	SetContainerBounds(sOrigin, sArea);

	sButtonPosition.x = sOrigin.x + AlignComponentHorizontal(meButtonAlignment, GetRowWidth(), sArea.x);
	sButtonPosition.y = sOrigin.y;

	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.x < 0)
		{
			sSize.x = 0;
		}
		sButtonArea.Init(sSize.x, sArea.y);
		LayoutComponentInArea(pComponent, sButtonPosition, sButtonArea);
		sButtonPosition.x += sSize.x + miGap;
	}

	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The buttons' widths and the gaps between them.  A button that is
// variable sized horizontally has no width of its own.
//
//////////////////////////////////////////////////////////////////////////
int32 CButtonBar::GetRowWidth(void)
{
	size				i;
	size				uiSize;
	Ptr<CComponent>		pComponent;
	SInt32Vec2			sSize;
	int32				iWidth;

	iWidth = 0;
	uiSize = maChildren.Size();
	for (i = 0; i < uiSize; i++)
	{
		pComponent = maChildren.Get(i);
		sSize = pComponent->GetDesiredSize();
		if (sSize.x > 0)
		{
			iWidth += sSize.x;
		}
		if (i > 0)
		{
			iWidth += miGap;
		}
	}
	return iWidth;
}


void CButtonBar::SetGap(int32 iGap) { miGap = iGap; }
int32 CButtonBar::GetGap(void) { return miGap; }
void CButtonBar::SetButtonAlignment(EContainerStyleHorizontal eAlignment) { meButtonAlignment = eAlignment; }
EContainerStyleHorizontal CButtonBar::GetButtonAlignment(void) { return meButtonAlignment; }

