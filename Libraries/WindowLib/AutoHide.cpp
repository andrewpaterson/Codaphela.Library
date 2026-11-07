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
#include "AutoHideFlyout.h"


//////////////////////////////////////////////////////////////////////////
//
// pTurnedFont is a font turned clockwise.
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::Init(Ptr<CWindow> pWindow, Ptr<CFont> pTurnedFont, const char* szText)
{
	PreInit();
	CLabel::Init(pWindow, pTurnedFont, szText);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTurnedLabel::Class(void)
{
	CLabel::Class();
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
void CAutoHideTab::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, Ptr<CDockPane> pPane, EDockSide eSide)
{
	PreInit();

	CContainer::Init(pWindow);
	mpManager = pManager;
	mpPane = pPane;
	meSide = eSide;
	SetHitAsWhole(true);
	SetCanGetFocus(true);

	mpBackgroundDraw = OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockStripTab));
	mpBackground = OMalloc<CCanvas>(pWindow, mpBackgroundDraw);
	mpLineDraw = OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockStripTabLine));
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
	return IsHorizontalDockSide(meSide);
}


//////////////////////////////////////////////////////////////////////////
//
// Along the strip: the text and a gap each side.
//
//////////////////////////////////////////////////////////////////////////
int32 CAutoHideTab::GetLength(void)
{
	SInt32Vec2	sText;

	//Down a side strip the tab is as long as its text is tall: turned text's length.  Without a turned font the
	//upright text is cut off by the strip's width.
	sText = mpText->GetDesiredSize();
	if (IsVertical())
	{
		return sText.y + 2 * mpManager->GetMetric(TM_AutoHideTabGap);
	}
	return sText.x + 2 * mpManager->GetMetric(TM_AutoHideTabGap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAutoHideTab::SetRequiredSize(void)
{
	if (IsVertical())
	{
		msRequiredSize.Init(mpManager->GetMetric(TM_AutoHideStripSize), GetLength());
	}
	else
	{
		msRequiredSize.Init(GetLength(), mpManager->GetMetric(TM_AutoHideStripSize));
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
		sLineSize.Init(mpManager->GetMetric(TM_AutoHideLine), sAreaSize.y);
		break;
	case DS_Right:
		sLinePosition.Init(sPosition.x + sAreaSize.x - mpManager->GetMetric(TM_AutoHideLine), sPosition.y);
		sLineSize.Init(mpManager->GetMetric(TM_AutoHideLine), sAreaSize.y);
		break;
	case DS_Top:
		sLinePosition = sPosition;
		sLineSize.Init(sAreaSize.x, mpManager->GetMetric(TM_AutoHideLine));
		break;
	default:
		sLinePosition.Init(sPosition.x, sPosition.y + sAreaSize.y - mpManager->GetMetric(TM_AutoHideLine));
		sLineSize.Init(sAreaSize.x, mpManager->GetMetric(TM_AutoHideLine));
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
	mpText->SetText(mpPane->GetTitle());
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
	mpBackgroundDraw->SetColour(mpManager->GetColour((IsMouseOver() || bOpen) ? TC_DockStripTabHover : TC_DockStripTab));
	mpLineDraw->SetColour(mpManager->GetColour(bOpen ? TC_DockActiveTitleBar : TC_DockStripTabLine));
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
// The title shown, turned or upright.
//
//////////////////////////////////////////////////////////////////////////
char* CAutoHideTab::GetText(void)
{
	return mpText->GetText();
}


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
	mpBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockStrip)));
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
	msRequiredSize.Init(mpManager->GetMetric(TM_AutoHideStripSize), mpManager->GetMetric(TM_AutoHideStripSize));
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
	bVertical = IsHorizontalDockSide(meSide);
	iAlong = mpManager->GetMetric(TM_AutoHideTabSpacing);
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
		iAlong += iLength + mpManager->GetMetric(TM_AutoHideTabSpacing);
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


