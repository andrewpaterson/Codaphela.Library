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
#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Glyph.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "Popup.h"
#include "DockManager.h"
#include "DockPane.h"
#include "DockArea.h"
#include "AutoHide.h"


//////////////////////////////////////////////////////////////////////////
//
// pTurnedFont is a font turned clockwise.
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::Init(Ptr<CWindow> pWindow, Ptr<CFont> pTurnedFont, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mszText.Init(szText);
	mpFont = pTurnedFont;
	mpSpriteMap = OMalloc<CSpriteMap>();
	mpMaps = OMalloc<CMaps>();
	mpMaps->AddMap(mpSpriteMap);
	mpMaps->SetViewportPosition(0, 0);
	msRequiredSize.Init(0, 0);
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);
	LayoutText();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::Class(void)
{
	CCanvas::Class();

	U_String(mszText);
	M_Pointer(mpFont);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpMaps);
	U_2Int32(msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::Free(void)
{
	mszText.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTurnedLabel::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTurnedLabel::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText);
	LayoutText();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// A turned glyph's sprite is placed by its full top left: the left of
// the line, down the text by the glyphs before it.
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::LayoutText(void)
{
	CUTF8			cUTF8;
	Ptr<CGlyph>		pGlyph;
	size			uiUTFLength;
	int32			iPen;

	mpSpriteMap->BeginChange();
	mpSpriteMap->Clear();
	cUTF8.Init(mszText.Text());
	iPen = 0;
	for (;;)
	{
		uiUTFLength = cUTF8.PeekUTFBytes();
		if ((uiUTFLength == 0) || (uiUTFLength == cUTF8.GetError()))
		{
			break;
		}
		pGlyph = mpFont->GetGlyph(&cUTF8);
		if (pGlyph.IsNull())
		{
			continue;
		}
		mpSpriteMap->AddSprite(pGlyph->GetCel(), 0, iPen);
		iPen += pGlyph->GetStep();
	}
	cUTF8.Kill();
	mpSpriteMap->EndChange();
	msRequiredSize.Init(mpFont->GetLineHeight(), iPen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTurnedLabel::GetDesiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = msRequiredSize;
	if (msDesiredSize.x >= 0)
	{
		sSize.x = msDesiredSize.x;
	}
	if (msDesiredSize.y >= 0)
	{
		sSize.y = msDesiredSize.y;
	}
	return sSize;
}


char* CTurnedLabel::GetText(void) { return mszText.Text(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane, EDockSide eSide)
{
	PreInit();

	CContainer::Init(pWindow);
	mpManager = pManager;
	mpPane = pPane;
	meSide = eSide;
	SetHitAsWhole(true);
	SetCanGetFocus(true);

	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(pManager->GetColour(DC_StripTab));
	mpBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	mpLineDraw = OMalloc<CClearCanvasDraw>(pManager->GetColour(DC_StripTabLine));
	mpLine = OMalloc<CCanvas>(pWindow, mpLineDraw);
	if (IsVertical() && pManager->GetTurnedFont().IsNotNull())
	{
		mpText = OMalloc<CTurnedLabel>(pWindow, pManager->GetTurnedFont(), pPane->GetTitle());
	}
	else
	{
		mpText = OMalloc<CLabel>(pWindow, pManager->GetFont(), pPane->GetTitle());
	}
	AddComponent(mpBackground);
	AddComponent(mpLine);
	AddComponent(mpText);

	AddInputAction(CIC_ActivateComponent, this, &CAutoHideTab::ActivateComponent);
	AddInputAction(CIC_Hover, this, &CAutoHideTab::Hover);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::Class(void)
{
	CContainer::Class();

	M_Pointer(mpManager);
	M_Pointer(mpPane);
	U_Enum(meSide);
	M_Pointer(mpBackgroundDraw);
	M_Pointer(mpBackground);
	M_Pointer(mpLineDraw);
	M_Pointer(mpLine);
	M_Pointer(mpText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideTab::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideTab::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// On the left and right strips the tab runs down the strip.
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideTab::IsVertical(void)
{
	return (meSide == DS_Left) || (meSide == DS_Right);
}


//////////////////////////////////////////////////////////////////////////
//
// Along the strip: the text and a gap each side.
//
//////////////////////////////////////////////////////////////////////////
int32 CAutoHideTab::GetLength(void)
{
	SInt32Vec2	sText;

	sText = mpText->GetDesiredSize();
	if (IsVertical())
	{
		if (mpManager->GetTurnedFont().IsNotNull())
		{
			return sText.y + 2 * AUTO_HIDE_TAB_GAP;
		}
		return sText.y + 2 * AUTO_HIDE_TAB_GAP;
	}
	return sText.x + 2 * AUTO_HIDE_TAB_GAP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::SetRequiredSize(void)
{
	if (IsVertical())
	{
		msRequiredSize.Init(AUTO_HIDE_STRIP_SIZE, GetLength());
	}
	else
	{
		msRequiredSize.Init(GetLength(), AUTO_HIDE_STRIP_SIZE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The line is along the window's edge, the text centred in the rest.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sText;
	SInt32Vec2	sTextPosition;
	SInt32Vec2	sLinePosition;
	SInt32Vec2	sLineSize;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	mpBackground->Layout(sPosition, sAreaSize);

	switch (meSide)
	{
	case DS_Left:
		sLinePosition = sPosition;
		sLineSize.Init(AUTO_HIDE_LINE, sAreaSize.y);
		break;
	case DS_Right:
		sLinePosition.Init(sPosition.x + sAreaSize.x - AUTO_HIDE_LINE, sPosition.y);
		sLineSize.Init(AUTO_HIDE_LINE, sAreaSize.y);
		break;
	case DS_Top:
		sLinePosition = sPosition;
		sLineSize.Init(sAreaSize.x, AUTO_HIDE_LINE);
		break;
	default:
		sLinePosition.Init(sPosition.x, sPosition.y + sAreaSize.y - AUTO_HIDE_LINE);
		sLineSize.Init(sAreaSize.x, AUTO_HIDE_LINE);
		break;
	}
	mpLine->Layout(sLinePosition, sLineSize);

	sText = mpText->GetDesiredSize();
	if (sText.x > sAreaSize.x)
	{
		sText.x = sAreaSize.x;
	}
	if (sText.y > sAreaSize.y)
	{
		sText.y = sAreaSize.y;
	}
	sTextPosition.Init(sPosition.x + (sAreaSize.x - sText.x) / 2, sPosition.y + (sAreaSize.y - sText.y) / 2);
	mpText->Layout(sTextPosition, sText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::TitleChanged(void)
{
	CLabel*			pcLabel;
	CTurnedLabel*	pcTurned;

	pcLabel = dynamic_cast<CLabel*>(&mpText);
	pcTurned = dynamic_cast<CTurnedLabel*>(&mpText);
	if (pcLabel)
	{
		pcLabel->SetText(mpPane->GetTitle());
	}
	else if (pcTurned)
	{
		pcTurned->SetText(mpPane->GetTitle());
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::FocusOrMouseOverChanged(void)
{
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// Lighter under the pointer or while its pane is slid out.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::UpdateLook(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;
	bool					bOpen;

	pFlyout = mpManager->GetFlyout();
	bOpen = pFlyout.IsNotNull() && (pFlyout->GetPane() == mpPane);
	mpBackgroundDraw->SetColour(mpManager->GetColour((IsMouseOver() || bOpen) ? DC_StripTabHover : DC_StripTab));
	mpLineDraw->SetColour(mpManager->GetColour(bOpen ? DC_ActiveTitleBar : DC_StripTabLine));
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing the tab of the pane slid out slides it back.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::ActivateComponent(int x, int y)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	pFlyout = mpManager->GetFlyout();
	if (pFlyout.IsNotNull() && (pFlyout->GetPane() == mpPane))
	{
		mpManager->CloseFlyout();
	}
	else
	{
		mpManager->ShowFlyout(mpPane);
	}
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// Resting the pointer on the tab slides its pane out, as Visual Studio's
// tabs do.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::Hover(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	pFlyout = mpManager->GetFlyout();
	if (pFlyout.IsNull() || (pFlyout->GetPane() != mpPane))
	{
		mpManager->ShowFlyout(mpPane);
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The tab opened the flyout's popup.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::PopupClosed(Ptr<CComponent> pContent)
{
	mpManager->FlyoutClosed();
	UpdateLook();
}


Ptr<CDockPane> CAutoHideTab::GetPane(void) { return mpPane; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideStrip::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, EDockSide eSide)
{
	PreInit();

	CContainer::Init(pWindow);
	mpManager = pManager;
	meSide = eSide;
	maTabs.Init();
	mpBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pManager->GetColour(DC_Strip)));
	AddComponent(mpBackground);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideStrip::Class(void)
{
	CContainer::Class();

	M_Pointer(mpManager);
	U_Enum(meSide);
	M_Pointer(mpBackground);
	M_Embedded(maTabs);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideStrip::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideStrip::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideStrip::SetRequiredSize(void)
{
	msRequiredSize.Init(AUTO_HIDE_STRIP_SIZE, AUTO_HIDE_STRIP_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// The tabs one after another along the strip from its start.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideStrip::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	size				i;
	int32				iAlong;
	int32				iLength;
	Ptr<CAutoHideTab>	pTab;
	bool				bVertical;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	mpBackground->Layout(sPosition, sAreaSize);
	bVertical = (meSide == DS_Left) || (meSide == DS_Right);
	iAlong = AUTO_HIDE_TAB_SPACING;
	for (i = 0; i < maTabs.Size(); i++)
	{
		pTab = maTabs.Get(i);
		iLength = pTab->GetLength();
		if (bVertical)
		{
			pTab->Layout(SInt32Vec2(sPosition.x, sPosition.y + iAlong), SInt32Vec2(sAreaSize.x, iLength));
		}
		else
		{
			pTab->Layout(SInt32Vec2(sPosition.x + iAlong, sPosition.y), SInt32Vec2(iLength, sAreaSize.y));
		}
		iAlong += iLength + AUTO_HIDE_TAB_SPACING;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CAutoHideTab> CAutoHideStrip::AddPane(Ptr<CDockPane> pPane)
{
	Ptr<CAutoHideTab>	pTab;

	pTab = OMalloc<CAutoHideTab>(mpWindow, mpManager, pPane, meSide);
	maTabs.Add(pTab);
	AddComponent(pTab);
	InvalidateLayout();
	return pTab;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideStrip::RemovePane(Ptr<CDockPane> pPane)
{
	Ptr<CAutoHideTab>	pTab;

	pTab = FindTab(pPane);
	if (pTab.IsNull())
	{
		return false;
	}
	RemoveComponent(pTab);
	maTabs.Remove(pTab);
	InvalidateLayout();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CAutoHideTab> CAutoHideStrip::FindTab(Ptr<CDockPane> pPane)
{
	size	i;

	for (i = 0; i < maTabs.Size(); i++)
	{
		if (maTabs.Get(i)->GetPane() == pPane)
		{
			return maTabs.Get(i);
		}
	}
	return NULL;
}


bool				CAutoHideStrip::HasTabs(void) { return maTabs.Size() > 0; }
size				CAutoHideStrip::NumTabs(void) { return maTabs.Size(); }
Ptr<CAutoHideTab>	CAutoHideStrip::GetTab(size uiIndex) { return maTabs.Get(uiIndex); }
EDockSide			CAutoHideStrip::GetSide(void) { return meSide; }


//////////////////////////////////////////////////////////////////////////
//
// The pane's content is put in the flyout; Detach takes it out again.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane)
{
	uint8	uiEdge;

	PreInit();

	CContainer::Init(pWindow);
	mpManager = pManager;
	mpPane = pPane;
	meSide = pPane->GetAutoHideSide();
	miStartSize = 0;
	msStartPointer.Init(0, 0);

	mpBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pManager->GetColour(DC_Background)));
	mpTitleBar = OMalloc<CDockTitleBar>(pWindow, pManager, pPane->GetTitle(), false, true, pPane->IsClosable());
	mpTitleBar->SetPinned(false);
	mpTitleBar->SetActive(true);
	mpTitleBar->GetButtonEvent(DTB_Pin)->Add(this, &CAutoHideFlyout::PinPressed);
	mpTitleBar->GetButtonEvent(DTB_Close)->Add(this, &CAutoHideFlyout::ClosePressed);
	switch (meSide)
	{
	case DS_Left:
		uiEdge = DOCK_EDGE_RIGHT;
		break;
	case DS_Right:
		uiEdge = DOCK_EDGE_LEFT;
		break;
	case DS_Top:
		uiEdge = DOCK_EDGE_BOTTOM;
		break;
	default:
		uiEdge = DOCK_EDGE_TOP;
		break;
	}
	mpEdge = OMalloc<CDockEdge>(pWindow, pManager, this, uiEdge, pManager->GetColour(DC_Sash));

	AddComponent(mpBackground);
	AddComponent(mpTitleBar);
	AddComponent(pPane->GetContent());
	AddComponent(mpEdge);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Class(void)
{
	CContainer::Class();

	M_Pointer(mpManager);
	M_Pointer(mpPane);
	U_Enum(meSide);
	M_Pointer(mpTitleBar);
	M_Pointer(mpEdge);
	M_Pointer(mpBackground);
	U_Int32(miStartSize);
	U_2Int32(msStartPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Free(void)
{
	if (mpTitleBar.IsNotNull())
	{
		mpTitleBar->GetButtonEvent(DTB_Pin)->Remove(this);
		mpTitleBar->GetButtonEvent(DTB_Close)->Remove(this);
	}
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideFlyout::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAutoHideFlyout::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane's content is let go, to be shown somewhere else.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Detach(void)
{
	Ptr<CComponent>	pContent;

	pContent = mpPane->GetContent();
	if (pContent->GetParent() == this)
	{
		RemoveComponent(pContent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::SetRequiredSize(void)
{
	msRequiredSize.Init(DOCK_MINIMUM_SIZE, DOCK_MINIMUM_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// Its desired size and position, in window client space, beside its strip
// inside the root's bounds.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Place(CInt32Rect* pcRootBounds, SInt32Vec2* psPosition)
{
	int32	iSize;
	int32	iMaximum;

	iSize = mpPane->GetAutoHideSize();
	if (iSize <= 0)
	{
		iSize = DOCK_DEFAULT_SIZE;
	}
	iMaximum = ((meSide == DS_Left) || (meSide == DS_Right)) ? pcRootBounds->GetWidth() : pcRootBounds->GetHeight();
	iMaximum -= DOCK_MINIMUM_SIZE;
	if (iSize > iMaximum)
	{
		iSize = iMaximum;
	}
	if (iSize < DOCK_MINIMUM_SIZE)
	{
		iSize = DOCK_MINIMUM_SIZE;
	}
	switch (meSide)
	{
	case DS_Left:
		SetDesiredSize(iSize, pcRootBounds->GetHeight());
		psPosition->Init(pcRootBounds->GetLeft(), pcRootBounds->GetTop());
		break;
	case DS_Right:
		SetDesiredSize(iSize, pcRootBounds->GetHeight());
		psPosition->Init(pcRootBounds->GetLeft() + pcRootBounds->GetWidth() - iSize, pcRootBounds->GetTop());
		break;
	case DS_Top:
		SetDesiredSize(pcRootBounds->GetWidth(), iSize);
		psPosition->Init(pcRootBounds->GetLeft(), pcRootBounds->GetTop());
		break;
	default:
		SetDesiredSize(pcRootBounds->GetWidth(), iSize);
		psPosition->Init(pcRootBounds->GetLeft(), pcRootBounds->GetTop() + pcRootBounds->GetHeight() - iSize);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The edge on the side away from the strip, the title bar along the top
// and the content under it.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	int32		iLeft;
	int32		iTop;
	int32		iRight;
	int32		iBottom;
	int32		iTitle;

	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	mpBackground->Layout(sPosition, sAreaSize);

	iLeft = sPosition.x;
	iTop = sPosition.y;
	iRight = sPosition.x + sAreaSize.x;
	iBottom = sPosition.y + sAreaSize.y;
	switch (meSide)
	{
	case DS_Left:
		mpEdge->Layout(SInt32Vec2(iRight - AUTO_HIDE_EDGE, iTop), SInt32Vec2(AUTO_HIDE_EDGE, sAreaSize.y));
		iRight -= AUTO_HIDE_EDGE;
		break;
	case DS_Right:
		mpEdge->Layout(SInt32Vec2(iLeft, iTop), SInt32Vec2(AUTO_HIDE_EDGE, sAreaSize.y));
		iLeft += AUTO_HIDE_EDGE;
		break;
	case DS_Top:
		mpEdge->Layout(SInt32Vec2(iLeft, iBottom - AUTO_HIDE_EDGE), SInt32Vec2(sAreaSize.x, AUTO_HIDE_EDGE));
		iBottom -= AUTO_HIDE_EDGE;
		break;
	default:
		mpEdge->Layout(SInt32Vec2(iLeft, iTop), SInt32Vec2(sAreaSize.x, AUTO_HIDE_EDGE));
		iTop += AUTO_HIDE_EDGE;
		break;
	}
	iTitle = DOCK_TITLE_BAR_HEIGHT;
	if (iTitle > iBottom - iTop)
	{
		iTitle = iBottom - iTop;
	}
	mpTitleBar->Layout(SInt32Vec2(iLeft, iTop), SInt32Vec2(iRight - iLeft, iTitle));
	iTop += iTitle;
	if (mpPane->GetContent()->GetParent() == this)
	{
		mpPane->GetContent()->Layout(SInt32Vec2(iLeft, iTop), SInt32Vec2(iRight - iLeft, iBottom - iTop));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pin docks the pane back where it was.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::PinPressed(CDockTitleBar* pcTitleBar)
{
	mpManager->Dock(mpPane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::ClosePressed(CDockTitleBar* pcTitleBar)
{
	mpManager->Close(mpPane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen)
{
	SInt32Vec2	sSize;

	sSize = GetActualSize();
	miStartSize = ((meSide == DS_Left) || (meSide == DS_Right)) ? sSize.x : sSize.y;
	msStartPointer = sScreen;
}


//////////////////////////////////////////////////////////////////////////
//
// The flyout grows away from its strip.
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen)
{
	int32		iSize;
	CInt32Rect	cRoot;
	SInt32Vec2	sPosition;
	Ptr<CPopup>	pPopup;

	switch (meSide)
	{
	case DS_Left:
		iSize = miStartSize + (sScreen.x - msStartPointer.x);
		break;
	case DS_Right:
		iSize = miStartSize - (sScreen.x - msStartPointer.x);
		break;
	case DS_Top:
		iSize = miStartSize + (sScreen.y - msStartPointer.y);
		break;
	default:
		iSize = miStartSize - (sScreen.y - msStartPointer.y);
		break;
	}
	if (iSize < DOCK_MINIMUM_SIZE)
	{
		iSize = DOCK_MINIMUM_SIZE;
	}
	mpPane->SetAutoHideSize(iSize);
	mpManager->GetArea()->GetRootBounds(&cRoot);
	Place(&cRoot, &sPosition);
	pPopup = mpWindow->GetPopup(this);
	if (pPopup.IsNotNull())
	{
		pPopup->SetOpenPosition(sPosition);
	}
	mpWindow->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideFlyout::EdgeReleased(CDockEdge* pcEdge)
{
	mpManager->LayoutChanged();
}


Ptr<CDockPane>		CAutoHideFlyout::GetPane(void) { return mpPane; }
Ptr<CDockTitleBar>	CAutoHideFlyout::GetTitleBar(void) { return mpTitleBar; }
Ptr<CDockEdge>		CAutoHideFlyout::GetEdge(void) { return mpEdge; }
EDockSide			CAutoHideFlyout::GetSide(void) { return meSide; }

