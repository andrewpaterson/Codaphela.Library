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
#include "AutoHide.h"
#include "AutoHideFlyout.h"
#include "DockManager.h"
#include "DockFocus.h"


//////////////////////////////////////////////////////////////////////////
//
// pManager holds the panes, groups and windows.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Init(Ptr<CDockManager> pManager, CDockState* pcState)
{
	PreInit();

	mpManager = pManager;
	mpcState = pcState;
	mpActivePane = NULL;
	mpActiveDocument = NULL;
	mpFlyout = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Class(void)
{
	M_Pointer(mpManager);
	U_Pointer(mpcState);
	M_Pointer(mpActivePane);
	M_Pointer(mpActiveDocument);
	M_Pointer(mpFlyout);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFocus::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockFocus::Load(CObjectReader* pcFile)
{
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
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

	if (mpManager->IsDragging())
	{
		return;
	}
	pFocussed = pcFocus->GetFocussedComponent();
	if (mpFlyout.IsNotNull() && pFocussed.IsNotNull() && IsOutsideFlyout(pFocussed))
	{
		CloseFlyout();
	}
	pPane = mpManager->FindPane(pFocussed);
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

	if (pFocussed->IsWithin(mpFlyout))
	{
		return false;
	}
	pcTab = dynamic_cast<CAutoHideTab*>(&pFocussed);
	if (pcTab && (pcTab->GetPane() == mpFlyout->GetPane()))
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

	if (pPane == mpActivePane)
	{
		return;
	}
	if (mpActivePane.IsNotNull())
	{
		pOld = mpActivePane->GetGroup();
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
	mpActivePane = pPane;
	if (mpcState->GetListener())
	{
		mpcState->GetListener()->ActivePaneChanged(&pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::SetActiveDocument(Ptr<CDockPane> pPane)
{
	if (pPane == mpActiveDocument)
	{
		return;
	}
	mpActiveDocument = pPane;
	if (mpcState->GetListener())
	{
		mpcState->GetListener()->DocumentSelected(&pPane);
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
	pFocusable = pPane->GetFocusTarget();
	if (pFocusable.IsNull() || !pFocusable->IsWithin(pPane->GetContent()))
	{
		pFocusable = FindFocusable(pPane->GetContent());
	}
	pWindow = pPane->GetContent()->GetWindow();
	if (pFocusable.IsNotNull())
	{
		pWindow->GetFocus()->SetFocus(pFocusable);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::RestoreFocus(Ptr<CWindow> pWindow)
{
	if (mpActivePane.IsNotNull() && (mpActivePane->GetContent()->GetWindow() == pWindow))
	{
		FocusPane(mpActivePane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
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
	if (IsInFlyout(pPane))
	{
		return;
	}
	CloseFlyout();
	pStrip = mpcState->GetArea()->GetStrip(pPane->GetAutoHideSide());
	pTab = pStrip->FindTab(pPane);
	if (pTab.IsNull())
	{
		return;
	}
	mpFlyout = OMalloc<CAutoHideFlyout>(mpcState->GetWindow(), mpManager, pPane);
	mpcState->GetArea()->GetRootBounds(&cRoot);
	mpFlyout->Place(&cRoot, &sPosition);
	mpcState->GetWindow()->OpenPopup(mpFlyout, sPosition, pTab);
	FocusPane(pPane);
	SetActivePane(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::CloseFlyout(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	if (mpFlyout.IsNull())
	{
		return;
	}
	pFlyout = mpFlyout;
	mpFlyout = NULL;
	mpcState->GetWindow()->ClosePopup(pFlyout);
	pFlyout->Detach();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::FlyoutClosed(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	if (mpFlyout.IsNull())
	{
		return;
	}
	pFlyout = mpFlyout;
	mpFlyout = NULL;
	pFlyout->Detach();
	if (mpActivePane == pFlyout->GetPane())
	{
		SetActivePane(NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pane is going: it is no longer the active pane or document.  The
// document goes quietly, as there is no other to select.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Forget(Ptr<CDockPane> pPane)
{
	if (mpActivePane == pPane)
	{
		SetActivePane(NULL);
	}
	if (mpActiveDocument == pPane)
	{
		mpActiveDocument = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// No pane is active, as when the layout is cleared.
//
//////////////////////////////////////////////////////////////////////////
void CDockFocus::Clear(void)
{
	SetActivePane(NULL);
	mpActiveDocument = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The flyout is open showing pPane.
//
//////////////////////////////////////////////////////////////////////////
bool CDockFocus::IsInFlyout(Ptr<CDockPane> pPane)
{
	return mpFlyout.IsNotNull() && (mpFlyout->GetPane() == pPane);
}


Ptr<CDockPane>			CDockFocus::GetActivePane(void) { return mpActivePane; }
Ptr<CDockPane>			CDockFocus::GetActiveDocument(void) { return mpActiveDocument; }
Ptr<CAutoHideFlyout>	CDockFocus::GetFlyout(void) { return mpFlyout; }
