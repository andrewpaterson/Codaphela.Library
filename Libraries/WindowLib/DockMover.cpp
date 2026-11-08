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
#include "Window.h"
#include "DockArea.h"
#include "DockSplit.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "AutoHide.h"
#include "DockManager.h"
#include "DockMover.h"


//The edges a wide group and a tall group go to, nearest first when distances tie.
static const EDockSide gaeWideGroupEdges[4] = { DS_Bottom, DS_Top, DS_Left, DS_Right };
static const EDockSide gaeTallGroupEdges[4] = { DS_Left, DS_Right, DS_Bottom, DS_Top };


//////////////////////////////////////////////////////////////////////////
//
// pcManager holds the panes, groups and windows.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::Init(CDockManager* pcManager, CDockState* pcState)
{
	mpcManager = pcManager;
	mpcState = pcState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::Float(Ptr<CDockPane> pPane, CInt32Rect* pcScreenRectangle)
{
	CInt32Rect					cRect;
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CDockGroup>				pGroup;
	Ptr<CComponent>				pFrom;

	if (pPane.IsNull())
	{
		return;
	}
	if (pPane->GetState() == DPS_Floating)
	{
		pFloating = mpcManager->FindFloating(pPane);
		if (pFloating.IsNotNull() && pFloating->GetRoot()->IsGroup() && (pFloating->GetRoot()->GetFirstGroup()->NumPanes() == 1))
		{
			if (pcScreenRectangle)
			{
				pFloating->Move(pcScreenRectangle);
			}
			return;
		}
	}
	pFrom = pPane->GetGroup();
	if (pcScreenRectangle)
	{
		cRect.Init(pcScreenRectangle);
	}
	else
	{
		FloatRectangleFor(pFrom, pPane, &cRect);
	}
	TakeOut(pPane, true);

	pFloating = CreateFloating(&cRect);
	pGroup = mpcManager->CreateGroup(pPane->IsDocument());
	pGroup->MoveToWindow(pFloating->GetWindow());
	pPane->GetContent()->MoveToWindow(pFloating->GetWindow());
	pGroup->AddPane(pPane);
	pFloating->SetRoot(pGroup);
	pPane->SetState(DPS_Floating);
	pPane->SetFloatRectangle(&cRect);
	pFloating->Show();
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::FloatGroup(Ptr<CDockGroup> pGroup, CInt32Rect* pcScreenRectangle)
{
	CInt32Rect					cRect;
	Ptr<CDockFloatingWindow>	pFloating;
	size						i;

	if (pGroup.IsNull() || (pGroup->NumPanes() == 0))
	{
		return;
	}
	pFloating = mpcManager->FindFloating(pGroup->GetPane(0));
	if (pFloating.IsNotNull() && (pFloating->GetRoot() == pGroup))
	{
		if (pcScreenRectangle)
		{
			pFloating->Move(pcScreenRectangle);
		}
		return;
	}
	if (pcScreenRectangle)
	{
		cRect.Init(pcScreenRectangle);
	}
	else
	{
		FloatRectangleFor(pGroup, NULL, &cRect);
	}

	mpcState->GetFocus()->CloseFlyout();
	if (pGroup->IsWithin(mpcState->GetArea()))
	{
		mpcState->GetRestorer()->GiveToGroup(pGroup);
	}
	if (mpcState->GetDocumentGroup() == pGroup)
	{
		mpcState->SetDocumentGroup(NULL);
	}
	mpcState->GetTree()->RemoveNode(pGroup);

	pFloating = CreateFloating(&cRect);
	pGroup->MoveToWindow(pFloating->GetWindow());
	pFloating->SetRoot(pGroup);
	pGroup->SetPaneStates(DPS_Floating);
	for (i = 0; i < pGroup->NumPanes(); i++)
	{
		pGroup->GetPane(i)->SetFloatRectangle(&cRect);
	}
	pFloating->Show();
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::Dock(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull() || (pPane->GetState() == DPS_Docked) || (pPane->GetState() == DPS_Removed))
	{
		return;
	}
	TakeOut(pPane, false);
	mpcState->GetRestorer()->Restore(pPane);
	pPane->GetGroup()->SelectPane(pPane);
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::DockAsDocument(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull())
	{
		return;
	}
	TakeOut(pPane, true);
	mpcManager->AddToWell(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::AutoHide(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>			pGroup;
	EDockSide				eSide;
	int32					iSize;
	SInt32Vec2				sSize;
	Ptr<CAutoHideStrip>		pStrip;

	if (pPane.IsNull() || pPane->IsDocument() || (pPane->GetState() != DPS_Docked))
	{
		return;
	}
	pGroup = pPane->GetGroup();
	if (mpcManager->IsInWell(pGroup))
	{
		return;
	}
	eSide = NearestEdge(pGroup);
	sSize = pGroup->GetActualSize();
	iSize = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	TakeOut(pPane, true);
	pPane->SetAutoHide(eSide, (iSize > DOCK_MINIMUM_SIZE) ? iSize : DOCK_DEFAULT_SIZE);
	pStrip = mpcState->GetArea()->GetStrip(eSide);
	pStrip->AddPane(pPane);
	pPane->SetState(DPS_AutoHidden);
	mpcState->GetArea()->StripsChanged();
	if (mpcState->GetFocus()->GetActivePane() == pPane)
	{
		mpcState->GetFocus()->SetActivePane(NULL);
	}
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::AutoHideGroup(Ptr<CDockGroup> pGroup)
{
	CArray<CDockPane>		aPanes;
	size					i;
	Ptr<CDockPane>			pPane;
	EDockSide				eSide;
	int32					iSize;
	SInt32Vec2				sSize;
	Ptr<CAutoHideStrip>		pStrip;

	if (pGroup.IsNull() || pGroup->IsDocumentGroup() || !pGroup->IsWithin(mpcState->GetArea()) || mpcManager->IsInWell(pGroup))
	{
		return;
	}
	eSide = NearestEdge(pGroup);
	sSize = pGroup->GetActualSize();
	iSize = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	mpcState->GetRestorer()->GiveToGroup(pGroup);

	aPanes.Init();
	pGroup->GetPanes(&aPanes);
	pStrip = mpcState->GetArea()->GetStrip(eSide);
	for (i = 0; i < aPanes.NumElements(); i++)
	{
		pPane = aPanes.Get(i);
		TakeOut(pPane, false);
		pPane->SetAutoHide(eSide, (iSize > DOCK_MINIMUM_SIZE) ? iSize : DOCK_DEFAULT_SIZE);
		pStrip->AddPane(pPane);
		pPane->SetState(DPS_AutoHidden);
		if (mpcState->GetFocus()->GetActivePane() == pPane)
		{
			mpcState->GetFocus()->SetActivePane(NULL);
		}
	}
	aPanes.Kill();
	mpcState->GetArea()->StripsChanged();
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::ToggleFloat(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull())
	{
		return;
	}
	if (pPane->GetState() == DPS_Floating)
	{
		Dock(pPane);
	}
	else
	{
		Float(pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::ToggleFloatGroup(Ptr<CDockGroup> pGroup)
{
	Ptr<CDockFloatingWindow>	pFloating;

	if (pGroup.IsNull() || (pGroup->NumPanes() == 0))
	{
		return;
	}
	if (pGroup->GetPane(0)->GetState() == DPS_Floating)
	{
		pFloating = mpcManager->FindFloating(pGroup->GetPane(0));
		if (pFloating.IsNotNull())
		{
			DockFloating(pFloating);
		}
	}
	else
	{
		FloatGroup(pGroup);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane of the floating window goes back where it was docked; the
// tabs selected in its groups are still selected.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::DockFloating(Ptr<CDockFloatingWindow> pFloating)
{
	CArray<CDockPane>	aPanes;
	CArray<CDockPane>	aSelected;
	size				i;
	Ptr<CDockNode>		pRoot;
	Ptr<CDockPane>		pPane;

	pRoot = pFloating->GetRoot();
	if (pRoot.IsNull())
	{
		return;
	}
	aPanes.Init();
	aSelected.Init();
	pRoot->GetPanes(&aPanes);
	for (i = 0; i < aPanes.NumElements(); i++)
	{
		pPane = aPanes.Get(i);
		if (pPane->GetGroup()->GetSelectedPane() == pPane)
		{
			aSelected.Add(pPane);
		}
	}
	for (i = 0; i < aPanes.NumElements(); i++)
	{
		pPane = aPanes.Get(i);
		Dock(pPane);
	}
	for (i = 0; i < aSelected.NumElements(); i++)
	{
		pPane = aSelected.Get(i);
		if (pPane->GetState() == DPS_Docked)
		{
			pPane->GetGroup()->SelectPane(pPane);
		}
	}
	aSelected.Kill();
	aPanes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::CloseFloating(Ptr<CDockFloatingWindow> pFloating)
{
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockNode>		pRoot;

	pRoot = pFloating->GetRoot();
	if (pRoot.IsNull())
	{
		mpcManager->RemoveFloating(pFloating);
		return;
	}
	aPanes.Init();
	pRoot->GetPanes(&aPanes);
	for (i = 0; i < aPanes.NumElements(); i++)
	{
		mpcManager->Close(aPanes.Get(i));
	}
	aPanes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The edge of the main area nearest the group.  A group touching two
// edges goes to the one it runs along: a wide group to the bottom or top,
// a tall one to the left or right.
//
//////////////////////////////////////////////////////////////////////////
EDockSide CDockMover::NearestEdge(Ptr<CDockGroup> pGroup)
{
	const EDockSide*	aeOrder;
	CInt32Rect			cRoot;
	CInt32Rect			cGroup;
	EDockSide			eBest;
	int32				iBest;
	int32				iDistance;
	int					i;

	mpcState->GetArea()->GetRootBounds(&cRoot);
	pGroup->GetActualBounds(&cGroup);
	if ((cGroup.GetWidth() <= 0) || (cGroup.GetHeight() <= 0))
	{
		//Not laid out yet: the side of the well it is on.
		return mpcState->GetTree()->SideOfWell(pGroup);
	}

	aeOrder = (cGroup.GetWidth() > cGroup.GetHeight()) ? gaeWideGroupEdges : gaeTallGroupEdges;
	eBest = aeOrder[0];
	iBest = DistanceToEdge(&cGroup, &cRoot, eBest);
	for (i = 1; i < 4; i++)
	{
		iDistance = DistanceToEdge(&cGroup, &cRoot, aeOrder[i]);
		if (iDistance < iBest)
		{
			iBest = iDistance;
			eBest = aeOrder[i];
		}
	}
	return eBest;
}


//////////////////////////////////////////////////////////////////////////
//
// How far pcGroup is from pcRoot's eSide edge.
//
//////////////////////////////////////////////////////////////////////////
int32 CDockMover::DistanceToEdge(CInt32Rect* pcGroup, CInt32Rect* pcRoot, EDockSide eSide)
{
	switch (eSide)
	{
	case DS_Left:
		return pcGroup->GetLeft() - pcRoot->GetLeft();
	case DS_Right:
		return (pcRoot->GetLeft() + pcRoot->GetWidth()) - (pcGroup->GetLeft() + pcGroup->GetWidth());
	case DS_Top:
		return pcGroup->GetTop() - pcRoot->GetTop();
	default:
		return (pcRoot->GetTop() + pcRoot->GetHeight()) - (pcGroup->GetTop() + pcGroup->GetHeight());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A screen rectangle for floating pComponent: its size where it is now,
// or the pane's last floating rectangle.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::FloatRectangleFor(Ptr<CComponent> pComponent, Ptr<CDockPane> pPane, CInt32Rect* pcDest)
{
	CInt32Rect	cScreen;
	CInt32Rect	cBounds;
	CInt32Rect	cWindow;
	int32		iWidth;
	int32		iHeight;

	if (pPane.IsNotNull() && pPane->GetFloatRectangle(pcDest))
	{
		return;
	}
	iWidth = DOCK_FLOAT_WIDTH;
	iHeight = DOCK_FLOAT_HEIGHT;
	cBounds.Init(0, 0, 0, 0);
	if (pComponent.IsNotNull())
	{
		pComponent->GetActualBounds(&cBounds);
		if ((cBounds.GetWidth() > DOCK_MINIMUM_SIZE) && (cBounds.GetHeight() > DOCK_MINIMUM_SIZE))
		{
			iWidth = cBounds.GetWidth() + 2 * mpcManager->GetMetric(TM_DockFrameBorder);
			iHeight = cBounds.GetHeight() + 2 * mpcManager->GetMetric(TM_DockFrameBorder);
		}
		pComponent->GetWindow()->GetScreenRectangle(&cWindow);
		pcDest->Init(cWindow.GetLeft() + cBounds.GetLeft(), cWindow.GetTop() + cBounds.GetTop(), cWindow.GetLeft() + cBounds.GetLeft() + iWidth, cWindow.GetTop() + cBounds.GetTop() + iHeight);
		return;
	}
	mpcState->GetWindow()->GetScreenRectangle(&cScreen);
	pcDest->Init(cScreen.GetLeft() + DOCK_FLOAT_OFFSET, cScreen.GetTop() + DOCK_FLOAT_OFFSET, cScreen.GetLeft() + DOCK_FLOAT_OFFSET + iWidth, cScreen.GetTop() + DOCK_FLOAT_OFFSET + iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves, and if it must shrinks, pcRect onto the screen.  Nothing is done
// if the screen's area isn't known.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::KeepOnScreen(CInt32Rect* pcRect)
{
	CInt32Rect	cScreen;
	int32		iLeft;
	int32		iTop;
	int32		iWidth;
	int32		iHeight;
	int32		iScreenRight;
	int32		iScreenBottom;

	if (!mpcState->GetWindow()->GetScreenArea(&cScreen))
	{
		return;
	}
	iWidth = pcRect->GetWidth();
	iHeight = pcRect->GetHeight();
	if (iWidth > cScreen.GetWidth())
	{
		iWidth = cScreen.GetWidth();
	}
	if (iHeight > cScreen.GetHeight())
	{
		iHeight = cScreen.GetHeight();
	}
	iScreenRight = cScreen.GetLeft() + cScreen.GetWidth();
	iScreenBottom = cScreen.GetTop() + cScreen.GetHeight();
	iLeft = pcRect->GetLeft();
	iTop = pcRect->GetTop();
	if (iLeft + iWidth > iScreenRight)
	{
		iLeft = iScreenRight - iWidth;
	}
	if (iTop + iHeight > iScreenBottom)
	{
		iTop = iScreenBottom - iHeight;
	}
	if (iLeft < cScreen.GetLeft())
	{
		iLeft = cScreen.GetLeft();
	}
	if (iTop < cScreen.GetTop())
	{
		iTop = cScreen.GetTop();
	}
	pcRect->Init(iLeft, iTop, iLeft + iWidth, iTop + iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// A floating window, at pcScreenRectangle, not yet shown.  It is kept on
// the screen.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockFloatingWindow> CDockMover::CreateFloating(CInt32Rect* pcScreenRectangle)
{
	Ptr<CDockFloatingWindow>	pFloating;
	CInt32Rect					cRect;

	cRect.Init(pcScreenRectangle);
	KeepOnScreen(&cRect);
	pFloating = OMalloc<CDockFloatingWindow>(Ptr<CDockManager>(mpcManager), &cRect);
	mpcState->GetFloating()->Add(pFloating);
	return pFloating;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the pane out of wherever it is shown: its group, its auto hide
// strip and flyout.  An empty group comes out of its tree and an empty
// floating window closes.  A docked pane remembers where it was if
// bRecordRestore.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::TakeOut(Ptr<CDockPane> pPane, bool bRecordRestore)
{
	Ptr<CDockGroup>			pGroup;
	Ptr<CAutoHideStrip>		pStrip;

	if ((pPane->GetState() == DPS_Docked) && bRecordRestore)
	{
		mpcState->GetRestorer()->Record(pPane);
	}
	if (mpcState->GetFocus()->IsInFlyout(pPane))
	{
		mpcManager->CloseFlyout();
	}
	if (pPane->GetState() == DPS_AutoHidden)
	{
		pStrip = mpcState->GetArea()->GetStrip(pPane->GetAutoHideSide());
		if (pStrip.IsNotNull())
		{
			pStrip->RemovePane(pPane);
			mpcState->GetArea()->StripsChanged();
		}
	}
	pGroup = pPane->GetGroup();
	if (pGroup.IsNotNull())
	{
		pPane->GetContent()->GetWindow()->GetFocus()->Forget(pPane->GetContent());
		pGroup->TakePane(pPane);
		if (pGroup->NumPanes() == 0)
		{
			if (mpcState->GetDocumentGroup() == pGroup)
			{
				mpcState->SetDocumentGroup(NULL);
			}
			mpcState->GetTree()->RemoveNode(pGroup);
		}
	}
	pPane->SetGroup(NULL);
	pPane->SetState(DPS_Hidden);
	if (pPane->GetContent()->GetWindow() != mpcState->GetWindow())
	{
		pPane->GetContent()->MoveToWindow(mpcState->GetWindow());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane is taken out, the floating windows closed and the main area
// left with the empty document well.
//
//////////////////////////////////////////////////////////////////////////
void CDockMover::ClearLayout(void)
{
	size			i;
	Ptr<CDockPane>	pPane;

	mpcManager->CloseFlyout();
	for (i = 0; i < mpcState->GetPanes()->NumElements(); i++)
	{
		pPane = mpcState->GetPanes()->Get(i);
		if (pPane->IsShown())
		{
			TakeOut(pPane, false);
		}
	}
	while (mpcState->GetFloating()->NumElements() > 0)
	{
		mpcManager->RemoveFloating(mpcState->GetFloating()->Get(mpcState->GetFloating()->NumElements() - 1));
	}
	mpcState->GetWell()->SetRoot(NULL);
	mpcState->GetArea()->SetRoot(mpcState->GetWell());
	mpcState->GetWell()->SetDockSize(0);
	mpcState->SetDocumentGroup(NULL);
	mpcState->GetFocus()->Clear();
}
