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
#include "AutoHideFlyout.h"
#include "DockManager.h"
#include "DockDrag.h"
#include "DockTree.h"
#include "DockLayoutFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockTarget::Init(void)
{
	eKind = DTK_None;
	eSide = DS_None;
	pcArea = NULL;
	pcNode = NULL;
	cPreview.Init(0, 0, 0, 0);
}


bool SDockTarget::IsValid(void) { return eKind != DTK_None; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SDockTarget::Equals(SDockTarget* psOther)
{
	return (eKind == psOther->eKind) && (eSide == psOther->eSide) && (pcArea == psOther->pcArea) && (pcNode == psOther->pcNode);
}


bool			CDockManagerListener::PaneClosing(CDockPane* pcPane) { return true; }
void			CDockManagerListener::PaneClosed(CDockPane* pcPane) {}
void			CDockManagerListener::ActivePaneChanged(CDockPane* pcPane) {}
void			CDockManagerListener::DocumentSelected(CDockPane* pcPane) {}
Ptr<CDockPane>	CDockManagerListener::CreatePane(const char* szID) { return NULL; }
void			CDockManagerListener::LayoutChanged(void) {}
bool			CDockManagerListener::IsPaneModified(CDockPane* pcPane) { return false; }
void			CDockManagerListener::DocumentMenuCreated(Ptr<CMenuContainer> pMenu) {}


//////////////////////////////////////////////////////////////////////////
//
// pWindow is the main window; put GetArea in it.  pTurnedFont is pTheme's
// font turned clockwise (FT_Clockwise) for the left and right auto hide
// strips' text; NULL draws it upright.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTurnedFont)
{
	PreInit();

	mpTheme = pTheme;
	mpFont = pTheme->GetFont();
	mpTurnedFont = pTurnedFont;
	mpState = OMalloc<CDockState>(pWindow);
	mpState->MakeParts(this);
	WatchFocus(pWindow);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Class(void)
{
	M_Pointer(mpTheme);
	M_Pointer(mpFont);
	M_Pointer(mpTurnedFont);
	M_Pointer(mpState);
}


//////////////////////////////////////////////////////////////////////////
//
// The floating windows come off the screen.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Free(void)
{
	size	i;

	if (mpState.IsNull())
	{
		return;
	}
	for (i = 0; i < mpState->GetFloating()->NumElements(); i++)
	{
		mpState->GetFloating()->Get(i)->Close();
	}
	if (mpState->GetWindow().IsNotNull())
	{
		UnwatchFocus(mpState->GetWindow());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Layouts are written as XML, see SaveLayout.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The active pane follows the focus in pWindow.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::WatchFocus(Ptr<CWindow> pWindow)
{
	mpState->GetFocus()->Watch(pWindow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::UnwatchFocus(Ptr<CWindow> pWindow)
{
	mpState->GetFocus()->Unwatch(pWindow);
}


//////////////////////////////////////////////////////////////////////////
//
// A group's ID that isn't in use, or iID if it isn't.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockManager::CreateGroup(bool bDocuments, int32 iID)
{
	Ptr<CDockGroup>	pGroup;

	iID = mpState->TakeGroupID(FindGroup(iID).IsNotNull() ? 0 : iID);
	pGroup = OMalloc<CDockGroup>(mpState->GetWindow(), Ptr<CDockManager>(this), iID, bDocuments);
	return pGroup;
}


//////////////////////////////////////////////////////////////////////////
//
// The group with the ID, docked or floating, NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockManager::FindGroup(int32 iID)
{
	size			i;
	Ptr<CDockGroup>	pGroup;

	if (iID <= 0)
	{
		return NULL;
	}
	for (i = 0; i < mpState->GetPanes()->NumElements(); i++)
	{
		pGroup = mpState->GetPanes()->Get(i)->GetGroup();
		if (pGroup.IsNotNull() && (pGroup->GetID() == iID))
		{
			return pGroup;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a pane and, if bShow, shows it in its default place (psDefault,
// see SDockPlace).  Returns NULL if a pane with the ID is already added.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockManager::AddPane(const char* szID, const char* szTitle, Ptr<CImageCel> pIcon, Ptr<CComponent> pContent, EDockPaneKind eKind, SDockPlace* psDefault, bool bShow)
{
	Ptr<CDockPane>	pPane;

	if (GetPane(szID).IsNotNull())
	{
		gcLogger.Error2(__METHOD__, " A pane [", szID, "] is already added.", NULL);
		return NULL;
	}
	pContent->SetAlignment(CSH_Stretch, CSV_Stretch);
	pPane = OMalloc<CDockPane>(szID, szTitle, pIcon, pContent, eKind, psDefault);
	pPane->SetState(DPS_Hidden);
	pPane->SetManager(this);
	mpState->GetPanes()->Add(pPane);
	if (bShow && (pPane->GetDefaultKind() != DPL_Hidden))
	{
		mpState->GetRestorer()->DockDefault(pPane);
	}
	return pPane;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane is taken from wherever it is and let go.  The application
// still has its content.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::RemovePane(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull() || !mpState->GetPanes()->Contains(pPane))
	{
		return false;
	}
	mpState->GetMover()->TakeOut(pPane, false);
	mpState->GetFocus()->Forget(pPane);
	pPane->SetState(DPS_Removed);
	pPane->SetManager(NULL);
	mpState->GetPanes()->Remove(pPane);
	LayoutChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there is no pane with the ID.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockManager::GetPane(const char* szID)
{
	size	i;

	if (szID == NULL)
	{
		return NULL;
	}
	for (i = 0; i < mpState->GetPanes()->NumElements(); i++)
	{
		if (StringCompare(mpState->GetPanes()->Get(i)->GetID(), szID) == 0)
		{
			return mpState->GetPanes()->Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane pComponent is in, NULL if it isn't in one.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockManager::FindPane(Ptr<CComponent> pComponent)
{
	size	i;

	if (pComponent.IsNull())
	{
		return NULL;
	}
	for (i = 0; i < mpState->GetPanes()->NumElements(); i++)
	{
		if (pComponent->IsWithin(mpState->GetPanes()->Get(i)->GetContent()))
		{
			return mpState->GetPanes()->Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane goes into the document well: into pGroup, or the group the
// last document went into, or the well's first group, or a new group.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AddToWell(Ptr<CDockPane> pPane, Ptr<CDockGroup> pGroup)
{
	if (pGroup.IsNull() || !IsInWell(pGroup))
	{
		pGroup = mpState->GetDocumentGroup();
	}
	if (pGroup.IsNull() || !IsInWell(pGroup))
	{
		pGroup = mpState->GetWell()->GetFirstGroup();
	}
	if (pGroup.IsNull())
	{
		pGroup = CreateGroup(true);
		mpState->GetWell()->SetRoot(pGroup);
	}
	PlaceInGroup(pGroup, pPane);
	mpState->SetDocumentGroup(pGroup);
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The floating window comes off the screen and out of the manager.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::RemoveFloating(Ptr<CDockFloatingWindow> pFloating)
{
	pFloating->Close();
	mpState->GetFloating()->Remove(pFloating);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the pane where it was: docked back, slid out, or the floating
// window raised.  Its tab is selected.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Show(Ptr<CDockPane> pPane)
{
	Ptr<CDockFloatingWindow>	pFloating;

	if (pPane.IsNull())
	{
		return;
	}
	switch (pPane->GetState())
	{
	case DPS_Hidden:
		mpState->GetRestorer()->Restore(pPane);
		LayoutChanged();
		break;
	case DPS_AutoHidden:
		ShowFlyout(pPane);
		break;
	case DPS_Floating:
		pFloating = FindFloating(pPane);
		if (pFloating.IsNotNull())
		{
			pFloating->GetWindow()->Raise();
		}
		break;
	default:
		break;
	}
	if (pPane->GetGroup().IsNotNull())
	{
		pPane->GetGroup()->SelectPane(pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pane can be seen: docked or floating (its window shown) and
// its group's selected tab, or auto hidden and slid out.  Not hidden,
// behind another tab, or auto hidden to its strip.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsPaneVisible(Ptr<CDockPane> pPane)
{
	Ptr<CDockFloatingWindow>	pFloating;

	if (pPane.IsNull())
	{
		return false;
	}
	switch (pPane->GetState())
	{
	case DPS_AutoHidden:
		return mpState->GetFocus()->IsInFlyout(pPane);
	case DPS_Floating:
		pFloating = FindFloating(pPane);
		if (pFloating.IsNull() || !pFloating->IsShown())
		{
			return false;
		}
		break;
	case DPS_Docked:
		break;
	default:
		return false;
	}
	return pPane->GetGroup().IsNotNull() && (pPane->GetGroup()->GetSelectedPane() == pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the pane and gives it the keys.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Activate(Ptr<CDockPane> pPane)
{
	Show(pPane);
	mpState->GetFocus()->FocusPane(pPane);
	mpState->GetFocus()->SetActivePane(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// The keys go back to the active pane in pWindow, after a sash or edge
// took them to be dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::RestoreFocus(Ptr<CWindow> pWindow)
{
	mpState->GetFocus()->RestoreFocus(pWindow);
}


//////////////////////////////////////////////////////////////////////////
//
// A tool is hidden; it remembers where it was.  A document is closed.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Hide(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull() || !pPane->IsShown())
	{
		return;
	}
	mpState->GetMover()->TakeOut(pPane, true);
	if (mpState->GetFocus()->GetActivePane() == pPane)
	{
		mpState->GetFocus()->SetActivePane(NULL);
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A document is closed if the listener lets it: it is removed and the
// listener told.  A tool is hidden.  Returns false if it is kept.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::Close(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull())
	{
		return false;
	}
	if (pPane->IsTool())
	{
		Hide(pPane);
		if (mpState->GetListener())
		{
			mpState->GetListener()->PaneClosed(&pPane);
		}
		return true;
	}
	if (mpState->GetListener() && !mpState->GetListener()->PaneClosing(&pPane))
	{
		return false;
	}
	RemovePane(pPane);
	if (mpState->GetListener())
	{
		mpState->GetListener()->PaneClosed(&pPane);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Closes documents in pPane's group, as eItem says: all of them, all but
// pPane, those the listener says aren't modified, or those to the left or
// right of pPane's tab.  Each is closed as Close does (the listener can
// keep it).  Returns how many closed.
//
//////////////////////////////////////////////////////////////////////////
size CDockManager::CloseMultiple(Ptr<CDockPane> pPane, EDockMenuItem eItem)
{
	Ptr<CDockGroup>	pGroup;
	CArrayChars		aszIDs;
	size			uiIndex;
	size			i;
	size			uiClosed;
	bool			bClose;

	pGroup = pPane->GetGroup();
	if (pGroup.IsNull())
	{
		return 0;
	}

	//The panes' IDs, as closing one changes the group.
	uiIndex = pGroup->IndexOfPane(pPane);
	aszIDs.Init();
	for (i = 0; i < pGroup->NumPanes(); i++)
	{
		switch (eItem)
		{
		case DMI_CloseAll:
			bClose = true;
			break;
		case DMI_CloseOthers:
			bClose = (i != uiIndex);
			break;
		case DMI_CloseUnmodified:
			bClose = !IsPaneModified(pGroup->GetPane(i));
			break;
		case DMI_CloseLeft:
			bClose = (i < uiIndex);
			break;
		case DMI_CloseRight:
			bClose = (i > uiIndex);
			break;
		default:
			bClose = false;
			break;
		}
		if (bClose && pGroup->GetPane(i)->IsDocument())
		{
			aszIDs.Add((char*)pGroup->GetPane(i)->GetID());
		}
	}

	uiClosed = 0;
	for (i = 0; i < aszIDs.NumElements(); i++)
	{
		if (Close(GetPane(aszIDs.GetText(i))))
		{
			uiClosed++;
		}
	}
	aszIDs.Kill();
	return uiClosed;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the application says the document has changes.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsPaneModified(Ptr<CDockPane> pPane)
{
	return mpState->GetListener() ? mpState->GetListener()->IsPaneModified(&pPane) : false;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane floats in a window of its own, at pcScreenRectangle or where it
// last floated.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Float(Ptr<CDockPane> pPane, CInt32Rect* pcScreenRectangle)
{
	mpState->GetMover()->Float(pPane, pcScreenRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
// The whole group floats.  Its panes remember where it was docked.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FloatGroup(Ptr<CDockGroup> pGroup, CInt32Rect* pcScreenRectangle)
{
	mpState->GetMover()->FloatGroup(pGroup, pcScreenRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
// The pane goes back to where it was docked, from floating, auto hidden or
// hidden.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Dock(Ptr<CDockPane> pPane)
{
	mpState->GetMover()->Dock(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// A tool goes into the document well as a tab, as Visual Studio's Dock as
// Tabbed Document puts it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockAsDocument(Ptr<CDockPane> pPane)
{
	mpState->GetMover()->DockAsDocument(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// A docked tool goes to a tab on the main window's edge nearest it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AutoHide(Ptr<CDockPane> pPane)
{
	mpState->GetMover()->AutoHide(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the group auto hides on the same edge, remembering the
// group, so docking one back brings the group back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AutoHideGroup(Ptr<CDockGroup> pGroup)
{
	mpState->GetMover()->AutoHideGroup(pGroup);
}


//////////////////////////////////////////////////////////////////////////
//
// A docked pane floats; a floating one docks back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ToggleFloat(Ptr<CDockPane> pPane)
{
	mpState->GetMover()->ToggleFloat(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ToggleFloatGroup(Ptr<CDockGroup> pGroup)
{
	mpState->GetMover()->ToggleFloatGroup(pGroup);
}


//////////////////////////////////////////////////////////////////////////
//
// Everything in the floating window docks back where it was.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockFloating(Ptr<CDockFloatingWindow> pFloating)
{
	mpState->GetMover()->DockFloating(pFloating);
}


//////////////////////////////////////////////////////////////////////////
//
// The floating window's close cross: its tools are hidden and its
// documents closed, if the listener lets them.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseFloating(Ptr<CDockFloatingWindow> pFloating)
{
	mpState->GetMover()->CloseFloating(pFloating);
}


//////////////////////////////////////////////////////////////////////////
//
// The platform closed a floating window.  It can't be shown again as it
// was; its panes are closed as by its close cross, and any left (a
// document kept) are docked back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FloatingWindowClosed(CWindow* pcWindow)
{
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;

	for (i = 0; i < mpState->GetFloating()->NumElements(); i++)
	{
		pFloating = mpState->GetFloating()->Get(i);
		if (&pFloating->GetWindow() == pcWindow)
		{
			CloseFloating(pFloating);
			if (mpState->GetFloating()->Contains(pFloating))
			{
				DockFloating(pFloating);
			}
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The floating window the pane is in, NULL if it isn't floating.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockFloatingWindow> CDockManager::FindFloating(Ptr<CDockPane> pPane)
{
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CDockGroup>				pGroup;

	pGroup = pPane->GetGroup();
	if (pGroup.IsNull())
	{
		return NULL;
	}
	for (i = 0; i < mpState->GetFloating()->NumElements(); i++)
	{
		pFloating = mpState->GetFloating()->Get(i);
		if (pGroup->IsWithin(pFloating->GetArea()))
		{
			return pFloating;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's window position menu.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::MenuChosen(Ptr<CDockPane> pPane, EDockMenuItem eItem)
{
	switch (eItem)
	{
	case DMI_Float:
		Float(pPane);
		break;
	case DMI_Dock:
		Dock(pPane);
		break;
	case DMI_DockAsDocument:
		DockAsDocument(pPane);
		break;
	case DMI_AutoHide:
		AutoHide(pPane);
		break;
	case DMI_Hide:
		Hide(pPane);
		break;
	case DMI_Close:
		Close(pPane);
		break;
	case DMI_CloseAll:
	case DMI_CloseOthers:
	case DMI_CloseUnmodified:
	case DMI_CloseLeft:
	case DMI_CloseRight:
		CloseMultiple(pPane, eItem);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A group's selected tab changed.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::PaneSelected(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>				pGroup;
	Ptr<CDockFloatingWindow>	pFloating;

	pGroup = pPane->GetGroup();
	if (pGroup.IsNotNull() && pGroup->IsActive())
	{
		mpState->GetFocus()->SetActivePane(pPane);
	}
	if (pPane->IsDocument() || (pGroup.IsNotNull() && pGroup->IsDocumentGroup()))
	{
		if (pGroup.IsNotNull() && IsInWell(pGroup))
		{
			mpState->SetDocumentGroup(pGroup);
		}
		mpState->GetFocus()->SetActiveDocument(pPane);
	}
	pFloating = FindFloating(pPane);
	if (pFloating.IsNotNull())
	{
		pFloating->UpdateTitle();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pane's title, icon or tab font changed: its tab and title bar, its
// auto hide tab and flyout and its floating window show it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::PaneChanged(Ptr<CDockPane> pPane)
{
	Ptr<CAutoHideStrip>			pStrip;
	Ptr<CAutoHideTab>			pTab;
	Ptr<CDockFloatingWindow>	pFloating;

	if (pPane->GetGroup().IsNotNull())
	{
		pPane->GetGroup()->PaneChanged(pPane);
	}
	if (pPane->GetState() == DPS_AutoHidden)
	{
		pStrip = mpState->GetArea()->GetStrip(pPane->GetAutoHideSide());
		pTab = pStrip.IsNotNull() ? pStrip->FindTab(pPane) : Ptr<CAutoHideTab>();
		if (pTab.IsNotNull())
		{
			pTab->TitleChanged();
		}
	}
	if (mpState->GetFocus()->IsInFlyout(pPane))
	{
		GetFlyout()->GetTitleBar()->SetTitle(pPane->GetTitle());
	}
	pFloating = FindFloating(pPane);
	if (pFloating.IsNotNull())
	{
		pFloating->UpdateTitle();
	}
	mpState->GetWindow()->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Control+F4: the active document closes, as Close closes it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseActiveDocument(void)
{
	Ptr<CDockPane>	pDocument;

	pDocument = mpState->GetFocus()->GetActiveDocument();
	if (pDocument.IsNotNull())
	{
		Close(pDocument);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift+Escape: the active pane, if it is a tool, is hidden.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::HideActiveTool(void)
{
	Ptr<CDockPane>	pPane;

	pPane = mpState->GetFocus()->GetActivePane();
	if (pPane.IsNotNull() && pPane->IsTool())
	{
		Hide(pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Slides an auto hidden pane out over the main window.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ShowFlyout(Ptr<CDockPane> pPane)
{
	mpState->GetFocus()->ShowFlyout(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Slides the pane back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseFlyout(void)
{
	mpState->GetFocus()->CloseFlyout();
}


//////////////////////////////////////////////////////////////////////////
//
// The flyout's popup closed, by a press outside it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FlyoutClosed(void)
{
	mpState->GetFocus()->FlyoutClosed();
}


//////////////////////////////////////////////////////////////////////////
//
// Something about the layout changed, to save it later.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::LayoutChanged(void)
{
	mpState->GetWindow()->InvalidateLayout();
	if (mpState->GetListener())
	{
		mpState->GetListener()->LayoutChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The panes in the floating windows, then every pane, go back to their
// default places; floating windows close.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ResetLayout(void)
{
	size			i;
	Ptr<CDockPane>	pPane;

	mpState->GetMover()->ClearLayout();
	for (i = 0; i < mpState->GetPanes()->NumElements(); i++)
	{
		pPane = mpState->GetPanes()->Get(i);
		pPane->ClearRestore();
		if (pPane->IsTool() && (pPane->GetDefaultKind() != DPL_Hidden))
		{
			mpState->GetRestorer()->DockDefault(pPane);
		}
	}
	for (i = 0; i < mpState->GetPanes()->NumElements(); i++)
	{
		pPane = mpState->GetPanes()->Get(i);
		if (pPane->IsDocument())
		{
			AddToWell(pPane);
		}
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockArea>				CDockManager::GetArea(void) { return mpState->GetArea(); }
Ptr<CDocumentWell>			CDockManager::GetWell(void) { return mpState->GetWell(); }
Ptr<CWindow>				CDockManager::GetWindow(void) { return mpState->GetWindow(); }
void						CDockManager::SetListener(CDockManagerListener* pcListener) { mpState->SetListener(pcListener); }
Ptr<CWindowTheme>			CDockManager::GetTheme(void) { return mpTheme; }
Ptr<CDockState>				CDockManager::GetState(void) { return mpState; }
Ptr<CDockPane>				CDockManager::GetMenuPane(void) { return mpState->GetMenuPane(); }  //The pane a tab or title bar menu was opened for last: what its items are for.
Ptr<CFont>					CDockManager::GetFont(void) { return mpFont; }
Ptr<CFont>					CDockManager::GetTurnedFont(void) { return mpTurnedFont; }
ARGB32						CDockManager::GetColour(EThemeColour eColour) { return mpTheme->GetColour(eColour); }
int32						CDockManager::GetMetric(EThemeMetric eMetric) { return mpTheme->GetMetric(eMetric); }
void						CDockManager::SetColour(EThemeColour eColour, ARGB32 sColour) { mpTheme->SetColour(eColour, sColour); }
size						CDockManager::NumPanes(void) { return mpState->GetPanes()->NumElements(); }
Ptr<CDockPane>				CDockManager::GetPane(size uiIndex) { return mpState->GetPanes()->Get(uiIndex); }
Ptr<CDockPane>				CDockManager::GetActivePane(void) { return mpState->GetFocus()->GetActivePane(); }
Ptr<CDockPane>				CDockManager::GetActiveDocument(void) { return mpState->GetFocus()->GetActiveDocument(); }
size						CDockManager::NumFloating(void) { return mpState->GetFloating()->NumElements(); }
Ptr<CDockFloatingWindow>	CDockManager::GetFloating(size uiIndex) { return mpState->GetFloating()->Get(uiIndex); }
Ptr<CAutoHideFlyout>		CDockManager::GetFlyout(void) { return mpState->GetFocus()->GetFlyout(); }
SDockTarget*				CDockManager::GetTarget(void) { return mpState->GetDrag()->GetTarget(); }
Ptr<CDockDrag>				CDockManager::GetDrag(void) { return mpState->GetDrag(); }
bool						CDockManager::IsDragging(void) { return mpState->GetDrag()->IsDragging(); }
void						CDockManager::SetDragNoDock(bool bNoDock) { mpState->GetDrag()->SetNoDock(bNoDock); }
void						CDockManager::BeginPaneDrag(Ptr<CDockPane> pPane) { mpState->GetDrag()->BeginPane(pPane); }
void						CDockManager::BeginGroupDrag(Ptr<CDockGroup> pGroup) { mpState->GetDrag()->BeginGroup(pGroup); }
void						CDockManager::BeginFloatingDrag(Ptr<CDockFloatingWindow> pFloating) { mpState->GetDrag()->BeginFloating(pFloating); }


//////////////////////////////////////////////////////////////////////////
//
// The node is in the document well's tree.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsInWell(Ptr<CDockNode> pNode)
{
	return pNode.IsNotNull() && (pNode != mpState->GetWell()) && pNode->IsWithin(mpState->GetWell());
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
	mpState->GetMover()->TakeOut(pPane, true);
	pGroup = CreateGroup(pPane->IsDocument());
	pGroup->AddPane(pPane);
	return DockNodeAt(pGroup, psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// See CDockTree::DockAt.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::DockNodeAt(Ptr<CDockNode> pNode, SDockTarget* psTarget)
{
	return mpState->GetTree()->DockAt(pNode, psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// The main window's client area on the screen in the last layout read.
// Returns false if there wasn't one.  ReadLayout applies it to the main
// window.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::GetLayoutWindowRectangle(CInt32Rect* pcDest)
{
	return mpState->GetLayoutWindow(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the layout to szFileName, see CDockLayoutWriter.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::SaveLayout(const char* szFileName)
{
	CDockLayoutWriter	cWriter;
	bool				bResult;

	cWriter.Init(this);
	bResult = cWriter.Save(szFileName);
	cWriter.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::WriteLayout(CXMLWriter* pcWriter)
{
	CDockLayoutWriter	cWriter;

	cWriter.Init(this);
	cWriter.Write(pcWriter);
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the layout from szFileName, see CDockLayoutReader.  A file that
// isn't there, or doesn't read, leaves the layout as it is and returns
// false.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::LoadLayout(const char* szFileName)
{
	CDockLayoutReader	cReader;
	bool				bResult;

	cReader.Init(this);
	bResult = cReader.Load(szFileName);
	cReader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::ReadLayout(CMarkupTag* pcRoot)
{
	CDockLayoutReader	cReader;
	bool				bResult;

	cReader.Init(this);
	bResult = cReader.Read(pcRoot);
	cReader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane becomes a tab of the group: docked in the main window's area,
// floating in a floating window's.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::PlaceInGroup(Ptr<CDockGroup> pGroup, Ptr<CDockPane> pPane, size uiIndex)
{
	pGroup->AddPane(pPane, uiIndex);
	pPane->SetState((pGroup->GetWindow() == mpState->GetWindow()) ? DPS_Docked : DPS_Floating);
}

