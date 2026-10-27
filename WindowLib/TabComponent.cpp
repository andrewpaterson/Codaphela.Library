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
#include "TabbedContainer.h"
#include "TabComponent.h"


#define TAB_SIDE_GAP		10	//Pixels from the tab's sides to its icon, name or cross.
#define TAB_GAP				6	//Between the icon, the name and the cross.
#define TAB_TOP_GAP			7
#define TAB_BOTTOM_GAP		4	//Above the underline.
#define TAB_UNDERLINE		3
#define TAB_ICON_SIZE		16


static EComponentInputCommand	gaeTabCommands[] = { CIC_ActivateComponent, CIC_ExecuteComponent, CIC_Space, CIC_Enter };


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
	mpTabbed = pTabbed;
	mpTheme = pTheme;
	mbSelected = false;
	mbMouseOver = false;
	mbOverClose = false;
	SetCanGetFocus(true);

	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(pTheme->GetTabBarColour());
	pBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pBackground, 0, 0, TT_NumColumns, 4);

	AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), TAB_SIDE_GAP, TAB_TOP_GAP), TT_LeftGap, 0);
	AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), TAB_SIDE_GAP, TAB_BOTTOM_GAP), TT_RightGap, 2);

	mpIcon = NULL;
	if (pIcon)
	{
		mpIcon = OMalloc<CIconComponent>(pWindow, pIcon, TAB_ICON_SIZE, TAB_ICON_SIZE);
		AddComponent(mpIcon, TT_Icon, 1);
		AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), TAB_GAP, 1), TT_IconGap, 1);
	}

	mpName = OMalloc<CLabel>(pWindow, pTheme->GetFont(), szName);
	AddComponent(mpName, TT_Name, 1);

	mpClose = NULL;
	if (bClosable && pTheme->GetTabClose())
	{
		mpClose = OMalloc<CIconComponent>(pWindow, pTheme->GetTabClose(), TAB_ICON_SIZE, TAB_ICON_SIZE);
		AddComponent(OMalloc<CIconComponent>(pWindow, Ptr<CImageCel>(), TAB_GAP, 1), TT_CloseGap, 1);
		AddComponent(mpClose, TT_Close, 1);
	}

	mpUnderlineDraw = OMalloc<CClearCanvasDraw>(pTheme->GetTabBarColour());
	pUnderline = OMalloc<CCanvas>(pWindow, mpUnderlineDraw);
	pUnderline->SetDesiredSize(-1, TAB_UNDERLINE);
	pUnderline->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pUnderline, 0, 3, TT_NumColumns, 1);

	AddInputActions(gaeTabCommands, sizeof(gaeTabCommands) / sizeof(EComponentInputCommand), this, &CTabComponent::CommandAction);

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
	U_Bool(mbMouseOver);
	U_Bool(mbOverClose);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::Free(void)
{
	CGridContainer::Free();
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
// A tab is as big as its parts need, like a button.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTabComponent::GetDesiredSize(void)
{
	return GetDesiredOrRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
// The tab is hit as a whole, not its parts, so it hears the pointer move
// over its cross.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTabComponent::FindComponentAt(int x, int y)
{
	if (IsPointIn(x, y))
	{
		return this;
	}
	return NULL;
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
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::MovedInto(void)
{
	mbMouseOver = true;
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::MovedOutof(void)
{
	mbMouseOver = false;
	mbOverClose = false;
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
// The input actions of gaeTabCommands.  Closing removes the tab from its
// container, so nothing of it is used after.  A release on the cross isn't
// a click, for double clicks.
//
//////////////////////////////////////////////////////////////////////////
void CTabComponent::CommandAction(SComponentInputEvent* psEvent)
{
	if (psEvent->eCommand == CIC_ExecuteComponent)
	{
		mpTabbed->TabReleased(this, psEvent->x, psEvent->y, !IsInClose(psEvent->x, psEvent->y));
	}
	else if ((psEvent->eCommand == CIC_ActivateComponent) && IsInClose(psEvent->x, psEvent->y))
	{
		mpTabbed->RemoveTab(this);
	}
	else
	{
		mpTabbed->Select(this);
	}
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
	mpBackgroundDraw->SetColour(mbMouseOver ? mpTheme->GetTabHoverColour() : mpTheme->GetTabBarColour());
	mpUnderlineDraw->SetColour(mbSelected ? mpTheme->GetTabUnderlineColour() : (mbMouseOver ? mpTheme->GetTabHoverColour() : mpTheme->GetTabBarColour()));
	if (mpClose)
	{
		mpClose->SetCel(mbOverClose ? mpTheme->GetTabCloseHover() : mpTheme->GetTabClose());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTabComponent::IsSelected(void) { return mbSelected; }
bool CTabComponent::IsClosable(void) { return mpClose.IsNotNull(); }
bool CTabComponent::IsMouseOver(void) { return mbMouseOver; }
char* CTabComponent::GetText(void) { return mpName->GetText(); }
void CTabComponent::SetText(const char* szName) { mpName->SetText(szName); }
Ptr<CIconComponent> CTabComponent::GetIcon(void) { return mpIcon; }
Ptr<CIconComponent> CTabComponent::GetClose(void) { return mpClose; }
