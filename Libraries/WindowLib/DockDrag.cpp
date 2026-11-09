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
#include "ComponentInput.h"
#include "DockArea.h"
#include "DockSplit.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "DockEdge.h"
#include "DockGuides.h"
#include "DockManager.h"
#include "DockDrag.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Init(Ptr<CDockManager> pManager, CDockState* pcState)
{
	PreInit();

	mpManager = pManager;
	mpcState = pcState;
	mpFloating = NULL;
	mpSource = NULL;
	mpTracker = NULL;
	msGrab.Init(0, 0);
	mcStart.Init(0, 0, 0, 0);
	mcFull.Init(0, 0, 0, 0);
	mbTornOff = false;
	mbCancelled = false;
	mbNoDock = false;
	msTarget.Init();
	mpGuidesArea = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Class(void)
{
	M_Pointer(mpManager);
	U_Pointer(mpcState);
	M_Pointer(mpFloating);
	M_Pointer(mpSource);
	M_Pointer(mpTracker);
	U_2Int32(msGrab);
	U_Data(CInt32Rect, mcStart);
	U_Data(CInt32Rect, mcFull);
	U_Bool(mbTornOff);
	U_Bool(mbCancelled);
	U_Bool(mbNoDock);
	U_Unknown(SDockTarget, msTarget);
	M_Pointer(mpGuidesArea);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A drag tracker takes the drag going on in pSource, before what is
// dragged is torn off and the tab or title bar that started it goes.
// Returns false, keeping nothing, if no drag is going on there.
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::Take(Ptr<CWindow> pSource)
{
	Ptr<CFocus>				pFocus;
	Ptr<CDockDragTracker>	pTracker;

	pFocus = pSource->GetFocus();
	if (pFocus->GetDrag() != FD_Select)
	{
		return false;
	}
	pTracker = OMalloc<CDockDragTracker>(pSource, mpManager);
	mpTracker = pTracker;
	mpSource = pSource;
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
void CDockDrag::BeginPane(Ptr<CDockPane> pPane)
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
	pFloating = mpManager->FindFloating(pPane);
	if (pFloating.IsNotNull() && pFloating->GetRoot()->IsGroup() && (pFloating->GetRoot()->GetFirstGroup()->NumPanes() == 1))
	{
		BeginFloating(pFloating);
		return;
	}

	pSource = pPane->GetGroup()->GetWindow();
	if (!Take(pSource))
	{
		return;
	}

	pSource->GetPointerScreenPosition(&sPointer);
	mpcState->GetMover()->FloatRectangleFor(pPane->GetGroup(), NULL, &cRect);
	iWidth = cRect.GetWidth();
	iHeight = cRect.GetHeight();
	sGrab.Init((iWidth > 2 * DOCK_GRAB_LEFT) ? DOCK_GRAB_LEFT : iWidth / 2, mpManager->GetMetric(TM_DockFrameBorder) + mpManager->GetMetric(TM_DockTitleBarHeight) / 2);
	cRect.Init(sPointer.x - sGrab.x, sPointer.y - sGrab.y, sPointer.x - sGrab.x + iWidth, sPointer.y - sGrab.y + iHeight);
	mpManager->Float(pPane, &cRect);
	Start(mpManager->FindFloating(pPane), sGrab, pSource, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A tool group's title bar pulled: the whole group floats where it is and
// is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::BeginGroup(Ptr<CDockGroup> pGroup)
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
	pFloating = mpManager->FindFloating(pGroup->GetPane(0));
	if (pFloating.IsNotNull() && (pFloating->GetRoot() == pGroup))
	{
		BeginFloating(pFloating);
		return;
	}

	pSource = pGroup->GetWindow();
	if (!Take(pSource))
	{
		return;
	}

	pSource->GetPointerScreenPosition(&sPointer);
	mpcState->GetMover()->FloatRectangleFor(pGroup, NULL, &cRect);
	sGrab.Init(sPointer.x - cRect.GetLeft(), sPointer.y - cRect.GetTop());
	mpManager->FloatGroup(pGroup, &cRect);
	Start(mpManager->FindFloating(pGroup->GetPane(0)), sGrab, pSource, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A floating window's title bar pulled: the window is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::BeginFloating(Ptr<CDockFloatingWindow> pFloating)
{
	SInt32Vec2		sPointer;
	CInt32Rect		cRect;
	SInt32Vec2		sGrab;

	if (pFloating.IsNull())
	{
		return;
	}
	if (mpTracker.IsNull() && !Take(pFloating->GetWindow()))
	{
		return;
	}
	mpSource->GetPointerScreenPosition(&sPointer);
	pFloating->GetRectangle(&cRect);
	sGrab.Init(sPointer.x - cRect.GetLeft(), sPointer.y - cRect.GetTop());
	Start(pFloating, sGrab, mpSource, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The tracker has the drag; the floating window follows the pointer, held
// at sGrab, until it is dropped.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Start(Ptr<CDockFloatingWindow> pFloating, SInt32Vec2 sGrab, Ptr<CWindow> pSource, bool bTornOff)
{
	if (pFloating.IsNull())
	{
		End();
		return;
	}
	mpFloating = pFloating;
	mpSource = pSource;
	msGrab = sGrab;
	mbTornOff = bTornOff;
	mbCancelled = false;
	mbNoDock = false;
	msTarget.Init();
	pFloating->GetRectangle(&mcStart);
	mcFull.Init(&mcStart);
	pFloating->GetWindow()->Raise();
	Moved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::IsDragging(void)
{
	return mpTracker.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
// While set, dropping leaves what is dragged floating.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::SetNoDock(bool bNoDock)
{
	mbNoDock = bNoDock;
}


//////////////////////////////////////////////////////////////////////////
//
// Control is held: the drag shows no guides and doesn't dock, as in
// Visual Studio, so a window can be floated over the others.
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::IsControlHeld(void)
{
	CComponentInput*	pcInput;

	pcInput = mpSource.IsNotNull() ? mpSource->GetComponentInput() : NULL;
	return (pcInput != NULL) && pcInput->IsControl();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDrag::IsDraggingDocuments(void)
{
	return mpFloating.IsNotNull() && mpFloating->HasDocuments();
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer moved: the floating window follows it and the guides of the
// window under it show where it would go, with the floating window small
// so it doesn't cover them.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Moved(void)
{
	SInt32Vec2	sPointer;
	CInt32Rect	cRect;
	int32		iWidth;
	int32		iHeight;

	if (mpFloating.IsNull() || mpSource.IsNull())
	{
		return;
	}
	mpSource->GetPointerScreenPosition(&sPointer);
	iWidth = mcFull.GetWidth();
	iHeight = mcFull.GetHeight();
	mcFull.Init(sPointer.x - msGrab.x, sPointer.y - msGrab.y, sPointer.x - msGrab.x + iWidth, sPointer.y - msGrab.y + iHeight);
	if (mbNoDock || IsControlHeld())
	{
		HideGuides();
		msTarget.Init();
	}
	else
	{
		FindTarget(sPointer, &msTarget);
	}

	if (mpGuidesArea.IsNotNull())
	{
		iHeight = mpManager->GetMetric(TM_DockFrameBorder) * 2 + mpManager->GetMetric(TM_DockTitleBarHeight);
		cRect.Init(sPointer.x + DOCK_DRAG_SMALL_OFFSET, sPointer.y + DOCK_DRAG_SMALL_OFFSET, sPointer.x + DOCK_DRAG_SMALL_OFFSET + DOCK_DRAG_SMALL_WIDTH, sPointer.y + DOCK_DRAG_SMALL_OFFSET + iHeight);
		mpFloating->Move(&cRect);
	}
	else
	{
		mpFloating->Move(&mcFull);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The window under sScreen, the floating windows over the main window,
// shows its guides; psTarget is the guide under the pointer, if any.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::FindTarget(SInt32Vec2 sScreen, SDockTarget* psTarget)
{
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;
	CInt32Rect					cScreen;
	SInt32Vec2					sClient;

	psTarget->Init();
	for (i = mpcState->GetFloating()->NumElements(); i > 0; i--)
	{
		pFloating = mpcState->GetFloating()->Get(i - 1);
		if ((pFloating == mpFloating) || !pFloating->IsShown())
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
	mpcState->GetWindow()->GetScreenRectangle(&cScreen);
	if (cScreen.ContainsPoint(sScreen.x, sScreen.y))
	{
		sClient.Init(sScreen.x - cScreen.GetLeft(), sScreen.y - cScreen.GetTop());
		if (mpcState->GetArea()->IsPointIn(sClient.x, sClient.y))
		{
			ShowGuides(psTarget, mpcState->GetArea(), sClient);
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
void CDockDrag::ShowGuides(SDockTarget* psTarget, Ptr<CDockArea> pArea, SInt32Vec2 sClient)
{
	Ptr<CDockGuides>	pGuides;
	SInt32Vec2			sDragged;

	if (mpGuidesArea != pArea)
	{
		HideGuides();
		pGuides = OMalloc<CDockGuides>(pArea->GetWindow(), mpManager, pArea);
		pArea->ShowGuides(pGuides);
		mpGuidesArea = pArea;
		pArea->GetWindow()->Layout();
	}
	pGuides = pArea->GetGuides();
	sDragged.Init(mcFull.GetWidth(), mcFull.GetHeight());
	pGuides->Build(sClient.x, sClient.y, IsDraggingDocuments(), sDragged);
	pGuides->Hit(sClient.x, sClient.y, psTarget, sDragged);
	pArea->GetWindow()->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::HideGuides(void)
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
void CDockDrag::Cancelled(void)
{
	mbCancelled = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The tracker lets go of the drag.
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::End(void)
{
	Ptr<CFocus>				pFocus;
	Ptr<CDockDragTracker>	pTracker;

	HideGuides();
	pTracker = mpTracker;
	mpTracker = NULL;
	mpFloating = NULL;
	mpSource = NULL;
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
void CDockDrag::Dropped(void)
{
	Ptr<CDockFloatingWindow>	pFloating;
	SDockTarget					sTarget;
	bool						bCancelled;
	bool						bTornOff;
	Ptr<CDockPane>				pSelected;
	Ptr<CDockGroup>				pGroup;
	CInt32Rect					cRect;
	CInt32Rect					cFull;

	if (!IsDragging())
	{
		return;
	}
	pFloating = mpFloating;
	sTarget = msTarget;
	bCancelled = mbCancelled;
	bTornOff = mbTornOff;
	cRect.Init(&mcStart);
	cFull.Init(&mcFull);
	End();
	if (pFloating.IsNull() || pFloating->GetRoot().IsNull())
	{
		return;
	}
	pGroup = pFloating->GetRoot()->GetFirstGroup();
	if (pGroup.IsNotNull())
	{
		pSelected = pGroup->GetSelectedPane();
	}

	Drop(pFloating, &sTarget, bCancelled, bTornOff, &cRect, &cFull);
	mpManager->LayoutChanged();
	if (pSelected.IsNotNull() && pSelected->IsShown())
	{
		mpManager->Activate(pSelected);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the dropped floating window where the drag left it: back where it
// started, or docked again if it was torn off, when cancelled (pcStart);
// docked at a valid target; else it stays floating, at its own size where
// the pointer has it (pcFull).
//
//////////////////////////////////////////////////////////////////////////
void CDockDrag::Drop(Ptr<CDockFloatingWindow> pFloating, SDockTarget* psTarget, bool bCancelled, bool bTornOff, CInt32Rect* pcStart, CInt32Rect* pcFull)
{
	Ptr<CDockNode>	pRoot;

	if (bCancelled)
	{
		if (bTornOff)
		{
			mpManager->DockFloating(pFloating);
		}
		else
		{
			pFloating->Move(pcStart);
		}
	}
	else if (psTarget->IsValid())
	{
		pRoot = pFloating->TakeRoot();
		mpManager->RemoveFloating(pFloating);
		mpManager->DockNodeAt(pRoot, psTarget);
	}
	else
	{
		pFloating->Move(pcFull);
		pFloating->GetRoot()->SetPaneFloatRectangles(pcFull);
	}
}


SDockTarget*				CDockDrag::GetTarget(void) { return &msTarget; }
Ptr<CDockFloatingWindow>	CDockDrag::GetFloating(void) { return mpFloating; }

