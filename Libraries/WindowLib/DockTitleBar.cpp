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
#include "Window.h"
#include "DockManager.h"
#include "DockTitleBar.h"


#define DOCK_TITLE_GAP			8
#define DOCK_TITLE_BUTTON_WIDTH	22


//////////////////////////////////////////////////////////////////////////
//
// The bar's buttons are shown as asked; ShowButton changes them after.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, const char* szTitle, bool bMenu, bool bPin, bool bClose)
{
	Ptr<CCanvas>	pBackground;
	int				i;

	PreInit();

	CGridContainer::Init(pWindow);
	mpManager = pManager;
	mbActive = false;
	mbPinned = true;
	mbPressed = false;
	msPressed.Init(0, 0);
	mePressedButton = DTB_None;
	msDoubleClick.Init();
	mcDragged.Init();
	mcDoubleClicked.Init();
	for (i = 0; i < DTB_NumButtons; i++)
	{
		macButtonPressed[i].Init();
	}
	SetHitAsWhole(true);
	SetCanGetFocus(true);
	SetDesiredSize(-1, DOCK_TITLE_BAR_HEIGHT);
	SetAlignment(CSH_Stretch, CSV_Stretch);

	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockTitleBar));
	pBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pBackground, 0, 0, DTB_NumButtons + 3, 1);
	AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), DOCK_TITLE_GAP, 1), 0, 0);

	mpTitle = OMalloc<CLabel>(pWindow, pManager->GetFont(), szTitle);
	mpTitle->SetAlignment(CSH_Left, CSV_Center);
	AddComponent(mpTitle, 1, 0, 1, 1, 1, 0);

	mapButtons[DTB_Menu] = OMalloc<CIconComponent>(pWindow, pManager->GetTheme()->GetImage(TI_DownChevron), DOCK_TITLE_BUTTON_WIDTH, DOCK_TITLE_BAR_HEIGHT);
	mapButtons[DTB_Pin] = OMalloc<CIconComponent>(pWindow, pManager->GetTheme()->GetImage(TI_Pin), DOCK_TITLE_BUTTON_WIDTH, DOCK_TITLE_BAR_HEIGHT);
	mapButtons[DTB_Close] = OMalloc<CIconComponent>(pWindow, pManager->GetTheme()->GetImage(TI_Close), DOCK_TITLE_BUTTON_WIDTH, DOCK_TITLE_BAR_HEIGHT);
	mabShown[DTB_Menu] = bMenu;
	mabShown[DTB_Pin] = bPin;
	mabShown[DTB_Close] = bClose;
	PlaceButtons();

	AddInputAction(CIC_ActivateComponent, this, &CDockTitleBar::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CDockTitleBar::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CDockTitleBar::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CDockTitleBar::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpManager);
	M_Pointer(mpBackgroundDraw);
	M_Pointer(mpTitle);
	M_Pointer(mapButtons[DTB_Menu]);
	M_Pointer(mapButtons[DTB_Pin]);
	M_Pointer(mapButtons[DTB_Close]);
	U_BoolArray(mabShown, DTB_NumButtons);
	U_Bool(mbActive);
	U_Bool(mbPinned);
	U_Bool(mbPressed);
	U_2Int32(msPressed);
	U_Enum(mePressedButton);
	U_Data(SDoubleClick, msDoubleClick);
	U_Unknown(CComponentEvent<CDockTitleBar>, mcDragged);
	U_Unknown(CComponentEvent<CDockTitleBar>, mcDoubleClicked);
	U_Unknown(CComponentEvent<CDockTitleBar>[DTB_NumButtons], macButtonPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::Free(void)
{
	int		i;

	for (i = 0; i < DTB_NumButtons; i++)
	{
		macButtonPressed[i].Kill();
	}
	mcDoubleClicked.Kill();
	mcDragged.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockTitleBar::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockTitleBar::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The shown buttons go right of the title, in order, then a gap.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::PlaceButtons(void)
{
	int		i;
	int		iColumn;

	for (i = 0; i < DTB_NumButtons; i++)
	{
		RemoveComponent(mapButtons[i]);
	}
	iColumn = 2;
	for (i = 0; i < DTB_NumButtons; i++)
	{
		if (mabShown[i])
		{
			AddComponent(mapButtons[i], iColumn, 0);
			iColumn++;
		}
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::ShowButton(EDockTitleButton eButton, bool bShow)
{
	if (mabShown[eButton] != bShow)
	{
		mabShown[eButton] = bShow;
		PlaceButtons();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::SetTitle(const char* szTitle)
{
	mpTitle->SetText(szTitle);
}


//////////////////////////////////////////////////////////////////////////
//
// The active window's title bar is highlighted.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::SetActive(bool bActive)
{
	if (mbActive != bActive)
	{
		mbActive = bActive;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pin is upright while the window is docked, on its side while it
// auto hides.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::SetPinned(bool bPinned)
{
	if (mbPinned != bPinned)
	{
		mbPinned = bPinned;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::UpdateLook(void)
{
	mpBackgroundDraw->SetColour(mpManager->GetColour(mbActive ? TC_DockActiveTitleBar : TC_DockTitleBar));
	mapButtons[DTB_Pin]->SetCel(mpManager->GetTheme()->GetImage(mbPinned ? TI_Pin : TI_Unpin));
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// The button shown at x, y relative to the bar, DTB_None for none.
//
//////////////////////////////////////////////////////////////////////////
EDockTitleButton CDockTitleBar::FindButton(int x, int y)
{
	int			i;
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	for (i = 0; i < DTB_NumButtons; i++)
	{
		if (mabShown[i] && mapButtons[i]->IsPointIn(sPosition.x + x, sPosition.y + y))
		{
			return (EDockTitleButton)i;
		}
	}
	return DTB_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A press on a button is remembered until the release; anywhere else it
// may start a drag.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::ActivateComponent(int x, int y)
{
	mePressedButton = FindButton(x, y);
	if (mePressedButton != DTB_None)
	{
		msDoubleClick.Forget();
		return;
	}
	if (mcDragged.NumCalls() > 0)
	{
		mbPressed = true;
		msPressed.Init(x, y);
		mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A release on the button that was pressed calls its event.  A release
// off the buttons may be a double click.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::ExecuteComponent(int x, int y)
{
	EDockTitleButton	eButton;
	Ptr<CDockTitleBar>	pThis;

	pThis = this;
	if (mbPressed)
	{
		mbPressed = false;
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
	eButton = FindButton(x, y);
	if (eButton != DTB_None)
	{
		if (eButton == mePressedButton)
		{
			mePressedButton = DTB_None;
			macButtonPressed[eButton].Call(&pThis);
		}
		return;
	}
	mePressedButton = DTB_None;
	if (msDoubleClick.Released(0, x, y))
	{
		mcDoubleClicked.Call(&pThis);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::ContDragSelect(int x, int y)
{
	int		iDistance;

	if (!mbPressed)
	{
		return;
	}
	iDistance = abs(x - msPressed.x) + abs(y - msPressed.y);
	if (iDistance >= DOCK_DRAG_DISTANCE)
	{
		mbPressed = false;
		msDoubleClick.Forget();
		mcDragged.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::StopDragSelect(int x, int y)
{
	mbPressed = false;
}


char*				CDockTitleBar::GetTitle(void) { return mpTitle->GetText(); }
bool				CDockTitleBar::IsActive(void) { return mbActive; }
bool				CDockTitleBar::IsPinned(void) { return mbPinned; }
Ptr<CIconComponent>	CDockTitleBar::GetButton(EDockTitleButton eButton) { return mapButtons[eButton]; }
CComponentEvent<CDockTitleBar>*	CDockTitleBar::GetDragEvent(void) { return &mcDragged; }
CComponentEvent<CDockTitleBar>*	CDockTitleBar::GetDoubleClickEvent(void) { return &mcDoubleClicked; }
CComponentEvent<CDockTitleBar>*	CDockTitleBar::GetButtonEvent(EDockTitleButton eButton) { return &macButtonPressed[eButton]; }


//////////////////////////////////////////////////////////////////////////
//
// Every function pvListener added to the bar's events is taken off, as
// the listener goes.
//
//////////////////////////////////////////////////////////////////////////
void CDockTitleBar::RemoveListener(void* pvListener)
{
	int		i;

	mcDragged.Remove(pvListener);
	mcDoubleClicked.Remove(pvListener);
	for (i = 0; i < DTB_NumButtons; i++)
	{
		macButtonPressed[i].Remove(pvListener);
	}
}

