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
#include "DockArea.h"
#include "DockSplit.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "DockEdge.h"
#include "DockGuides.h"
#include "AutoHide.h"
#include "DockManager.h"


//////////////////////////////////////////////////////////////////////////
//
// The drag tracker takes the drag going on in pSource, before what is
// dragged is torn off and the tab or title bar that started it goes.
// Returns false if no drag is going on there.
//
//////////////////////////////////////////////////////////////////////////
static bool HandOverToTracker(Ptr<CWindow> pSource, Ptr<CDockDragTracker> pTracker)
{
	Ptr<CFocus>	pFocus;

	pFocus = pSource->GetFocus();
	if (pFocus->GetDrag() != FD_Select)
	{
		return false;
	}
	pFocus->SetFocus(pTracker);
	pFocus->HandOverDrag(pTracker);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A tab pulled off its group: the pane floats under the pointer and the
// floating window is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::BeginPaneDrag(Ptr<CDockPane> pPane)
{
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CWindow>				pSource;
	SInt32Vec2					sPointer;
	CInt32Rect					cRect;
	SInt32Vec2					sGrab;
	int32						iWidth;
	int32						iHeight;

	if (IsDragging() || pPane.IsNull() || pPane->GetGroup().IsNull())
	{
		return;
	}
	pFloating = FindFloating(pPane);
	if (pFloating.IsNotNull() && pFloating->GetRoot()->IsGroup() && (pFloating->GetRoot()->GetFirstGroup()->NumPanes() == 1))
	{
		BeginFloatingDrag(pFloating);
		return;
	}

	pSource = pPane->GetGroup()->GetWindow();
	mpTracker = OMalloc<CDockDragTracker>(pSource, Ptr<CDockManager>(this));
	if (!HandOverToTracker(pSource, mpTracker))
	{
		mpTracker = NULL;
		return;
	}
	mpDragSource = pSource;

	pSource->GetPointerScreenPosition(&sPointer);
	FloatRectangleFor(pPane->GetGroup(), NULL, &cRect);
	iWidth = cRect.GetWidth();
	iHeight = cRect.GetHeight();
	sGrab.Init((iWidth > 80) ? 40 : iWidth / 2, DOCK_FRAME_BORDER + DOCK_TITLE_BAR_HEIGHT / 2);
	cRect.Init(sPointer.x - sGrab.x, sPointer.y - sGrab.y, sPointer.x - sGrab.x + iWidth, sPointer.y - sGrab.y + iHeight);
	Float(pPane, &cRect);
	StartDrag(FindFloating(pPane), sGrab, pSource, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A tool group's title bar pulled: the whole group floats where it is and
// is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::BeginGroupDrag(Ptr<CDockGroup> pGroup)
{
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CWindow>				pSource;
	SInt32Vec2					sPointer;
	CInt32Rect					cRect;
	SInt32Vec2					sGrab;

	if (IsDragging() || pGroup.IsNull() || (pGroup->NumPanes() == 0))
	{
		return;
	}
	pFloating = FindFloating(pGroup->GetPane(0));
	if (pFloating.IsNotNull() && (pFloating->GetRoot() == pGroup))
	{
		BeginFloatingDrag(pFloating);
		return;
	}

	pSource = pGroup->GetWindow();
	mpTracker = OMalloc<CDockDragTracker>(pSource, Ptr<CDockManager>(this));
	if (!HandOverToTracker(pSource, mpTracker))
	{
		mpTracker = NULL;
		return;
	}
	mpDragSource = pSource;

	pSource->GetPointerScreenPosition(&sPointer);
	FloatRectangleFor(pGroup, NULL, &cRect);
	sGrab.Init(sPointer.x - cRect.GetLeft(), sPointer.y - cRect.GetTop());
	FloatGroup(pGroup, &cRect);
	StartDrag(FindFloating(pGroup->GetPane(0)), sGrab, pSource, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A floating window's title bar pulled: the window is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::BeginFloatingDrag(Ptr<CDockFloatingWindow> pFloating)
{
	Ptr<CWindow>	pSource;
	SInt32Vec2		sPointer;
	CInt32Rect		cRect;
	SInt32Vec2		sGrab;

	if (pFloating.IsNull())
	{
		return;
	}
	if (mpTracker.IsNull())
	{
		pSource = pFloating->GetWindow();
		mpTracker = OMalloc<CDockDragTracker>(pSource, Ptr<CDockManager>(this));
		if (!HandOverToTracker(pSource, mpTracker))
		{
			mpTracker = NULL;
			return;
		}
		mpDragSource = pSource;
	}
	mpDragSource->GetPointerScreenPosition(&sPointer);
	pFloating->GetRectangle(&cRect);
	sGrab.Init(sPointer.x - cRect.GetLeft(), sPointer.y - cRect.GetTop());
	StartDrag(pFloating, sGrab, mpDragSource, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The tracker has the drag; the floating window follows the pointer, held
// at sGrab, until it is dropped.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::StartDrag(Ptr<CDockFloatingWindow> pFloating, SInt32Vec2 sGrab, Ptr<CWindow> pSource, bool bTornOff)
{
	if (pFloating.IsNull())
	{
		EndDrag();
		return;
	}
	mpDragFloating = pFloating;
	mpDragSource = pSource;
	msDragGrab = sGrab;
	mbDragTornOff = bTornOff;
	mbDragCancelled = false;
	mbDragNoDock = false;
	msTarget.Init();
	pFloating->GetRectangle(&mcDragStart);
	pFloating->GetWindow()->Raise();
	DragMoved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsDragging(void)
{
	return mpTracker.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
// While set, dropping leaves what is dragged floating.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::SetDragNoDock(bool bNoDock)
{
	mbDragNoDock = bNoDock;
}


//////////////////////////////////////////////////////////////////////////
//
// What the drag holds: the floating window's root.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockManager::GetDraggedNode(void)
{
	if (mpDragFloating.IsNull())
	{
		return NULL;
	}
	return mpDragFloating->GetRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsDraggingDocuments(void)
{
	return mpDragFloating.IsNotNull() && mpDragFloating->HasDocuments();
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer moved: the floating window follows it and the guides of the
// window under it show where it would go.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DragMoved(void)
{
	SInt32Vec2	sPointer;
	CInt32Rect	cRect;
	int32		iWidth;
	int32		iHeight;

	if (mpDragFloating.IsNull() || mpDragSource.IsNull())
	{
		return;
	}
	mpDragSource->GetPointerScreenPosition(&sPointer);
	mpDragFloating->GetRectangle(&cRect);
	iWidth = cRect.GetWidth();
	iHeight = cRect.GetHeight();
	cRect.Init(sPointer.x - msDragGrab.x, sPointer.y - msDragGrab.y, sPointer.x - msDragGrab.x + iWidth, sPointer.y - msDragGrab.y + iHeight);
	mpDragFloating->Move(&cRect);
	if (mbDragNoDock)
	{
		HideGuides();
		msTarget.Init();
		return;
	}
	FindTarget(sPointer, &msTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// The window under sScreen, the floating windows over the main window,
// shows its guides; psTarget is the guide under the pointer, if any.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FindTarget(SInt32Vec2 sScreen, SDockTarget* psTarget)
{
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;
	CInt32Rect					cScreen;
	SInt32Vec2					sClient;

	psTarget->Init();
	for (i = maFloating.Size(); i > 0; i--)
	{
		pFloating = maFloating.Get(i - 1);
		if ((pFloating == mpDragFloating) || !pFloating->IsShown())
		{
			continue;
		}
		pFloating->GetWindow()->GetScreenRectangle(&cScreen);
		if (cScreen.ContainsPoint(sScreen.x, sScreen.y))
		{
			sClient.Init(sScreen.x - cScreen.GetLeft(), sScreen.y - cScreen.GetTop());
			ShowGuides(psTarget, pFloating->GetArea(), sClient);
			return;
		}
	}
	mpWindow->GetScreenRectangle(&cScreen);
	if (cScreen.ContainsPoint(sScreen.x, sScreen.y))
	{
		sClient.Init(sScreen.x - cScreen.GetLeft(), sScreen.y - cScreen.GetTop());
		if (mpArea->IsPointIn(sClient.x, sClient.y))
		{
			ShowGuides(psTarget, mpArea, sClient);
			return;
		}
	}
	HideGuides();
}


//////////////////////////////////////////////////////////////////////////
//
// The area's guides for the pointer at sClient in its window.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ShowGuides(SDockTarget* psTarget, Ptr<CDockArea> pArea, SInt32Vec2 sClient)
{
	Ptr<CDockGuides>	pGuides;
	CInt32Rect			cRect;
	SInt32Vec2			sDragged;

	if (mpGuidesArea != pArea)
	{
		HideGuides();
		pGuides = OMalloc<CDockGuides>(pArea->GetWindow(), Ptr<CDockManager>(this), pArea);
		pArea->ShowGuides(pGuides);
		mpGuidesArea = pArea;
		pArea->GetWindow()->Layout();
	}
	pGuides = pArea->GetGuides();
	mpDragFloating->GetRectangle(&cRect);
	sDragged.Init(cRect.GetWidth(), cRect.GetHeight());
	pGuides->Build(sClient.x, sClient.y, IsDraggingDocuments(), sDragged);
	pGuides->Hit(sClient.x, sClient.y, psTarget, sDragged);
	pArea->GetWindow()->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::HideGuides(void)
{
	if (mpGuidesArea.IsNotNull())
	{
		mpGuidesArea->HideGuides();
		mpGuidesArea->GetWindow()->InvalidateLayout();
		mpGuidesArea = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Escape: what is dragged goes back where it was.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DragCancelled(void)
{
	mbDragCancelled = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The tracker lets go of the drag.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::EndDrag(void)
{
	Ptr<CFocus>				pFocus;
	Ptr<CDockDragTracker>	pTracker;

	HideGuides();
	pTracker = mpTracker;
	mpTracker = NULL;
	mpDragFloating = NULL;
	mpDragSource = NULL;
	msTarget.Init();
	if (pTracker.IsNotNull())
	{
		pFocus = pTracker->GetWindow()->GetFocus();
		if (pFocus->GetDragComponent() == pTracker)
		{
			//The tracker hears the stop but the manager isn't dragging any more.
			pFocus->StopDrag(FD_Select);
		}
		if (pFocus->GetFocussedComponent() == pTracker)
		{
			pFocus->SetFocus(NULL);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The drag stopped.  Dropped on a guide, what is dragged docks there;
// anywhere else it stays floating; cancelled, it goes back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DragDropped(void)
{
	Ptr<CDockFloatingWindow>	pFloating;
	SDockTarget					sTarget;
	bool						bCancelled;
	bool						bTornOff;
	Ptr<CDockNode>				pRoot;
	Ptr<CDockPane>				pSelected;
	Ptr<CDockGroup>				pGroup;
	CInt32Rect					cRect;
	CArray<CDockPane>			aPanes;
	size						i;

	if (!IsDragging())
	{
		return;
	}
	pFloating = mpDragFloating;
	sTarget = msTarget;
	bCancelled = mbDragCancelled;
	bTornOff = mbDragTornOff;
	cRect.Init(&mcDragStart);
	EndDrag();
	if (pFloating.IsNull() || pFloating->GetRoot().IsNull())
	{
		return;
	}
	pGroup = pFloating->GetRoot()->GetFirstGroup();
	if (pGroup.IsNotNull())
	{
		pSelected = pGroup->GetSelectedPane();
	}

	if (bCancelled)
	{
		if (bTornOff)
		{
			DockFloating(pFloating);
		}
		else
		{
			pFloating->Move(&cRect);
		}
	}
	else if (sTarget.IsValid())
	{
		pRoot = pFloating->TakeRoot();
		pFloating->Close();
		maFloating.Remove(pFloating);
		DockNodeAt(pRoot, &sTarget);
	}
	else
	{
		pFloating->GetRectangle(&cRect);
		aPanes.Init();
		pFloating->GetRoot()->GetPanes(&aPanes);
		for (i = 0; i < aPanes.Size(); i++)
		{
			aPanes.Get(i)->SetFloatRectangle(&cRect);
		}
		aPanes.Kill();
	}
	LayoutChanged();
	if (pSelected.IsNotNull() && pSelected->IsShown())
	{
		Activate(pSelected);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pane is docked at the target as a group of its own, or as a tab in
// the target's group.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::DockPaneAt(Ptr<CDockPane> pPane, SDockTarget* psTarget)
{
	Ptr<CDockGroup>	pGroup;

	if (pPane.IsNull() || !psTarget->IsValid())
	{
		return false;
	}
	TakeOut(pPane, true);
	pGroup = CreateGroup(pPane->IsDocument());
	pGroup->AddPane(pPane);
	return DockNodeAt(pGroup, psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// Docks pNode, which isn't in a tree, at the target: along the area's
// edge, beside or into a group, beside the document well or into the
// empty well.  Anything going into the well becomes a document group.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::DockNodeAt(Ptr<CDockNode> pNode, SDockTarget* psTarget)
{
	Ptr<CDockArea>		pArea;
	Ptr<CWindow>		pWindow;
	EDockPaneState		eState;
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockPane>		pPane;
	Ptr<CDockGroup>		pGroup;
	Ptr<CDockGroup>		pFirst;
	Ptr<CDockPane>		pSelected;
	Ptr<CDockNode>		pRoot;
	Ptr<CDockNode>		pDocked;
	int32				iSize;
	bool				bInWell;
	bool				bResult;

	if (pNode.IsNull() || !psTarget->IsValid())
	{
		return false;
	}
	pArea = psTarget->pcArea;
	pWindow = pArea->GetWindow();
	eState = pArea->IsMain() ? DPS_Docked : DPS_Floating;
	if (pNode->GetWindow() != pWindow)
	{
		pNode->MoveToWindow(pWindow);
	}
	aPanes.Init();
	pNode->GetPanes(&aPanes);
	pFirst = pNode->GetFirstGroup();
	if (pFirst.IsNotNull())
	{
		pSelected = pFirst->GetSelectedPane();
	}
	iSize = IsHorizontalDockSide(psTarget->eSide) ? psTarget->cPreview.GetWidth() : psTarget->cPreview.GetHeight();

	bResult = true;
	pDocked = pNode;
	switch (psTarget->eKind)
	{
	case DTK_AreaEdge:
		pRoot = pArea->GetRoot();
		if (pRoot.IsNull())
		{
			pArea->SetRoot(pNode);
		}
		else
		{
			DockBeside(pRoot, pNode, psTarget->eSide, iSize);
		}
		break;

	case DTK_Group:
		pGroup = Ptr<CDockGroup>(dynamic_cast<CDockGroup*>(psTarget->pcNode));
		bInWell = pGroup->IsWithin(mpWell);
		if (psTarget->eSide == DS_Centre)
		{
			for (i = 0; i < aPanes.Size(); i++)
			{
				pPane = aPanes.Get(i);
				pPane->GetGroup()->TakePane(pPane);
				pGroup->AddPane(pPane);
			}
			pDocked = pGroup;
			if (bInWell)
			{
				mpDocumentGroup = pGroup;
			}
		}
		else if (bInWell)
		{
			pDocked = GatherInto(pNode, true);
			DockBeside(pGroup, pDocked, psTarget->eSide, iSize);
		}
		else if (!pNode->HasDocuments() || pGroup->IsDocumentGroup())
		{
			DockBeside(pGroup, pNode, psTarget->eSide, iSize);
		}
		else
		{
			bResult = false;
		}
		break;

	case DTK_WellOuter:
		if (pNode->HasDocuments())
		{
			bResult = false;
			break;
		}
		DockBeside(mpWell, pNode, psTarget->eSide, iSize);
		break;

	case DTK_WellCentre:
		pDocked = GatherInto(pNode, true);
		if (mpWell->GetRoot().IsNull())
		{
			mpWell->SetRoot(pDocked);
		}
		else
		{
			pGroup = mpWell->GetFirstGroup();
			for (i = 0; i < aPanes.Size(); i++)
			{
				pPane = aPanes.Get(i);
				pPane->GetGroup()->TakePane(pPane);
				pGroup->AddPane(pPane);
			}
			pDocked = pGroup;
		}
		mpDocumentGroup = pDocked->GetFirstGroup();
		break;

	default:
		bResult = false;
		break;
	}

	if (!bResult)
	{
		aPanes.Kill();
		return false;
	}
	for (i = 0; i < aPanes.Size(); i++)
	{
		aPanes.Get(i)->SetState(eState);
	}
	aPanes.Kill();
	if (pSelected.IsNotNull() && pSelected->GetGroup().IsNotNull())
	{
		pSelected->GetGroup()->SelectPane(pSelected);
	}
	LayoutChanged();
	return true;
}

