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
#include "JewelColours.h"
#include "JewelImageGenerator.h"
#include "DockArea.h"
#include "DockSplit.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "DockEdge.h"
#include "DockGuides.h"
#include "AutoHide.h"
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

	mpWindow = pWindow;
	mpTheme = pTheme;
	mpFont = pTheme->GetFont();
	mpTurnedFont = pTurnedFont;
	maImages.Init();
	CreateImages();

	maPanes.Init();
	maFloating.Init();
	miNextGroupID = 1;
	mpActivePane = NULL;
	mpActiveDocument = NULL;
	mpDocumentGroup = NULL;
	mpcListener = NULL;
	mpFlyout = NULL;
	mcLayoutWindow.Init(0, 0, 0, 0);
	mbLayoutWindow = false;

	mpDrag = OMalloc<CDockDrag>(Ptr<CDockManager>(this));
	mcTree.Init(this);
	mpArea = OMalloc<CDockArea>(pWindow, Ptr<CDockManager>(this), true);
	mpWell = OMalloc<CDocumentWell>(pWindow, Ptr<CDockManager>(this), GetColour(DC_Background));
	mpArea->SetRoot(mpWell);
	WatchFocus(pWindow);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CreateImages(void)
{
	CColourFormatHelper	cFormat;

	mpWindow->GetColourFormat(&cFormat);
	maImages.RemoveAll();
	maImages.Add(CJewelImageGenerator::CreateDownChevron(JEWEL_ICON, &cFormat));
	maImages.Add(CJewelImageGenerator::CreatePin(JEWEL_ICON, true, &cFormat));
	maImages.Add(CJewelImageGenerator::CreatePin(JEWEL_ICON, false, &cFormat));
	maImages.Add(CJewelImageGenerator::CreateCross(JEWEL_ICON, &cFormat));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpTheme);
	M_Pointer(mpFont);
	M_Pointer(mpTurnedFont);
	M_Embedded(maImages);
	M_Pointer(mpArea);
	M_Pointer(mpWell);
	M_Embedded(maPanes);
	M_Embedded(maFloating);
	U_Int32(miNextGroupID);
	M_Pointer(mpActivePane);
	M_Pointer(mpActiveDocument);
	M_Pointer(mpDocumentGroup);
	U_Pointer(mpcListener);
	M_Pointer(mpDrag);
	U_Data(CDockTree, mcTree);
	M_Pointer(mpFlyout);
	U_Data(CInt32Rect, mcLayoutWindow);
	U_Bool(mbLayoutWindow);
}


