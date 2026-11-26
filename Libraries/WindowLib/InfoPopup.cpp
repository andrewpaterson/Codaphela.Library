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
#include "InfoPopup.h"


//////////////////////////////////////////////////////////////////////////
//
// Its text is in pFamily's faces: a code family for code.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontFamily> pFamily)
{
	PreInit();
	mpWindow = pWindow;
	mpTheme = pTheme;
	msPoint.Init(0, 0);
	mbAbove = false;
	miMaximumHeight = -1;
	mbTakesPointer = false;
	mcPressed.Init();
	mpText = OMalloc<CTextArea>(pWindow, pTheme, pFamily, "");
	mpText->SetEditable(false);
	mpText->SetSelectable(false);
	mpText->SetCanGetFocus(false);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpTheme);
	M_Pointer(mpText);
	M_Pointer(mpPopup);
	U_Data(SInt32Vec2, msPoint);
	U_Bool(mbAbove);
	U_Int32(miMaximumHeight);
	U_Bool(mbTakesPointer);
	U_Unknown(CInfoPopupPressEvent, mcPressed);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running, so it
// takes itself off its popup, which is closed.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::Free(void)
{
	if (mpPopup.IsNotNull())
	{
		mpPopup->GetClosedEvent()->Remove(this);
		Closed();
		if (mpWindow.IsNotNull())
		{
			mpWindow->CloseOnePopup(mpText);
		}
	}
	mcPressed.Kill();
}


bool CInfoPopup::Save(CObjectWriter* pcFile) { return false; }
bool CInfoPopup::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// What it shows, with no spans: they are added after.  Shown already, it
// is laid out again where it was shown, as its size may have changed.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::SetText(const char* szText)
{
	mpText->ClearSpans();
	mpText->SetText(szText);
	if (mpPopup.IsNotNull())
	{
		Show(msPoint, mbAbove);
	}
}


const char* CInfoPopup::GetText(void) { return mpText->GetText(); }
bool CInfoPopup::AddSpan(size uiStart, size uiLength, ARGB32 sColour) { return mpText->AddSpan(uiStart, uiLength, sColour); }
bool CInfoPopup::AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride) { return mpText->AddSpan(uiStart, uiLength, psOverride); }


//////////////////////////////////////////////////////////////////////////
//
// Shown with its top left a little under sPoint, in the window's client
// space, or with bAbove its bottom left a little over it; kept inside the
// window where that would put it past the right or the bottom, and under
// the point where there is no room over it.  Shown already, it moves.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::Show(SInt32Vec2 sPoint, bool bAbove)
{
	SInsets		sInsets;
	SInt32Vec2	sSize;
	SInt32Vec2	sWindow;
	SInt32Vec2	sPosition;
	int32		iPaddingX;
	int32		iPaddingY;

	msPoint = sPoint;
	mbAbove = bAbove;
	iPaddingX = mpTheme->GetMetric(TM_ToolTipPaddingX);
	iPaddingY = mpTheme->GetMetric(TM_ToolTipPaddingY);
	sSize = mpText->GetRequiredSize();
	if ((miMaximumHeight >= 0) && (sSize.y > miMaximumHeight))
	{
		sSize.y = miMaximumHeight;
	}
	sSize.x += 2 * (iPaddingX + 1);
	sSize.y += 2 * (iPaddingY + 1);
	sWindow = mpWindow->GetCanvas()->GetActualSize();

	sPosition.Init(sPoint.x, sPoint.y + INFO_POPUP_GAP);
	if (bAbove && (sPoint.y - INFO_POPUP_GAP - sSize.y >= 0))
	{
		sPosition.y = sPoint.y - INFO_POPUP_GAP - sSize.y;
	}
	else if ((sPosition.y + sSize.y > sWindow.y) && (sPoint.y - INFO_POPUP_GAP - sSize.y >= 0))
	{
		sPosition.y = sPoint.y - INFO_POPUP_GAP - sSize.y;
	}
	if (sPosition.x + sSize.x > sWindow.x)
	{
		sPosition.x = (sWindow.x > sSize.x) ? sWindow.x - sSize.x : 0;
	}

	if (mpPopup.IsNotNull())
	{
		mpPopup->SetOpenPosition(sPosition);
		mpWindow->InvalidateLayout();
		return;
	}
	mpPopup = mpWindow->OpenPopup(mpText, sPosition, NULL);
	mpPopup->SetPointerPasses(!mbTakesPointer);
	if (miMaximumHeight >= 0)
	{
		mpPopup->SetMaximumHeight(miMaximumHeight, mpTheme);
	}
	sInsets.Init(iPaddingY + 1, iPaddingX + 1, iPaddingY + 1, iPaddingX + 1);  //The border is a pixel inside the insets.
	mpPopup->SetLook(mpTheme->GetColour(TC_ToolTip), mpTheme->GetColour(TC_ToolTipBorder), &sInsets);
	mpPopup->GetClosedEvent()->Add(this, &CInfoPopup::PopupClosed);
	mpWindow->GetFocus()->GetPressEvent()->Add(this, &CInfoPopup::FocusPressed);
}


