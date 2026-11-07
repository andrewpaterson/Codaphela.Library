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
#include "ClearCanvasDraw.h"
#include "Window.h"
#include "DockManager.h"
#include "DockArea.h"
#include "DockGroup.h"
#include "DockPane.h"
#include "DockFloatingWindow.h"


#define DOCK_FLOATING_MINIMUM	80


//////////////////////////////////////////////////////////////////////////
//
// The window is made but not shown, see Show.  pcScreenRectangle is its
// client area on the screen.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::Init(Ptr<CDockManager> pManager, CInt32Rect* pcScreenRectangle)
{
	Ptr<CWindow>		pMain;
	CColourFormatHelper	cFormat;
	Ptr<CWindowTick>	pNoTick;

	PreInit();

	mpManager = pManager;
	mcRectangle.Init(pcScreenRectangle);
	mcResizeStart.Init(0, 0, 0, 0);
	msResizePointer.Init(0, 0);
	maEdges.Init();
	mbTitleShown = false;

	pMain = pManager->GetWindow();
	pMain->GetColourFormat(&cFormat);
	mpWindow = OMalloc<CWindow>("", pMain->GetFactory(), pNoTick, OMalloc<CClearCanvasDraw>(pManager->GetColour(TC_DockBackground)), &cFormat);
	mpWindow->SetRedrawLike(pMain);
	mpWindow->GetClosedEvent()->Add(this, &CDockFloatingWindow::WindowClosed);

	//A border of edges around the inside: a title bar over the area.
	mpFrame = OMalloc<CGridContainer>(&mpWindow);
	mpFrame->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddEdge(DOCK_EDGE_LEFT | DOCK_EDGE_TOP, 0, 0, 0, 0);
	AddEdge(DOCK_EDGE_TOP, 1, 0, 1, 0);
	AddEdge(DOCK_EDGE_RIGHT | DOCK_EDGE_TOP, 2, 0, 0, 0);
	AddEdge(DOCK_EDGE_LEFT, 0, 1, 0, 1);
	AddEdge(DOCK_EDGE_RIGHT, 2, 1, 0, 1);
	AddEdge(DOCK_EDGE_LEFT | DOCK_EDGE_BOTTOM, 0, 2, 0, 0);
	AddEdge(DOCK_EDGE_BOTTOM, 1, 2, 1, 0);
	AddEdge(DOCK_EDGE_RIGHT | DOCK_EDGE_BOTTOM, 2, 2, 0, 0);

	mpInner = OMalloc<CGridContainer>(&mpWindow);
	mpInner->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpFrame->AddComponent(mpInner, 1, 1, 1, 1, 1, 1);

	mpTitleBar = OMalloc<CDockTitleBar>(&mpWindow, pManager, "", true, false, true);
	mpTitleBar->GetDragEvent()->Add(this, &CDockFloatingWindow::TitleDragged);
	mpTitleBar->GetDoubleClickEvent()->Add(this, &CDockFloatingWindow::TitleDoubleClicked);
	mpTitleBar->GetButtonEvent(DTB_Close)->Add(this, &CDockFloatingWindow::ClosePressed);
	mpTitleBar->GetButtonEvent(DTB_Menu)->Add(this, &CDockFloatingWindow::MenuPressed);

	mpArea = OMalloc<CDockArea>(&mpWindow, pManager, false);
	mpArea->SetFloating(this);
	mpInner->AddComponent(mpArea, 0, 1, 1, 1, 1, 1);
	mpWindow->SetContainer(mpFrame);
	UpdateTitle();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::AddEdge(uint8 uiEdges, int iColumn, int iRow, int iWidthWeight, int iHeightWeight)
{
	Ptr<CDockEdge>	pEdge;

	pEdge = OMalloc<CDockEdge>(&mpWindow, mpManager, this, uiEdges, mpManager->GetColour(TC_DockFrameBorder));
	pEdge->SetDesiredSize(iWidthWeight ? -1 : mpManager->GetMetric(TM_DockFrameBorder), iHeightWeight ? -1 : mpManager->GetMetric(TM_DockFrameBorder));
	maEdges.Add(pEdge);
	mpFrame->AddComponent(pEdge, iColumn, iRow, 1, 1, iWidthWeight, iHeightWeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::Class(void)
{
	M_Pointer(mpManager);
	M_Pointer(mpWindow);
	M_Pointer(mpFrame);
	M_Pointer(mpInner);
	M_Pointer(mpTitleBar);
	M_Pointer(mpArea);
	M_Embedded(maEdges);
	U_Bool(mbTitleShown);
	U_Data(CInt32Rect, mcRectangle);
	U_Data(CInt32Rect, mcResizeStart);
	U_2Int32(msResizePointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::Free(void)
{
	if (mpTitleBar.IsNotNull())
	{
		mpTitleBar->RemoveListener(this);
	}
	if (mpWindow.IsNotNull())
	{
		mpWindow->GetClosedEvent()->Remove(this);
		mpWindow->CloseOwned();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the window over the main window.
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::Show(void)
{
	bool	bShown;

	if (mpWindow->GetOwner().IsNotNull())
	{
		return true;
	}
	bShown = mpWindow->ShowOwned(mpManager->GetWindow(), &mcRectangle);
	if (bShown)
	{
		mpManager->WatchFocus(mpWindow);
	}
	return bShown;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the window off the screen.  What it holds stays in it.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::Close(void)
{
	if (mpWindow->GetOwner().IsNotNull())
	{
		mpManager->UnwatchFocus(mpWindow);
		mpWindow->CloseOwned();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::IsShown(void)
{
	return mpWindow->GetOwner().IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
// pRoot must be of this window already, see CComponent::MoveToWindow.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::SetRoot(Ptr<CDockNode> pRoot)
{
	mpArea->SetRoot(pRoot);
	UpdateTitle();
}


//////////////////////////////////////////////////////////////////////////
//
// The root comes out of the window, which is left empty.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockFloatingWindow::TakeRoot(void)
{
	Ptr<CDockNode>	pRoot;

	pRoot = mpArea->GetRoot();
	mpArea->SetRoot(NULL);
	return pRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// The group's own title bar moves the window then.
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::IsSingleToolGroup(void)
{
	Ptr<CDockNode>	pRoot;
	Ptr<CDockGroup>	pGroup;

	pRoot = mpArea->GetRoot();
	if (pRoot.IsNull() || !pRoot->IsGroup())
	{
		return false;
	}
	pGroup = pRoot->GetFirstGroup();
	return !pGroup->IsDocumentGroup();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFloatingWindow::HasDocuments(void)
{
	Ptr<CDockNode>	pRoot;

	pRoot = mpArea->GetRoot();
	return pRoot.IsNotNull() && pRoot->HasDocuments();
}


//////////////////////////////////////////////////////////////////////////
//
// The title bar, and the platform's title of the window, show the first
// group's selected pane.  The title bar is left out while a single tool
// group's title bar does its job.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::UpdateTitle(void)
{
	Ptr<CDockNode>	pRoot;
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pPane;
	bool			bShow;

	pRoot = mpArea->GetRoot();
	pPane = NULL;
	if (pRoot.IsNotNull())
	{
		pGroup = pRoot->GetFirstGroup();
		if (pGroup.IsNotNull())
		{
			pPane = pGroup->GetSelectedPane();
		}
	}
	mpTitleBar->SetTitle(pPane.IsNotNull() ? pPane->GetTitle() : "");
	mpWindow->SetWindowTitle(pPane.IsNotNull() ? pPane->GetTitle() : "");
	bShow = !IsSingleToolGroup();
	if (bShow != mbTitleShown)
	{
		mbTitleShown = bShow;
		if (bShow)
		{
			mpInner->AddComponent(mpTitleBar, 0, 0, 1, 1, 1, 0);
		}
		else
		{
			mpInner->RemoveComponent(mpTitleBar);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Moves and resizes the window, its client area on the screen.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::Move(CInt32Rect* pcScreenRectangle)
{
	mcRectangle.Init(pcScreenRectangle);
	if (IsShown())
	{
		mpWindow->SetScreenRectangle(pcScreenRectangle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::GetRectangle(CInt32Rect* pcDest)
{
	pcDest->Init(&mcRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::TitleDragged(CDockTitleBar* pcTitleBar)
{
	mpManager->BeginFloatingDrag(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::TitleDoubleClicked(CDockTitleBar* pcTitleBar)
{
	mpManager->DockFloating(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::ClosePressed(CDockTitleBar* pcTitleBar)
{
	mpManager->CloseFloating(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The selected pane's menu, of the first group.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::MenuPressed(CDockTitleBar* pcTitleBar)
{
	Ptr<CDockNode>		pRoot;
	Ptr<CDockGroup>		pGroup;
	Ptr<CIconComponent>	pButton;
	SInt32Vec2			sPosition;

	pRoot = mpArea->GetRoot();
	if (pRoot.IsNull())
	{
		return;
	}
	pGroup = pRoot->GetFirstGroup();
	if (pGroup.IsNull() || pGroup->GetSelectedPane().IsNull())
	{
		return;
	}
	pButton = mpTitleBar->GetButton(DTB_Menu);
	sPosition = pButton->GetPosition();
	sPosition.y += pButton->GetActualSize().y;
	pGroup->OpenMenu(pGroup->GetSelectedPane(), sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::EdgeDragStarted(CDockEdge* pcEdge, SInt32Vec2 sScreen)
{
	mcResizeStart.Init(&mcRectangle);
	msResizePointer = sScreen;
}


//////////////////////////////////////////////////////////////////////////
//
// The edges dragged move with the pointer; the window is kept at least a
// small size.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::EdgeDragged(CDockEdge* pcEdge, SInt32Vec2 sScreen)
{
	int32		iLeft;
	int32		iTop;
	int32		iRight;
	int32		iBottom;
	int32		dx;
	int32		dy;
	uint8		uiEdges;
	CInt32Rect	cRect;

	uiEdges = pcEdge->GetEdges();
	dx = sScreen.x - msResizePointer.x;
	dy = sScreen.y - msResizePointer.y;
	iLeft = mcResizeStart.GetLeft();
	iTop = mcResizeStart.GetTop();
	iRight = iLeft + mcResizeStart.GetWidth();
	iBottom = iTop + mcResizeStart.GetHeight();
	if (uiEdges & DOCK_EDGE_LEFT)
	{
		iLeft = MoveLowEdge(iLeft + dx, iRight);
	}
	if (uiEdges & DOCK_EDGE_RIGHT)
	{
		iRight = MoveHighEdge(iRight + dx, iLeft);
	}
	if (uiEdges & DOCK_EDGE_TOP)
	{
		iTop = MoveLowEdge(iTop + dy, iBottom);
	}
	if (uiEdges & DOCK_EDGE_BOTTOM)
	{
		iBottom = MoveHighEdge(iBottom + dy, iTop);
	}
	cRect.Init(iLeft, iTop, iRight, iBottom);
	Move(&cRect);
}


//////////////////////////////////////////////////////////////////////////
//
// A left or top edge moved to iEdge, kept the minimum size before the
// opposite edge, iHigh.
//
//////////////////////////////////////////////////////////////////////////
int32 CDockFloatingWindow::MoveLowEdge(int32 iEdge, int32 iHigh)
{
	return (iHigh - iEdge < DOCK_FLOATING_MINIMUM) ? iHigh - DOCK_FLOATING_MINIMUM : iEdge;
}


//////////////////////////////////////////////////////////////////////////
//
// A right or bottom edge moved to iEdge, kept the minimum size after
// the opposite edge, iLow.
//
//////////////////////////////////////////////////////////////////////////
int32 CDockFloatingWindow::MoveHighEdge(int32 iEdge, int32 iLow)
{
	return (iEdge - iLow < DOCK_FLOATING_MINIMUM) ? iLow + DOCK_FLOATING_MINIMUM : iEdge;
}


//////////////////////////////////////////////////////////////////////////
//
// The panes remember where they float.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::EdgeReleased(CDockEdge* pcEdge)
{
	Ptr<CDockNode>		pRoot;

	pRoot = mpArea->GetRoot();
	if (pRoot.IsNotNull())
	{
		pRoot->SetPaneFloatRectangles(&mcRectangle);
	}
	mpManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The platform closed the window, Alt+F4 say.
//
//////////////////////////////////////////////////////////////////////////
void CDockFloatingWindow::WindowClosed(CWindow* pcWindow)
{
	mpManager->FloatingWindowClosed(pcWindow);
}


Ptr<CWindow>		CDockFloatingWindow::GetWindow(void) { return mpWindow; }
Ptr<CDockArea>		CDockFloatingWindow::GetArea(void) { return mpArea; }
Ptr<CDockTitleBar>	CDockFloatingWindow::GetTitleBar(void) { return mpTitleBar; }
Ptr<CDockNode>		CDockFloatingWindow::GetRoot(void) { return mpArea->GetRoot(); }
size				CDockFloatingWindow::NumEdges(void) { return maEdges.NumElements(); }
Ptr<CDockEdge>		CDockFloatingWindow::GetEdge(size uiIndex) { return maEdges.Get(uiIndex); }

