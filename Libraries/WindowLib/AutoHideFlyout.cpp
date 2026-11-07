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
#include "DockManager.h"
#include "DockPane.h"
#include "DockArea.h"
#include "AutoHide.h"
#include "AutoHideFlyout.h"


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
	//A popup isn't in the dock area: Shift+Escape and Control+F4 are taken here too.
	AddInputAction(CIC_CloseDocument, &pManager, &CDockManager::CloseActiveDocument);
	AddInputAction(CIC_HideToolWindow, &pManager, &CDockManager::HideActiveTool);

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
		mpTitleBar->RemoveListener(this);
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
