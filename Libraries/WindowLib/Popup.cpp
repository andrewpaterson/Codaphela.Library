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
#include "Popup.h"


//////////////////////////////////////////////////////////////////////////
//
// A new popup has no insets and is drawn with nothing behind its content.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Init(Ptr<CWindow> pWindow, Ptr<CComponent> pContent, SInt32Vec2 sPosition, Ptr<CComponent> pOwner)
{
	PreInit();

	CContainer::Init(pWindow);
	//A popup is always the size it needs, however big the window is.
	SetSizedByContent(true);
	mpContent = pContent;
	mpOwner = pOwner;
	msOpenPosition = sPosition;
	msBackground = 0;
	msBorder = 0;
	mbBackground = false;
	mbBorder = false;
	mbOwnerPressKeepsOpen = true;
	mbPointerPasses = false;
	mpScrollPane = NULL;
	miMaximumHeight = -1;
	AddComponent(pContent);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Class(void)
{
	CContainer::Class();

	M_Pointer(mpContent);
	M_Pointer(mpOwner);
	U_2Int32(msOpenPosition);
	U_UInt32(msBackground);
	U_UInt32(msBorder);
	U_Bool(mbBackground);
	U_Bool(mbBorder);
	U_Bool(mbOwnerPressKeepsOpen);
	U_Bool(mbPointerPasses);
	M_Pointer(mpScrollPane);
	U_Int32(miMaximumHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The border, if there is one, is drawn in the outside pixel of the
// insets so they should be at least one pixel.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetLook(ARGB32 sBackground, ARGB32 sBorder, SInsets* psInsets)
{
	msBackground = sBackground;
	msBorder = sBorder;
	mbBackground = true;
	mbBorder = true;
	msInsets = *psInsets;

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetLook(ARGB32 sBackground, SInsets* psInsets)
{
	msBackground = sBackground;
	mbBackground = true;
	mbBorder = false;
	msInsets = *psInsets;

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetRequiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = mpContent->GetDesiredSize();
	if (sSize.x < 0)
	{
		sSize.x = 0;
	}
	if (sSize.y < 0)
	{
		sSize.y = 0;
	}
	if ((miMaximumHeight >= 0) && (sSize.y > miMaximumHeight))
	{
		sSize.y = miMaximumHeight;
	}
	AddInsets(&sSize);
	msRequiredSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The popup is no taller than iHeight (inside its insets); its content
// scrolls in a scroll pane in pTheme's look if it is taller.  -1 lets it
// be as tall as its content again.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetMaximumHeight(int32 iHeight, Ptr<CWindowTheme> pTheme)
{
	Ptr<CComponent>	pFocussed;

	miMaximumHeight = iHeight;
	if ((iHeight >= 0) && mpScrollPane.IsNull())
	{
		//The content keeps the keys if it has them (a menu opened with them), though it is taken out to go in the scroll pane.
		pFocussed = mpWindow->GetFocus()->GetFocussedComponent();
		if (pFocussed.IsNotNull() && !pFocussed->IsWithin(mpContent))
		{
			pFocussed = NULL;
		}
		RemoveComponent(mpContent);
		mpScrollPane = OMalloc<CScrollPane>(mpWindow, pTheme);
		mpScrollPane->SetPolicy(SBO_Horizontal, SPP_Never);
		mpScrollPane->SetContent(mpContent);
		AddComponent(mpScrollPane);
		if (pFocussed.IsNotNull())
		{
			mpWindow->GetFocus()->SetFocus(pFocussed);
		}
	}
	InvalidateLayout();
}


int32 CPopup::GetMaximumHeight(void) { return miMaximumHeight; }
Ptr<CScrollPane> CPopup::GetScrollPane(void) { return mpScrollPane; }


//////////////////////////////////////////////////////////////////////////
//
// Closed (CWindow::ClosePopup), the content is let go of, out of the
// scroll pane too, so it can be opened in another popup.
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::RemoveAllComponents(void)
{
	if (mpScrollPane.IsNotNull())
	{
		mpScrollPane->SetContent(NULL);
	}
	return CContainer::RemoveAllComponents();
}


//////////////////////////////////////////////////////////////////////////
//
// The area is the whole window.  The popup ignores where it would put it
// and goes where it was opened, kept inside the area.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sSize;
	SInt32Vec2	sPopupPosition;
	SInt32Vec2	sContentPosition;
	SInt32Vec2	sContentSize;

	sSize = GetDesiredSize();
	sPopupPosition = msOpenPosition;
	if (sPopupPosition.x + sSize.x > sPosition.x + sAreaSize.x)
	{
		sPopupPosition.x = sPosition.x + sAreaSize.x - sSize.x;
	}
	if (sPopupPosition.y + sSize.y > sPosition.y + sAreaSize.y)
	{
		sPopupPosition.y = sPosition.y + sAreaSize.y - sSize.y;
	}
	if (sPopupPosition.x < sPosition.x)
	{
		sPopupPosition.x = sPosition.x;
	}
	if (sPopupPosition.y < sPosition.y)
	{
		sPopupPosition.y = sPosition.y;
	}

	sContentPosition = sPopupPosition;
	sContentSize = sSize;
	InsetArea(&sContentPosition, &sContentSize);
	SetContainerBounds(sContentPosition, sContentSize);

	if (mpScrollPane.IsNotNull())
	{
		mpScrollPane->Layout(sContentPosition, sContentSize);
	}
	else
	{
		mpContent->Layout(sContentPosition, sContentSize);
	}

	SetActualBounds(sPopupPosition, sSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The background and border go straight onto the window's canvas, under
// the content.  A popup without a look has neither.
//
//////////////////////////////////////////////////////////////////////////
bool CPopup::Draw(void)
{
	Ptr<CCanvas>	pWindowCanvas;
	CInt32Rect		cRect;

	if (!mbBackground)
	{
		return DrawChildren();
	}

	pWindowCanvas = mpWindow->GetCanvas();
	cRect.Init(mcActualBounds.GetLeft(), mcActualBounds.GetTop(), mcActualBounds.GetRight() - 1, mcActualBounds.GetBottom() - 1);
	if ((cRect.GetRight() >= cRect.GetLeft()) && (cRect.GetBottom() >= cRect.GetTop()))
	{
		if (mbBorder)
		{
			pWindowCanvas->DrawBox(&cRect, true, msBorder);
			cRect.Init(cRect.GetLeft() + 1, cRect.GetTop() + 1, cRect.GetRight() - 1, cRect.GetBottom() - 1);
		}
		if ((cRect.GetRight() >= cRect.GetLeft()) && (cRect.GetBottom() >= cRect.GetTop()))
		{
			pWindowCanvas->DrawBox(&cRect, true, msBackground);
		}
	}
	return DrawChildren();
}


//////////////////////////////////////////////////////////////////////////
//
// A popup with a background draws it over all of its area.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea)
{
	if (!bDrawn || !mbBackground)
	{
		pcArea->Init(0, 0, 0, 0);
		return;
	}
	pcArea->Init(&mcActualBounds);
	if (bClipped)
	{
		pcArea->ClipTo(pcClip);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CPopup::GetContent(void) { return mpContent; }
Ptr<CComponent> CPopup::GetOwner(void) { return mpOwner; }
void CPopup::SetOwnerPressKeepsOpen(bool bKeepsOpen) { mbOwnerPressKeepsOpen = bKeepsOpen; }  //True, as it starts, for a small owner like a combo box: pressing it again toggles the popup itself.
bool CPopup::IsOwnerPressKeepsOpen(void) { return mbOwnerPressKeepsOpen; }
void CPopup::SetPointerPasses(bool bPasses) { mbPointerPasses = bPasses; }


//////////////////////////////////////////////////////////////////////////
//
// Nothing, if the pointer passes through the popup to what is under it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CPopup::FindComponentAt(int x, int y)
{
	if (mbPointerPasses)
	{
		return NULL;
	}
	return CContainer::FindComponentAt(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves the popup; it is placed there when the window is next laid out.
//
//////////////////////////////////////////////////////////////////////////
void CPopup::SetOpenPosition(SInt32Vec2 sPosition)
{
	msOpenPosition = sPosition;
	LayoutChanged();
}