//////////////////////////////////////////////////////////////////////////
//
// Taken away, whatever popups were opened over it staying.  False if it
// wasn't shown.
//
//////////////////////////////////////////////////////////////////////////
bool CInfoPopup::Hide(void)
{
	if (mpPopup.IsNull())
	{
		return false;
	}
	mpPopup->GetClosedEvent()->Remove(this);
	Closed();
	mpWindow->CloseOnePopup(mpText);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Closed by the window: a press somewhere, or every popup closing.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::PopupClosed(void)
{
	if (mpPopup.IsNotNull())
	{
		mpPopup->GetClosedEvent()->Remove(this);
		Closed();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// It is no longer shown: the window's presses aren't listened to.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::Closed(void)
{
	mpPopup = NULL;
	if (mpWindow.IsNotNull())
	{
		mpWindow->GetFocus()->GetPressEvent()->Remove(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text is shown no taller than iHeight pixels, scrolling if it is
// taller; -1 for as tall as it is.  It holds from when it is next shown.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::SetMaximumHeight(int32 iHeight)
{
	miMaximumHeight = iHeight;
	if (mpPopup.IsNotNull())
	{
		mpPopup->SetMaximumHeight(iHeight, mpTheme);
		Show(msPoint, mbAbove);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// With bTakesPointer the pointer finds the popup, not what is under it:
// the wheel scrolls it and a press on its text calls the press event.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::SetTakesPointer(bool bTakesPointer)
{
	mbTakesPointer = bTakesPointer;
	if (mpPopup.IsNotNull())
	{
		mpPopup->SetPointerPasses(!bTakesPointer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A press somewhere in the window, before it is given to anything: on the
// popup while it takes the pointer, the press event is called with the
// character under the pointer, MAX_SIZE if none is.
//
//////////////////////////////////////////////////////////////////////////
void CInfoPopup::FocusPressed(void)
{
	Ptr<CComponent>	pOver;
	CInt32Rect		cBounds;
	SInt32Vec2		sPointer;
	size			uiPosition;

	if (!mbTakesPointer || mpPopup.IsNull())
	{
		return;
	}
	pOver = mpWindow->GetFocus()->GetMouseOverComponent();
	if (pOver.IsNull() || ((pOver != mpPopup) && !pOver->IsWithin(mpPopup)))
	{
		return;
	}
	mpText->GetActualBounds(&cBounds);
	sPointer = mpWindow->GetFocus()->GetPointer();
	if (!mpText->FindCharacterAt(sPointer.x - cBounds.GetLeft(), sPointer.y - cBounds.GetTop(), &uiPosition))
	{
		uiPosition = MAX_SIZE;
	}
	mcPressed.Call(this, uiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Scrolled down iLines of its text (up, if negative), or a view's height
// less a line for each of iPages, as far as its ends.  False if it isn't
// shown or all of its text fits: there is nothing to scroll.
//
//////////////////////////////////////////////////////////////////////////
bool CInfoPopup::Scroll(int32 iLines)
{
	CInt32Rect	cLine;

	if (!IsScrolling() || !mpText->GetLineBounds(0, &cLine))
	{
		return false;
	}
	return ScrollBy(iLines * cLine.GetHeight());
}


bool CInfoPopup::ScrollPages(int32 iPages)
{
	CInt32Rect	cLine;
	CInt32Rect	cView;
	int32		iPage;

	if (!IsScrolling() || !mpText->GetLineBounds(0, &cLine) || !mpPopup->GetScrollPane()->GetScrollView(&cView))
	{
		return false;
	}
	iPage = cView.GetHeight() - cLine.GetHeight();
	if (iPage < cLine.GetHeight())
	{
		iPage = cLine.GetHeight();
	}
	return ScrollBy(iPages * iPage);
}


bool CInfoPopup::ScrollBy(int32 iPixels)
{
	Ptr<CScrollPane>	pPane;
	SInt32Vec2			sMaximum;
	int32				iY;

	pPane = mpPopup->GetScrollPane();
	sMaximum = pPane->GetMaximumScrollPosition();
	iY = pPane->GetScrollPosition().y + iPixels;
	if (iY > sMaximum.y)
	{
		iY = sMaximum.y;
	}
	if (iY < 0)
	{
		iY = 0;
	}
	pPane->SetScrollPosition(SBO_Vertical, iY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// It is shown with more text than its maximum height shows.
//
//////////////////////////////////////////////////////////////////////////
bool CInfoPopup::IsScrolling(void)
{
	if (mpPopup.IsNull() || mpPopup->GetScrollPane().IsNull())
	{
		return false;
	}
	mpWindow->Layout();
	return mpPopup->GetScrollPane()->GetMaximumScrollPosition().y > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// How far down its text is scrolled, in pixels; 0 when it isn't shown.
//
//////////////////////////////////////////////////////////////////////////
int32 CInfoPopup::GetScrollPosition(void)
{
	if (mpPopup.IsNull() || mpPopup->GetScrollPane().IsNull())
	{
		return 0;
	}
	return mpPopup->GetScrollPane()->GetScrollPosition().y;
}


int32 CInfoPopup::GetMaximumHeight(void) const { return miMaximumHeight; }
bool CInfoPopup::IsTakesPointer(void) const { return mbTakesPointer; }
CInfoPopupPressEvent* CInfoPopup::GetPressEvent(void) { return &mcPressed; }
bool CInfoPopup::IsShown(void) { return mpPopup.IsNotNull(); }
SInt32Vec2 CInfoPopup::GetPoint(void) const { return msPoint; }
bool CInfoPopup::IsAbove(void) const { return mbAbove; }
Ptr<CTextArea> CInfoPopup::GetTextArea(void) const { return mpText; }
Ptr<CPopup> CInfoPopup::GetPopup(void) const { return mpPopup; }