//////////////////////////////////////////////////////////////////////////
//
// The floating windows come off the screen.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Free(void)
{
	size	i;

	for (i = 0; i < maFloating.Size(); i++)
	{
		maFloating.Get(i)->Close();
	}
	if (mpWindow.IsNotNull())
	{
		UnwatchFocus(mpWindow);
	}
	mcTree.Kill();
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
	pWindow->GetFocus()->GetFocusChangeEvent()->Remove(this);
	pWindow->GetFocus()->GetFocusChangeEvent()->Add(this, &CDockManager::FocusChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::UnwatchFocus(Ptr<CWindow> pWindow)
{
	pWindow->GetFocus()->GetFocusChangeEvent()->Remove(this);
}


//////////////////////////////////////////////////////////////////////////
//
// A group's ID that isn't in use, or iID if it isn't.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockManager::CreateGroup(bool bDocuments, int32 iID)
{
	Ptr<CDockGroup>	pGroup;

	if ((iID <= 0) || FindGroup(iID).IsNotNull())
	{
		iID = miNextGroupID;
	}
	if (iID >= miNextGroupID)
	{
		miNextGroupID = iID + 1;
	}
	pGroup = OMalloc<CDockGroup>(mpWindow, Ptr<CDockManager>(this), iID, bDocuments);
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
	for (i = 0; i < maPanes.Size(); i++)
	{
		pGroup = maPanes.Get(i)->GetGroup();
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
	maPanes.Add(pPane);
	if (bShow && (pPane->GetDefaultKind() != DPL_Hidden))
	{
		DockDefault(pPane);
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
	if (pPane.IsNull() || !maPanes.Contains(pPane))
	{
		return false;
	}
	TakeOut(pPane, false);
	if (mpActivePane == pPane)
	{
		SetActivePane(NULL);
	}
	if (mpActiveDocument == pPane)
	{
		mpActiveDocument = NULL;
	}
	pPane->SetState(DPS_Removed);
	pPane->SetManager(NULL);
	maPanes.Remove(pPane);
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
	for (i = 0; i < maPanes.Size(); i++)
	{
		if (StringCompare(maPanes.Get(i)->GetID(), szID) == 0)
		{
			return maPanes.Get(i);
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
	for (i = 0; i < maPanes.Size(); i++)
	{
		if (pComponent->IsWithin(maPanes.Get(i)->GetContent()))
		{
			return maPanes.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the pane out of wherever it is shown: its group, its auto hide
// strip and flyout.  An empty group comes out of its tree and an empty
// floating window closes.  A docked pane remembers where it was if
// bRecordRestore.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::TakeOut(Ptr<CDockPane> pPane, bool bRecordRestore)
{
	Ptr<CDockGroup>			pGroup;
	Ptr<CAutoHideStrip>		pStrip;

	if ((pPane->GetState() == DPS_Docked) && bRecordRestore)
	{
		RecordRestore(pPane);
	}
	if (mpFlyout.IsNotNull() && (mpFlyout->GetPane() == pPane))
	{
		CloseFlyout();
	}
	if (pPane->GetState() == DPS_AutoHidden)
	{
		pStrip = mpArea->GetStrip(pPane->GetAutoHideSide());
		if (pStrip.IsNotNull())
		{
			pStrip->RemovePane(pPane);
			mpArea->StripsChanged();
		}
	}
	pGroup = pPane->GetGroup();
	if (pGroup.IsNotNull())
	{
		pPane->GetContent()->GetWindow()->GetFocus()->Forget(pPane->GetContent());
		pGroup->TakePane(pPane);
		if (pGroup->NumPanes() == 0)
		{
			if (mpDocumentGroup == pGroup)
			{
				mpDocumentGroup = NULL;
			}
			mcTree.RemoveNode(pGroup);
		}
	}
	pPane->SetGroup(NULL);
	pPane->SetState(DPS_Hidden);
	if (pPane->GetContent()->GetWindow() != mpWindow)
	{
		pPane->GetContent()->MoveToWindow(mpWindow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where the group would go back to as a whole: its ID and the pane in the
// node beside it in its split, with the side of that node it was on.  A
// group beside the document well remembers the well's side.  Returns
// false for a group that isn't docked in the main area.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::GetGroupRestore(Ptr<CDockGroup> pGroup, SDockRestore* psDest)
{
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockNode>		pSibling;
	Ptr<CDockGroup>		pSiblingGroup;
	size				uiIndex;
	bool				bAfter;
	SInt32Vec2			sSize;

	psDest->iGroup = pGroup->GetID();
	psDest->iIndex = 0;
	psDest->SetNeighbour(NULL);
	psDest->eSide = DS_None;
	psDest->iSize = 0;
	psDest->bInWell = IsInWell(pGroup);

	pSplit = pGroup->GetParentSplit();
	if (pSplit.IsNull())
	{
		return pGroup->IsWithin(mpArea);
	}
	uiIndex = pSplit->IndexOf(pGroup);
	if (uiIndex + 1 < pSplit->NumNodes())
	{
		pSibling = pSplit->GetNode(uiIndex + 1);
		bAfter = false;
	}
	else if (uiIndex > 0)
	{
		pSibling = pSplit->GetNode(uiIndex - 1);
		bAfter = true;
	}
	else
	{
		return true;
	}

	sSize = pGroup->GetActualSize();
	psDest->iSize = pSplit->IsHorizontal() ? sSize.x : sSize.y;
	if (pSplit->IsHorizontal())
	{
		psDest->eSide = bAfter ? DS_Right : DS_Left;
	}
	else
	{
		psDest->eSide = bAfter ? DS_Bottom : DS_Top;
	}

	//The well, if the node beside holds it, as it is never hidden; otherwise a pane in it.  Restore climbs from
	//there back up to the node beside.
	if (pSibling->ContainsDocumentWell())
	{
		psDest->SetNeighbour(DOCK_WELL_NEIGHBOUR);
		return true;
	}
	pSiblingGroup = pSibling->GetFirstGroup();
	if (pSiblingGroup.IsNotNull() && (pSiblingGroup->NumPanes() > 0))
	{
		psDest->SetNeighbour(pSiblingGroup->GetPane(0)->GetID());
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the group remembers the group's place, as a whole, with
// its own tab's place in it: so the first docked back brings the group
// back and the others join it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::GiveGroupRestore(Ptr<CDockGroup> pGroup)
{
	SDockRestore	sRestore;
	size			i;

	sRestore.Init();
	if (GetGroupRestore(pGroup, &sRestore))
	{
		for (i = 0; i < pGroup->NumPanes(); i++)
		{
			sRestore.iIndex = (int32)i;
			pGroup->GetPane(i)->SetRestore(&sRestore);
		}
	}
	sRestore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A docked pane leaving its group remembers it: the group and its place
// in it, and a pane left in the group or, if it was the last, the group's
// place beside its neighbour.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::RecordRestore(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>	pGroup;
	size			uiIndex;
	SDockRestore	sRestore;

	pGroup = pPane->GetGroup();
	if (pGroup.IsNull())
	{
		return;
	}
	uiIndex = pGroup->IndexOfPane(pPane);
	sRestore.Init();
	if (pGroup->NumPanes() > 1)
	{
		sRestore.iGroup = pGroup->GetID();
		sRestore.iIndex = (int32)uiIndex;
		sRestore.SetNeighbour(pGroup->GetPane((uiIndex > 0) ? uiIndex - 1 : 1)->GetID());
		sRestore.eSide = DS_Centre;
		sRestore.bInWell = IsInWell(pGroup);
		pPane->SetRestore(&sRestore);
	}
	else if (GetGroupRestore(pGroup, &sRestore))
	{
		sRestore.iIndex = (int32)uiIndex;
		pPane->SetRestore(&sRestore);
	}
	sRestore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Puts a pane that isn't shown back where it was last docked: its group
// if it is still docked, else beside or with its neighbour, else in the
// well if it was there, else its default place.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Restore(Ptr<CDockPane> pPane)
{
	RestoreFrom(pPane, pPane, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Puts pPane where pFrom remembers being docked.  When pFrom's neighbour
// isn't docked either, auto hidden or hidden say, pPane goes where that
// neighbour would go back to, a few neighbours along at most: a pane
// hidden with the rest of its group comes back where the group was.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::RestoreFrom(Ptr<CDockPane> pPane, Ptr<CDockPane> pFrom, int iDepth)
{
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pNeighbour;
	Ptr<CDockGroup>	pNeighbourGroup;
	Ptr<CDockGroup>	pNew;
	Ptr<CDockNode>	pBeside;
	EDockSide		eSide;
	int32			iSize;
	bool			bInWell;
	SDockRestore*	psFrom;

	psFrom = pFrom->GetRestore();
	pGroup = FindGroup(psFrom->iGroup);
	if (pGroup.IsNotNull() && pGroup->IsWithin(mpArea) && (pGroup->IsDocumentGroup() || !pPane->IsDocument()))
	{
		PlaceInGroup(pGroup, pPane, (size)pPane->GetRestore()->iIndex);
		return;
	}

	eSide = psFrom->eSide;
	iSize = psFrom->iSize;
	if (psFrom->IsBesideWell())
	{
		pBeside = mcTree.ClimbToSplit(mpWell, eSide);
		pNew = CreateGroup(IsInWell(pBeside), psFrom->iGroup);
		PlaceInGroup(pNew, pPane);
		mcTree.DockBeside(pBeside, pNew, eSide, iSize);
		return;
	}
	pNeighbour = GetPane(psFrom->szNeighbour.Text());
	if (pNeighbour.IsNotNull() && (pNeighbour != pPane) && (pNeighbour->GetState() == DPS_Docked))
	{
		pNeighbourGroup = pNeighbour->GetGroup();
		if ((eSide == DS_Centre) || (eSide == DS_None))
		{
			if (pNeighbourGroup->IsDocumentGroup() || !pPane->IsDocument())
			{
				PlaceInGroup(pNeighbourGroup, pPane, (size)pPane->GetRestore()->iIndex);
				return;
			}
		}
		else
		{
			pBeside = mcTree.ClimbToSplit(pNeighbourGroup, eSide);
			bInWell = IsInWell(pBeside);
			pNew = CreateGroup(bInWell, psFrom->iGroup);
			PlaceInGroup(pNew, pPane);
			mcTree.DockBeside(pBeside, pNew, eSide, iSize);
			return;
		}
	}
	else if (pNeighbour.IsNotNull() && (pNeighbour != pPane) && (pNeighbour != pFrom) && pNeighbour->HasRestore() && (iDepth < DOCK_RESTORE_DEPTH))
	{
		RestoreFrom(pPane, pNeighbour, iDepth + 1);
		return;
	}
	if (psFrom->bInWell || pPane->IsDocument())
	{
		AddToWell(pPane);
		return;
	}
	DockDefault(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows a pane in its default place.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockDefault(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pWith;
	int32			iSize;

	switch (pPane->GetDefaultKind())
	{
	case DPL_WithPane:
		pWith = GetPane(pPane->GetDefaultPane());
		if (pWith.IsNotNull() && (pWith != pPane) && (pWith->GetState() == DPS_Docked) && (pWith->GetGroup()->IsDocumentGroup() || !pPane->IsDocument()))
		{
			PlaceInGroup(pWith->GetGroup(), pPane);
			return;
		}
		if (pWith.IsNotNull() && (pWith != pPane) && (pWith->GetState() == DPS_Hidden || pWith->GetState() == DPS_Removed) && (pWith->GetDefaultKind() == DPL_BesideWell))
		{
			pGroup = CreateGroup(false);
			PlaceInGroup(pGroup, pPane);
			iSize = pWith->GetDefaultSize();
			mcTree.DockBeside(mpWell, pGroup, pWith->GetDefaultSide(), (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
			return;
		}
		break;
	case DPL_InWell:
		AddToWell(pPane);
		return;
	case DPL_Hidden:
		pPane->SetState(DPS_Hidden);
		return;
	default:
		break;
	}
	if (pPane->IsDocument())
	{
		AddToWell(pPane);
		return;
	}
	pGroup = CreateGroup(false);
	PlaceInGroup(pGroup, pPane);
	iSize = pPane->GetDefaultSize();
	mcTree.DockBeside(mpWell, pGroup, (pPane->GetDefaultKind() == DPL_BesideWell) ? pPane->GetDefaultSide() : DS_Left, (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
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
		pGroup = mpDocumentGroup;
	}
	if (pGroup.IsNull() || !IsInWell(pGroup))
	{
		pGroup = mpWell->GetFirstGroup();
	}
	if (pGroup.IsNull())
	{
		pGroup = CreateGroup(true);
		mpWell->SetRoot(pGroup);
	}
	PlaceInGroup(pGroup, pPane);
	mpDocumentGroup = pGroup;
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
	maFloating.Remove(pFloating);
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the node takes the state, of a node docked in the main
// area or floating.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::SetStates(Ptr<CDockNode> pNode, EDockPaneState eState)
{
	pNode->SetPaneStates(eState);
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
		Restore(pPane);
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
// Shows the pane and gives it the keys.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Activate(Ptr<CDockPane> pPane)
{
	Show(pPane);
	FocusPane(pPane);
	SetActivePane(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// The first component in pComponent that can take the keys, depth first.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CDockManager::FindFocusable(Ptr<CComponent> pComponent)
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
void CDockManager::FocusPane(Ptr<CDockPane> pPane)
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
void CDockManager::RestoreFocus(Ptr<CWindow> pWindow)
{
	if (mpActivePane.IsNotNull() && (mpActivePane->GetContent()->GetWindow() == pWindow))
	{
		FocusPane(mpActivePane);
	}
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
	TakeOut(pPane, true);
	if (mpActivePane == pPane)
	{
		SetActivePane(NULL);
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
		if (mpcListener)
		{
			mpcListener->PaneClosed(&pPane);
		}
		return true;
	}
	if (mpcListener && !mpcListener->PaneClosing(&pPane))
	{
		return false;
	}
	RemovePane(pPane);
	if (mpcListener)
	{
		mpcListener->PaneClosed(&pPane);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A screen rectangle for floating pComponent: its size where it is now,
// or the pane's last floating rectangle.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FloatRectangleFor(Ptr<CComponent> pComponent, Ptr<CDockPane> pPane, CInt32Rect* pcDest)
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
			iWidth = cBounds.GetWidth() + 2 * DOCK_FRAME_BORDER;
			iHeight = cBounds.GetHeight() + 2 * DOCK_FRAME_BORDER;
		}
		pComponent->GetWindow()->GetScreenRectangle(&cWindow);
		pcDest->Init(cWindow.GetLeft() + cBounds.GetLeft(), cWindow.GetTop() + cBounds.GetTop(), cWindow.GetLeft() + cBounds.GetLeft() + iWidth, cWindow.GetTop() + cBounds.GetTop() + iHeight);
		return;
	}
	mpWindow->GetScreenRectangle(&cScreen);
	pcDest->Init(cScreen.GetLeft() + DOCK_FLOAT_OFFSET, cScreen.GetTop() + DOCK_FLOAT_OFFSET, cScreen.GetLeft() + DOCK_FLOAT_OFFSET + iWidth, cScreen.GetTop() + DOCK_FLOAT_OFFSET + iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves, and if it must shrinks, pcRect onto the screen.  Nothing is done
// if the screen's area isn't known.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::KeepOnScreen(CInt32Rect* pcRect)
{
	CInt32Rect	cScreen;
	int32		iLeft;
	int32		iTop;
	int32		iWidth;
	int32		iHeight;
	int32		iScreenRight;
	int32		iScreenBottom;

	if (!mpWindow->GetScreenArea(&cScreen))
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
Ptr<CDockFloatingWindow> CDockManager::CreateFloating(CInt32Rect* pcScreenRectangle)
{
	Ptr<CDockFloatingWindow>	pFloating;
	CInt32Rect					cRect;

	cRect.Init(pcScreenRectangle);
	KeepOnScreen(&cRect);
	pFloating = OMalloc<CDockFloatingWindow>(Ptr<CDockManager>(this), &cRect);
	maFloating.Add(pFloating);
	return pFloating;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane floats in a window of its own, at pcScreenRectangle or where it
// last floated.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Float(Ptr<CDockPane> pPane, CInt32Rect* pcScreenRectangle)
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
		pFloating = FindFloating(pPane);
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
	pGroup = CreateGroup(pPane->IsDocument());
	pGroup->MoveToWindow(pFloating->GetWindow());
	pPane->GetContent()->MoveToWindow(pFloating->GetWindow());
	pGroup->AddPane(pPane);
	pFloating->SetRoot(pGroup);
	pPane->SetState(DPS_Floating);
	pPane->SetFloatRectangle(&cRect);
	pFloating->Show();
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The whole group floats.  Its panes remember where it was docked.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FloatGroup(Ptr<CDockGroup> pGroup, CInt32Rect* pcScreenRectangle)
{
	CInt32Rect					cRect;
	Ptr<CDockFloatingWindow>	pFloating;
	size						i;

	if (pGroup.IsNull() || (pGroup->NumPanes() == 0))
	{
		return;
	}
	pFloating = FindFloating(pGroup->GetPane(0));
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

	CloseFlyout();
	if (pGroup->IsWithin(mpArea))
	{
		GiveGroupRestore(pGroup);
	}
	if (mpDocumentGroup == pGroup)
	{
		mpDocumentGroup = NULL;
	}
	mcTree.RemoveNode(pGroup);

	pFloating = CreateFloating(&cRect);
	pGroup->MoveToWindow(pFloating->GetWindow());
	pFloating->SetRoot(pGroup);
	SetStates(pGroup, DPS_Floating);
	for (i = 0; i < pGroup->NumPanes(); i++)
	{
		pGroup->GetPane(i)->SetFloatRectangle(&cRect);
	}
	pFloating->Show();
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The pane goes back to where it was docked, from floating, auto hidden or
// hidden.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Dock(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull() || (pPane->GetState() == DPS_Docked) || (pPane->GetState() == DPS_Removed))
	{
		return;
	}
	TakeOut(pPane, false);
	Restore(pPane);
	pPane->GetGroup()->SelectPane(pPane);
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A tool goes into the document well as a tab, as Visual Studio's Dock as
// Tabbed Document puts it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockAsDocument(Ptr<CDockPane> pPane)
{
	if (pPane.IsNull())
	{
		return;
	}
	TakeOut(pPane, true);
	AddToWell(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// The edge of the main area nearest the group.
//
//////////////////////////////////////////////////////////////////////////
EDockSide CDockManager::NearestEdge(Ptr<CDockGroup> pGroup)
{
	CInt32Rect	cRoot;
	CInt32Rect	cGroup;
	int32		iLeft;
	int32		iRight;
	int32		iTop;
	int32		iBottom;
	int32		iBest;
	EDockSide	eSide;

	mpArea->GetRootBounds(&cRoot);
	pGroup->GetActualBounds(&cGroup);
	if ((cGroup.GetWidth() <= 0) || (cGroup.GetHeight() <= 0))
	{
		//Not laid out yet: the side of the well it is on.
		return mcTree.SideOfWell(pGroup);
	}
	iLeft = cGroup.GetLeft() - cRoot.GetLeft();
	iRight = (cRoot.GetLeft() + cRoot.GetWidth()) - (cGroup.GetLeft() + cGroup.GetWidth());
	iTop = cGroup.GetTop() - cRoot.GetTop();
	iBottom = (cRoot.GetTop() + cRoot.GetHeight()) - (cGroup.GetTop() + cGroup.GetHeight());
	//A group touching two edges goes to the one it runs along: a wide group to the top or bottom.
	if (cGroup.GetWidth() > cGroup.GetHeight())
	{
		eSide = (iBottom <= iTop) ? DS_Bottom : DS_Top;
		iBest = (iBottom <= iTop) ? iBottom : iTop;
		if (iLeft < iBest)
		{
			iBest = iLeft;
			eSide = DS_Left;
		}
		if (iRight < iBest)
		{
			eSide = DS_Right;
		}
	}
	else
	{
		eSide = (iLeft <= iRight) ? DS_Left : DS_Right;
		iBest = (iLeft <= iRight) ? iLeft : iRight;
		if (iBottom < iBest)
		{
			iBest = iBottom;
			eSide = DS_Bottom;
		}
		if (iTop < iBest)
		{
			eSide = DS_Top;
		}
	}
	return eSide;
}


//////////////////////////////////////////////////////////////////////////
//
// A docked tool goes to a tab on the main window's edge nearest it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AutoHide(Ptr<CDockPane> pPane)
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
	if (IsInWell(pGroup))
	{
		return;
	}
	eSide = NearestEdge(pGroup);
	sSize = pGroup->GetActualSize();
	iSize = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	TakeOut(pPane, true);
	pPane->SetAutoHide(eSide, (iSize > DOCK_MINIMUM_SIZE) ? iSize : DOCK_DEFAULT_SIZE);
	pStrip = mpArea->GetStrip(eSide);
	pStrip->AddPane(pPane);
	pPane->SetState(DPS_AutoHidden);
	mpArea->StripsChanged();
	if (mpActivePane == pPane)
	{
		SetActivePane(NULL);
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the group auto hides on the same edge, remembering the
// group, so docking one back brings the group back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AutoHideGroup(Ptr<CDockGroup> pGroup)
{
	CArray<CDockPane>		aPanes;
	size					i;
	Ptr<CDockPane>			pPane;
	EDockSide				eSide;
	int32					iSize;
	SInt32Vec2				sSize;
	Ptr<CAutoHideStrip>		pStrip;

	if (pGroup.IsNull() || pGroup->IsDocumentGroup() || !pGroup->IsWithin(mpArea) || IsInWell(pGroup))
	{
		return;
	}
	eSide = NearestEdge(pGroup);
	sSize = pGroup->GetActualSize();
	iSize = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	GiveGroupRestore(pGroup);

	aPanes.Init();
	pGroup->GetPanes(&aPanes);
	pStrip = mpArea->GetStrip(eSide);
	for (i = 0; i < aPanes.Size(); i++)
	{
		pPane = aPanes.Get(i);
		TakeOut(pPane, false);
		pPane->SetAutoHide(eSide, (iSize > DOCK_MINIMUM_SIZE) ? iSize : DOCK_DEFAULT_SIZE);
		pStrip->AddPane(pPane);
		pPane->SetState(DPS_AutoHidden);
		if (mpActivePane == pPane)
		{
			SetActivePane(NULL);
		}
	}
	aPanes.Kill();
	mpArea->StripsChanged();
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A docked pane floats; a floating one docks back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ToggleFloat(Ptr<CDockPane> pPane)
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
void CDockManager::ToggleFloatGroup(Ptr<CDockGroup> pGroup)
{
	Ptr<CDockFloatingWindow>	pFloating;

	if (pGroup.IsNull() || (pGroup->NumPanes() == 0))
	{
		return;
	}
	if (pGroup->GetPane(0)->GetState() == DPS_Floating)
	{
		pFloating = FindFloating(pGroup->GetPane(0));
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
// Everything in the floating window docks back where it was.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockFloating(Ptr<CDockFloatingWindow> pFloating)
{
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockNode>		pRoot;
	Ptr<CDockPane>		pPane;

	pRoot = pFloating->GetRoot();
	if (pRoot.IsNull())
	{
		return;
	}
	aPanes.Init();
	pRoot->GetPanes(&aPanes);
	for (i = 0; i < aPanes.Size(); i++)
	{
		pPane = aPanes.Get(i);
		Dock(pPane);
	}
	aPanes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The floating window's close cross: its tools are hidden and its
// documents closed, if the listener lets them.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseFloating(Ptr<CDockFloatingWindow> pFloating)
{
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockNode>		pRoot;

	pRoot = pFloating->GetRoot();
	if (pRoot.IsNull())
	{
		RemoveFloating(pFloating);
		return;
	}
	aPanes.Init();
	pRoot->GetPanes(&aPanes);
	for (i = 0; i < aPanes.Size(); i++)
	{
		Close(aPanes.Get(i));
	}
	aPanes.Kill();
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

	for (i = 0; i < maFloating.Size(); i++)
	{
		pFloating = maFloating.Get(i);
		if (&pFloating->GetWindow() == pcWindow)
		{
			CloseFloating(pFloating);
			if (maFloating.Contains(pFloating))
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
	for (i = 0; i < maFloating.Size(); i++)
	{
		pFloating = maFloating.Get(i);
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
		SetActivePane(pPane);
	}
	if (pPane->IsDocument() || (pGroup.IsNotNull() && pGroup->IsDocumentGroup()))
	{
		if (pGroup.IsNotNull() && IsInWell(pGroup))
		{
			mpDocumentGroup = pGroup;
		}
		SetActiveDocument(pPane);
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
		pStrip = mpArea->GetStrip(pPane->GetAutoHideSide());
		pTab = pStrip.IsNotNull() ? pStrip->FindTab(pPane) : Ptr<CAutoHideTab>();
		if (pTab.IsNotNull())
		{
			pTab->TitleChanged();
		}
	}
	if (mpFlyout.IsNotNull() && (mpFlyout->GetPane() == pPane))
	{
		mpFlyout->GetTitleBar()->SetTitle(pPane->GetTitle());
	}
	pFloating = FindFloating(pPane);
	if (pFloating.IsNotNull())
	{
		pFloating->UpdateTitle();
	}
	mpWindow->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Control+F4: the active document closes, as Close closes it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseActiveDocument(void)
{
	if (mpActiveDocument.IsNotNull())
	{
		Close(mpActiveDocument);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift+Escape: the active pane, if it is a tool, is hidden.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::HideActiveTool(void)
{
	if (mpActivePane.IsNotNull() && mpActivePane->IsTool())
	{
		Hide(mpActivePane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pane whose content has the focus is the active pane.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FocusChanged(CFocus* pcFocus)
{
	Ptr<CComponent>	pFocussed;
	Ptr<CDockPane>	pPane;

	if (IsDragging())
	{
		return;
	}
	pFocussed = pcFocus->GetFocussedComponent();
	if (mpFlyout.IsNotNull() && pFocussed.IsNotNull() && IsFocusOutsideFlyout(pFocussed))
	{
		CloseFlyout();
	}
	pPane = FindPane(pFocussed);
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
bool CDockManager::IsFocusOutsideFlyout(Ptr<CComponent> pFocussed)
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
void CDockManager::SetActivePane(Ptr<CDockPane> pPane)
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
	if (mpcListener)
	{
		mpcListener->ActivePaneChanged(&pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::SetActiveDocument(Ptr<CDockPane> pPane)
{
	if (pPane == mpActiveDocument)
	{
		return;
	}
	mpActiveDocument = pPane;
	if (mpcListener)
	{
		mpcListener->DocumentSelected(&pPane);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Slides an auto hidden pane out over the main window.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ShowFlyout(Ptr<CDockPane> pPane)
{
	Ptr<CAutoHideStrip>	pStrip;
	Ptr<CAutoHideTab>	pTab;
	CInt32Rect			cRoot;
	SInt32Vec2			sPosition;

	if (pPane.IsNull() || (pPane->GetState() != DPS_AutoHidden))
	{
		return;
	}
	if (mpFlyout.IsNotNull() && (mpFlyout->GetPane() == pPane))
	{
		return;
	}
	CloseFlyout();
	pStrip = mpArea->GetStrip(pPane->GetAutoHideSide());
	pTab = pStrip->FindTab(pPane);
	if (pTab.IsNull())
	{
		return;
	}
	mpFlyout = OMalloc<CAutoHideFlyout>(mpWindow, Ptr<CDockManager>(this), pPane);
	mpArea->GetRootBounds(&cRoot);
	mpFlyout->Place(&cRoot, &sPosition);
	mpWindow->OpenPopup(mpFlyout, sPosition, pTab);
	FocusPane(pPane);
	SetActivePane(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// Slides the pane back.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::CloseFlyout(void)
{
	Ptr<CAutoHideFlyout>	pFlyout;

	if (mpFlyout.IsNull())
	{
		return;
	}
	pFlyout = mpFlyout;
	mpFlyout = NULL;
	mpWindow->ClosePopup(pFlyout);
	pFlyout->Detach();
}


//////////////////////////////////////////////////////////////////////////
//
// The flyout's popup closed, by a press outside it.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::FlyoutClosed(void)
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
// Something about the layout changed, to save it later.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::LayoutChanged(void)
{
	mpWindow->InvalidateLayout();
	if (mpcListener)
	{
		mpcListener->LayoutChanged();
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

	ClearLayout();
	for (i = 0; i < maPanes.Size(); i++)
	{
		pPane = maPanes.Get(i);
		pPane->ClearRestore();
		if (pPane->IsTool() && (pPane->GetDefaultKind() != DPL_Hidden))
		{
			DockDefault(pPane);
		}
	}
	for (i = 0; i < maPanes.Size(); i++)
	{
		pPane = maPanes.Get(i);
		if (pPane->IsDocument())
		{
			AddToWell(pPane);
		}
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane is taken out, the floating windows closed and the main area
// left with the empty document well.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ClearLayout(void)
{
	size						i;
	Ptr<CDockPane>				pPane;
	Ptr<CDockFloatingWindow>	pFloating;

	CloseFlyout();
	for (i = 0; i < maPanes.Size(); i++)
	{
		pPane = maPanes.Get(i);
		if (pPane->IsShown())
		{
			TakeOut(pPane, false);
		}
	}
	while (maFloating.Size() > 0)
	{
		RemoveFloating(maFloating.Get(maFloating.Size() - 1));
	}
	mpWell->SetRoot(NULL);
	mpArea->SetRoot(mpWell);
	mpWell->SetDockSize(0);
	mpDocumentGroup = NULL;
	SetActivePane(NULL);
	mpActiveDocument = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockArea>				CDockManager::GetArea(void) { return mpArea; }
Ptr<CDocumentWell>			CDockManager::GetWell(void) { return mpWell; }
Ptr<CWindow>				CDockManager::GetWindow(void) { return mpWindow; }
void						CDockManager::SetListener(CDockManagerListener* pcListener) { mpcListener = pcListener; }
Ptr<CWindowTheme>			CDockManager::GetTheme(void) { return mpTheme; }
Ptr<CFont>					CDockManager::GetFont(void) { return mpFont; }
Ptr<CFont>					CDockManager::GetTurnedFont(void) { return mpTurnedFont; }
ARGB32						CDockManager::GetColour(EDockColour eColour) { return mpTheme->GetDockColour(eColour); }
void						CDockManager::SetColour(EDockColour eColour, ARGB32 sColour) { mpTheme->SetDockColour(eColour, sColour); }
Ptr<CImageCel>				CDockManager::GetImage(EDockImage eImage) { return maImages.Get(eImage); }
size						CDockManager::NumPanes(void) { return maPanes.Size(); }
Ptr<CDockPane>				CDockManager::GetPane(size uiIndex) { return maPanes.Get(uiIndex); }
Ptr<CDockPane>				CDockManager::GetActivePane(void) { return mpActivePane; }
Ptr<CDockPane>				CDockManager::GetActiveDocument(void) { return mpActiveDocument; }
size						CDockManager::NumFloating(void) { return maFloating.Size(); }
Ptr<CDockFloatingWindow>	CDockManager::GetFloating(size uiIndex) { return maFloating.Get(uiIndex); }
Ptr<CAutoHideFlyout>		CDockManager::GetFlyout(void) { return mpFlyout; }
SDockTarget*				CDockManager::GetTarget(void) { return mpDrag->GetTarget(); }
Ptr<CDockDrag>				CDockManager::GetDrag(void) { return mpDrag; }
bool						CDockManager::IsDragging(void) { return mpDrag->IsDragging(); }
void						CDockManager::SetDragNoDock(bool bNoDock) { mpDrag->SetNoDock(bNoDock); }
void						CDockManager::BeginPaneDrag(Ptr<CDockPane> pPane) { mpDrag->BeginPane(pPane); }
void						CDockManager::BeginGroupDrag(Ptr<CDockGroup> pGroup) { mpDrag->BeginGroup(pGroup); }
void						CDockManager::BeginFloatingDrag(Ptr<CDockFloatingWindow> pFloating) { mpDrag->BeginFloating(pFloating); }
void						CDockManager::SetNextGroupID(int32 iID) { miNextGroupID = iID; }
int32						CDockManager::GetNextGroupID(void) { return miNextGroupID; }


//////////////////////////////////////////////////////////////////////////
//
// The node is in the document well's tree.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::IsInWell(Ptr<CDockNode> pNode)
{
	return pNode.IsNotNull() && (pNode != mpWell) && pNode->IsWithin(mpWell);
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
// See CDockTree::DockAt.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::DockNodeAt(Ptr<CDockNode> pNode, SDockTarget* psTarget)
{
	return mcTree.DockAt(pNode, psTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// The main window's client area on the screen in the last layout read.
// Returns false if there wasn't one.  ReadLayout applies it if the main
// window isn't shown yet.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::GetLayoutWindowRectangle(CInt32Rect* pcDest)
{
	pcDest->Init(&mcLayoutWindow);
	return mbLayoutWindow;
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
	pPane->SetState((pGroup->GetWindow() == mpWindow) ? DPS_Docked : DPS_Floating);
}

