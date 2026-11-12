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
#include "StandardLib/Objects.h"
#include "Window.h"
#include "SplitContainer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::Init(Ptr<CWindow> pWindow, CSplitContainer* pcSplit, bool bHorizontal, ARGB32 sColour)
{
	PreInit();

	CCanvas::Init(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	mpcSplit = pcSplit;
	mbDragging = false;
	miPressed = 0;
	SetCanGetFocus(true);
	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetCursor(bHorizontal ? NC_SizeWestEast : NC_SizeNorthSouth);
	AddInputAction(CIC_ActivateComponent, this, &CSplitSash::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CSplitSash::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CSplitSash::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CSplitSash::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::Class(void)
{
	CCanvas::Class();

	U_Pointer(mpcSplit);
	U_Bool(mbDragging);
	U_Int32(miPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CSplitSash::Save(CObjectWriter* pcFile) { return false; }
bool CSplitSash::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::ActivateComponent(int x, int y)
{
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	miPressed = mpcSplit->IsHorizontal() ? sPosition.x + x : sPosition.y + y;
	mbDragging = true;
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::ExecuteComponent(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The split moves the sash by how far the pointer has come since it was
// pressed or last moved it.  x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::ContDragSelect(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iAlong;

	if (!mbDragging)
	{
		return;
	}
	sPosition = GetPosition();
	iAlong = mpcSplit->IsHorizontal() ? sPosition.x + x : sPosition.y + y;
	if (iAlong != miPressed)
	{
		mpcSplit->SashMoved(iAlong - miPressed);
		miPressed = iAlong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitSash::StopDragSelect(int x, int y)
{
	mbDragging = false;
}


bool CSplitSash::IsDragging(void) { return mbDragging; }


//////////////////////////////////////////////////////////////////////////
//
// The sash is the theme's dock sash: its size and its colour.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bHorizontal)
{
	PreInit();

	CContainer::Init(pWindow);
	mbHorizontal = bHorizontal;
	mbSecondShown = true;
	miShare = SPLIT_CONTAINER_WHOLE / 2;
	miSashSize = pTheme->GetMetric(TM_DockSashSize);
	miFirstSize = 0;
	miAlong = 0;
	mcShareChanged.Init();
	mpSash = OMalloc<CSplitSash>(pWindow, this, bHorizontal, pTheme->GetColour(TC_DockSash));

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::Class(void)
{
	CContainer::Class();

	M_Pointer(mpFirst);
	M_Pointer(mpSecond);
	M_Pointer(mpSash);
	U_Bool(mbHorizontal);
	U_Bool(mbSecondShown);
	U_Int32(miShare);
	U_Int32(miSashSize);
	U_Int32(miFirstSize);
	U_Int32(miAlong);
	U_Unknown(CComponentEvent<CSplitContainer>, mcShareChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::Free(void)
{
	mcShareChanged.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CSplitContainer::Save(CObjectWriter* pcFile) { return false; }
bool CSplitContainer::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// The first (left or top) and second components, in place of any it had.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::SetComponents(Ptr<CComponent> pFirst, Ptr<CComponent> pSecond)
{
	RemoveAllComponents();
	mpFirst = pFirst;
	mpSecond = pSecond;
	CContainer::AddComponent(mpFirst);
	CContainer::AddComponent(mpSash);
	CContainer::AddComponent(mpSecond);
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CSplitContainer::AddComponent(Ptr<CComponent> pComponent)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Hidden, the second isn't drawn or laid out and the first has all the
// room.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::SetSecondShown(bool bShown)
{
	if (mbSecondShown != bShown)
	{
		mbSecondShown = bShown;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first's share of the room, out of SPLIT_CONTAINER_WHOLE.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::SetShare(int32 iShare)
{
	if (iShare < 0)
	{
		iShare = 0;
	}
	if (iShare > SPLIT_CONTAINER_WHOLE)
	{
		iShare = SPLIT_CONTAINER_WHOLE;
	}
	if (miShare != iShare)
	{
		miShare = iShare;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The sash was dragged iDelta along the split: the first is that much
// bigger, within what both sides must keep, and the share is what it now
// has of the room.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::SashMoved(int32 iDelta)
{
	int32	iRoom;
	int32	iSize;
	int32	iMinimum;

	iRoom = miAlong - miSashSize;
	if (!mbSecondShown || (iRoom <= 0))
	{
		return;
	}
	iMinimum = (iRoom >= 2 * SPLIT_CONTAINER_MINIMUM) ? SPLIT_CONTAINER_MINIMUM : iRoom / 2;
	iSize = miFirstSize + iDelta;
	if (iSize < iMinimum)
	{
		iSize = iMinimum;
	}
	if (iSize > iRoom - iMinimum)
	{
		iSize = iRoom - iMinimum;
	}
	if (iSize != miFirstSize)
	{
		miShare = (int32)(((int64)iSize * SPLIT_CONTAINER_WHOLE + iRoom / 2) / iRoom);
		miFirstSize = iSize;
		InvalidateLayout();
		mcShareChanged.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How much of iAlong the first has: its share of what the sash leaves,
// neither side less than the minimum while there is room.
//
//////////////////////////////////////////////////////////////////////////
int32 CSplitContainer::WorkOutFirstSize(int32 iAlong)
{
	int32	iRoom;
	int32	iSize;
	int32	iMinimum;

	if (!mbSecondShown)
	{
		return iAlong;
	}
	iRoom = iAlong - miSashSize;
	if (iRoom <= 0)
	{
		return 0;
	}
	iMinimum = (iRoom >= 2 * SPLIT_CONTAINER_MINIMUM) ? SPLIT_CONTAINER_MINIMUM : iRoom / 2;
	iSize = (int32)(((int64)iRoom * miShare) / SPLIT_CONTAINER_WHOLE);
	if (iSize < iMinimum)
	{
		iSize = iMinimum;
	}
	if (iSize > iRoom - iMinimum)
	{
		iSize = iRoom - iMinimum;
	}
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Each side's minimum and the sash along the split; nothing across it.
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::SetRequiredSize(void)
{
	SInt32Vec2	sSize;
	int32		iAlong;

	iAlong = mbSecondShown ? 2 * SPLIT_CONTAINER_MINIMUM + miSashSize : SPLIT_CONTAINER_MINIMUM;
	sSize.Init(mbHorizontal ? iAlong : 0, mbHorizontal ? 0 : iAlong);
	AddInsets(&sSize);
	msRequiredSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sOrigin;
	SInt32Vec2	sArea;
	SInt32Vec2	sAt;
	SInt32Vec2	sSize;
	int32		iSecond;

	sOrigin = sPosition;
	sArea = sAreaSize;
	InsetArea(&sOrigin, &sArea);
	SetContainerBounds(sOrigin, sArea);

	miAlong = mbHorizontal ? sArea.x : sArea.y;
	miFirstSize = WorkOutFirstSize(miAlong);
	if (mpFirst.IsNotNull())
	{
		sSize.Init(mbHorizontal ? miFirstSize : sArea.x, mbHorizontal ? sArea.y : miFirstSize);
		mpFirst->Layout(sOrigin, sSize);
	}
	if (mbSecondShown)
	{
		sAt.Init(sOrigin.x + (mbHorizontal ? miFirstSize : 0), sOrigin.y + (mbHorizontal ? 0 : miFirstSize));
		sSize.Init(mbHorizontal ? miSashSize : sArea.x, mbHorizontal ? sArea.y : miSashSize);
		mpSash->Layout(sAt, sSize);

		iSecond = miAlong - miFirstSize - miSashSize;
		if (iSecond < 0)
		{
			iSecond = 0;
		}
		if (mpSecond.IsNotNull())
		{
			sAt.Init(sOrigin.x + (mbHorizontal ? miFirstSize + miSashSize : 0), sOrigin.y + (mbHorizontal ? 0 : miFirstSize + miSashSize));
			sSize.Init(mbHorizontal ? iSecond : sArea.x, mbHorizontal ? sArea.y : iSecond);
			mpSecond->Layout(sAt, sSize);
		}
	}
	else
	{
		//Hidden, they are nowhere for the pointer to find.
		sSize.Init(0, 0);
		mpSash->Layout(sOrigin, sSize);
		if (mpSecond.IsNotNull())
		{
			mpSecond->Layout(sOrigin, sSize);
		}
	}
	SetActualBounds(sPosition, sAreaSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A hidden second, and the sash with it, aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CSplitContainer::IsChildDrawn(Ptr<CComponent> pChild)
{
	return mbSecondShown || ((pChild != mpSecond) && (pChild != mpSash));
}


Ptr<CComponent> CSplitContainer::GetFirst(void) { return mpFirst; }
Ptr<CComponent> CSplitContainer::GetSecond(void) { return mpSecond; }
Ptr<CSplitSash> CSplitContainer::GetSash(void) { return mpSash; }
bool CSplitContainer::IsHorizontal(void) { return mbHorizontal; }
bool CSplitContainer::IsSecondShown(void) { return mbSecondShown; }
int32 CSplitContainer::GetShare(void) { return miShare; }
int32 CSplitContainer::GetFirstSize(void) { return miFirstSize; }
CComponentEvent<CSplitContainer>* CSplitContainer::GetShareEvent(void) { return &mcShareChanged; }

