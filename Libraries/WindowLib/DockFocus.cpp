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
#include "AutoHideFlyout.h"
#include "DockManager.h"
#include "DockFocus.h"


//////////////////////////////////////////////////////////////////////////
//
// pcManager holds the panes, groups and windows.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Init(CDockManager* pcManager)
{
	mpcManager = pcManager;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The active pane follows the focus in pWindow.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Watch(Ptr<CWindow> pWindow)
{
	pWindow->GetFocus()->GetFocusChangeEvent()->Remove(this);
	pWindow->GetFocus()->GetFocusChangeEvent()->Add(this, &CDockFocus::FocusChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Unwatch(Ptr<CWindow> pWindow)
{
	pWindow->GetFocus()->GetFocusChangeEvent()->Remove(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The pane whose content has the focus is the active pane.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::FocusChanged(CFocus* pcFocus)
{
	Ptr<CComponent>	pFocussed;
	Ptr<CDockPane>	pPane;

	if (mpcManager->IsDragging())
	{
		return;
	}
	pFocussed = pcFocus->GetFocussedComponent();
	if (mpcManager->mpFlyout.IsNotNull() && pFocussed.IsNotNull() && IsOutsideFlyout(pFocussed))
	{
		CloseFlyout();
	}
	pPane = mpcManager->FindPane(pFocussed);
	if (pPane.IsNotNull())
	{
		SetActivePane(pPane);
		if (pPane->IsDocument() || (pPane->GetGroup().IsNotNull() && pPane->GetGroup()->IsDocumentGroup()))
		{
			SetActiveDocument(pPane);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys have gone somewhere that slides the flyout back, as Visual
// Studio's does when its window loses focus: anywhere but the flyout, a
// popup (its pane's menu, say) or the strip tab of its pane, which slides
// it back itself.
//
//////////////////////////////////////////////////////////////////////////
bool CDockFocus::IsOutsideFlyout(Ptr<CComponent> pFocussed)
{
	Ptr<CComponent>	pTop;
	CAutoHideTab*	pcTab;

	if (pFocussed->IsWithin(mpcManager->mpFlyout))
	{
		return false;
	}
	pcTab = dynamic_cast<CAutoHideTab*>(&pFocussed);
	if (pcTab && (pcTab->GetPane() == mpcManager->mpFlyout->GetPane()))
	{
		return false;
	}
	pTop = pFocussed;
	while (pTop->GetParent().IsNotNull())
	{
		pTop = pTop->GetParent();
	}
	return dynamic_cast<CPopup*>(&pTop) == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Its group's title bar is highlighted.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::SetActivePane(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>	pOld;
	Ptr<CDockGroup>	pNew;

	if (pPane == mpcManager->mpActivePane)
	{
		return;
	}
	if (mpcManager->mpActivePane.IsNotNull())
	{
		pOld = mpcManager->mpActivePane->GetGroup();
	}
	if (pPane.IsNotNull())
	{
		pNew = pPane->GetGroup();
	}
	if (pOld.IsNotNull() && (pOld != pNew))
	{
		pOld->SetActive(false);
	}
	if (pNew.IsNotNull())
	{
		pNew->SetActive(true);
	}
	mpcManager->mpActivePane = pPane;
	if (mpcManager->mpcListener)
	{
		mpcManager->mpcListener->ActivePaneChanged(&pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::SetActiveDocument(Ptr<CDockPane> pPane)
{
	if (pPane == mpcManager->mpActiveDocument)
	{
		return;
	}
	mpcManager->mpActiveDocument = pPane;
	if (mpcManager->mpcListener)
	{
		mpcManager->mpcListener->DocumentSelected(&pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first component in pComponent that can take the keys, depth first.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CDockFocus::FindFocusable(Ptr<CComponent> pComponent)
{
	size			i;
	Ptr<CComponent>	pFound;

	if (pComponent->CanGetFocus())
	{
		return pComponent;
	}
	for (i = 0; i < pComponent->NumComponents(); i++)
	{
		pFound = FindFocusable(pComponent->GetComponent(i));
		if (pFound.IsNotNull())
		{
			return pFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::FocusPane(Ptr<CDockPane> pPane)
{
	Ptr<CComponent>	pFocusable;
	Ptr<CWindow>	pWindow;

	if (pPane.IsNull() || !pPane->IsShown())
	{
		return;
	}
	pFocusable = FindFocusable(pPane->GetContent());
	pWindow = pPane->GetContent()->GetWindow();
	if (pFocusable.IsNotNull())
	{
		pWindow->GetFocus()->SetFocus(pFocusable);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys go back to the active pane in pWindow, after a sash or edge
// took them to be dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::RestoreFocus(Ptr<CWindow> pWindow)
{
	if (mpcManager->mpActivePane.IsNotNull() && (mpcManager->mpActivePane->GetContent()->GetWindow() == pWindow))
	{
		FocusPane(mpcManager->mpActivePane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Slides an auto hidden pane out over the main window.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::ShowFlyout(Ptr<CDockPane> pPane)
{
	Ptr<CAutoHideStrip>	pStrip;
	Ptr<CAutoHideTab>	pTab;
	CInt32Rect			cRoot;
	SInt32Vec2			sPosition;

	if (pPane.IsNull() || (pPane->GetState() != DPS_AutoHidden))
	{
		return;
	}
	if (mpcManager->mpFlyout.IsNotNull() && (mpcManager->mpFlyout->GetPane() == pPane))
	{
		return;
	}
	CloseFlyout();
	pStrip = mpcManager->mpArea->GetStrip(pPane->GetAutoHideSide());
	pTab = pStrip->FindTab(pPane);
	if (pTab.IsNull())
	{
		return;
	}
	mpcManager->mpFlyout = OMalloc<CAutoHideFlyout>(mpcManager->mpWindow, Ptr<CDockManager>(mpcManager), pPane);
	mpcManager->mpArea->GetRootBounds(&cRoot);
	mpcManager->mpFlyout->Place(&cRoot, &sPosition);
	mpcManager->mpWindow->OpenPopup(mpcManager->mpFlyout, sPosition, pTab);
	FocusPane(pPane);
	SetActivePane(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Slides the pane back.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::CloseFlyout(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	if (mpcManager->mpFlyout.IsNull())
	{
		return;
	}
	pFlyout = mpcManager->mpFlyout;
	mpcManager->mpFlyout = NULL;
	mpcManager->mpWindow->ClosePopup(pFlyout);
	pFlyout->Detach();
}


//////////////////////////////////////////////////////////////////////////
//
// The flyout's popup closed, by a press outside it.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::FlyoutClosed(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	if (mpcManager->mpFlyout.IsNull())
	{
		return;
	}
	pFlyout = mpcManager->mpFlyout;
	mpcManager->mpFlyout = NULL;
	pFlyout->Detach();
	if (mpcManager->mpActivePane == pFlyout->GetPane())
	{
		SetActivePane(NULL);
	}
}

