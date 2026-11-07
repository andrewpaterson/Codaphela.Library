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
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "TabbedContainer.h"
#include "Window.h"
#include "TabComponent.h"


#define TAB_DRAG_DISTANCE	6	//Pixels a pressed tab is pulled before it is dragged.


//The tab's columns and rows.  The background spans them all, under everything else.
enum ETabTrack
{
	TT_LeftGap,
	TT_Icon,
	TT_IconGap,
	TT_Name,
	TT_CloseGap,
	TT_Close,
	TT_RightGap,
	TT_NumColumns,
};


//////////////////////////////////////////////////////////////////////////
//
// pIcon can be NULL.  A tab is only closable if the theme has a cross.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CTabbedContainer> pTabbed, const char* szName, Ptr<CImageCel> pIcon, bool bClosable)
{
	Ptr<CCanvas>	pBackground;
	Ptr<CCanvas>	pUnderline;

	PreInit();

	CGridContainer::Init(pWindow);
	SetSizedByContent(true);
	//Not its parts, so it hears the pointer move over its cross.
	SetHitAsWhole(true);
	mpTabbed = pTabbed;
	mpTheme = pTheme;
	mbSelected = false;
	mbOverClose = false;
	mbPressed = false;
	msPressed.Init(0, 0);
	SetCanGetFocus(true);

	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_TabBar));
	pBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pBackground, 0, 0, TT_NumColumns, 4);

	AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), pTheme->GetMetric(TM_TabSideGap), pTheme->GetMetric(TM_TabTopGap)), TT_LeftGap, 0);
	AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), pTheme->GetMetric(TM_TabSideGap), pTheme->GetMetric(TM_TabBottomGap)), TT_RightGap, 2);

	mpIcon = NULL;
	if (pIcon)
	{
		mpIcon = OMalloc<CIconComponent>(pWindow, pIcon, pTheme->GetMetric(TM_TabIconSize), pTheme->GetMetric(TM_TabIconSize));
		AddComponent(mpIcon, TT_Icon, 1);
		AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), pTheme->GetMetric(TM_TabGap), 1), TT_IconGap, 1);
	}

	mpName = OMalloc<CLabel>(pWindow, pTheme->GetFont(), szName);
	AddComponent(mpName, TT_Name, 1);

	mpClose = NULL;
	if (bClosable && pTheme->IsImageSet(TI_TabClose))
	{
		mpClose = OMalloc<CIconComponent>(pWindow, pTheme->GetImage(TI_TabClose), pTheme->GetMetric(TM_TabIconSize), pTheme->GetMetric(TM_TabIconSize));
		AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), pTheme->GetMetric(TM_TabGap), 1), TT_CloseGap, 1);
		AddComponent(mpClose, TT_Close, 1);
	}

	mpUnderlineDraw = OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_TabBar));
	pUnderline = OMalloc<CCanvas>(pWindow, mpUnderlineDraw);
	pUnderline->SetDesiredSize(-1, pTheme->GetMetric(TM_TabUnderline));
	pUnderline->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pUnderline, 0, 3, TT_NumColumns, 1);

	AddInputAction(CIC_ActivateComponent, this, &CTabComponent::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CTabComponent::ExecuteComponent);
	AddInputAction(CIC_Space, this, &CTabComponent::Select);
	AddInputAction(CIC_Enter, this, &CTabComponent::Select);
	AddInputAction(CIC_ContDragSelect, this, &CTabComponent::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CTabComponent::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpTabbed);
	M_Pointer(mpTheme);
	M_Pointer(mpBackgroundDraw);
	M_Pointer(mpUnderlineDraw);
	M_Pointer(mpIcon);
	M_Pointer(mpName);
	M_Pointer(mpClose);
	U_Bool(mbSelected);
	U_Bool(mbOverClose);
	U_Bool(mbPressed);
	U_2Int32(msPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabComponent::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabComponent::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Called by the tabbed container, which shows the page.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::SetSelected(bool bSelected)
{
	mbSelected = bSelected;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// Lighter under the pointer.  Leaving the tab leaves its cross too.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::FocusOrMouseOverChanged(void)
{
	if (!IsMouseOver())
	{
		mbOverClose = false;
	}
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the tab's top left.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::MovedOver(int x, int y)
{
	bool	bOverClose;

	bOverClose = IsInClose(x, y);
	if (bOverClose != mbOverClose)
	{
		mbOverClose = bOverClose;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing the cross closes the tab, which removes it from its container,
// so nothing of it is used after.  Pressing anywhere else selects it.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::ActivateComponent(int x, int y)
{
	if (IsInClose(x, y))
	{
		mpTabbed->TabClosePressed(this);
	}
	else
	{
		Select();
		if (mpTabbed->IsTabDraggable())
		{
			mbPressed = true;
			msPressed.Init(x, y);
			mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pulled TAB_DRAG_DISTANCE pixels away from where it was pressed the tab
// is dragged; the tabbed container's drag listener takes the rest of the
// drag.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::ContDragSelect(int x, int y)
{
	int		iDistance;

	if (!mbPressed)
	{
		return;
	}
	iDistance = abs(x - msPressed.x) + abs(y - msPressed.y);
	if (iDistance >= TAB_DRAG_DISTANCE)
	{
		mbPressed = false;
		mpTabbed->TabDragged(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::StopDragSelect(int x, int y)
{
	mbPressed = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A release on the cross isn't a click, for double clicks.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mbPressed = false;
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
	mpTabbed->TabReleased(this, x, y, !IsInClose(x, y));
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the tab's page, as space and enter do while it has focus.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::Select(void)
{
	mpTabbed->Select(this);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the tab's top left.
//
//////////////////////////////////////////////////////////////////////////
bool CTabComponent::IsInClose(int x, int y)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;
	int			iLeft;
	int			iTop;

	if (mpClose.IsNull())
	{
		return false;
	}

	sPosition = mpClose->GetPosition();
	sSize = mpClose->GetActualSize();
	iLeft = sPosition.x - GetPosition().x;
	iTop = sPosition.y - GetPosition().y;
	return (x >= iLeft) && (x < iLeft + sSize.x) && (y >= iTop) && (y < iTop + sSize.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::UpdateLook(void)
{
	mpBackgroundDraw->SetColour(IsMouseOver() ? mpTheme->GetColour(TC_TabHover) : mpTheme->GetColour(TC_TabBar));
	mpUnderlineDraw->SetColour(mbSelected ? mpTheme->GetColour(TC_TabUnderline) : (IsMouseOver() ? mpTheme->GetColour(TC_TabHover) : mpTheme->GetColour(TC_TabBar)));
	if (mpClose)
	{
		mpClose->SetCel(mpTheme->GetImage(mbOverClose ? TI_TabCloseHover : TI_TabClose));
	}

	//The background's and underline's draws changed, which their canvases can't see.
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The name is drawn in pFont, or the theme's font if it is NULL.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::SetTextFont(Ptr<CFont> pFont)
{
	if (pFont.IsNull())
	{
		pFont = mpTheme->GetFont();
	}
	if (mpName->GetFont() != pFont)
	{
		mpName->SetFont(pFont);
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabComponent::IsSelected(void) { return mbSelected; }
bool CTabComponent::IsClosable(void) { return mpClose.IsNotNull(); }
char* CTabComponent::GetText(void) { return mpName->GetText(); }
void CTabComponent::SetText(const char* szName) { mpName->SetText(szName); InvalidateLayout(); }
Ptr<CIconComponent> CTabComponent::GetIcon(void) { return mpIcon; }
Ptr<CIconComponent> CTabComponent::GetClose(void) { return mpClose; }
