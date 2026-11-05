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
	masColours[DC_TitleBar] = JEWEL_PANEL;
	masColours[DC_ActiveTitleBar] = JEWEL_SELECTION;
	masColours[DC_Sash] = JEWEL_EDITOR;
	masColours[DC_Background] = JEWEL_EDITOR;
	masColours[DC_Strip] = JEWEL_BAR_BACKGROUND;
	masColours[DC_StripTab] = JEWEL_PANEL;
	masColours[DC_StripTabHover] = JEWEL_HOVER;
	masColours[DC_StripTabLine] = JEWEL_BORDER;
	masColours[DC_Guide] = JEWEL_PANEL;
	masColours[DC_GuideBorder] = JEWEL_BORDER_HOVER;
	masColours[DC_GuideHover] = JEWEL_SELECTION;
	masColours[DC_GuideArrow] = JEWEL_BLUE;
	masColours[DC_Preview] = Set32BitColour((uint8)0x35, (uint8)0x74, (uint8)0xF0, (uint8)0x50);
	masColours[DC_PreviewBorder] = JEWEL_BLUE;
	masColours[DC_FrameBorder] = JEWEL_SEPARATOR;
	maImages.Init();
	CreateImages();

	maPanes.Init();
	maFloating.Init();
	miNextGroupID = 1;
	mpActivePane = NULL;
	mpActiveDocument = NULL;
	mpDocumentGroup = NULL;
	mpcListener = NULL;
	mpDragFloating = NULL;
	mpDragSource = NULL;
	mpTracker = NULL;
	msDragGrab.Init(0, 0);
	mcDragStart.Init(0, 0, 0, 0);
	mbDragTornOff = false;
	mbDragCancelled = false;
	mbDragNoDock = false;
	msTarget.Init();
	mpGuidesArea = NULL;
	mpFlyout = NULL;
	mcLayoutWindow.Init(0, 0, 0, 0);
	mbLayoutWindow = false;

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
	U_Data(ARGB32[DC_NumColours], masColours);
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
	M_Pointer(mpDragFloating);
	M_Pointer(mpDragSource);
	M_Pointer(mpTracker);
	U_2Int32(msDragGrab);
	U_Data(CInt32Rect, mcDragStart);
	U_Bool(mbDragTornOff);
	U_Bool(mbDragCancelled);
	U_Bool(mbDragNoDock);
	U_Data(SDockTarget, msTarget);
	M_Pointer(mpGuidesArea);
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
			RemoveNode(pGroup);
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
bool CDockManager::GetGroupRestore(Ptr<CDockGroup> pGroup, int32* piGroup, CChars* pszNeighbour, EDockSide* peSide, int32* piSize, bool* pbInWell)
{
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockNode>		pChild;
	Ptr<CDockNode>		pSibling;
	Ptr<CDockGroup>		pSiblingGroup;
	size				uiIndex;
	bool				bAfter;
	SInt32Vec2			sSize;
	Ptr<CDockPane>		pNeighbour;

	*piGroup = pGroup->GetID();
	pszNeighbour->Kill();
	pszNeighbour->Init();
	*peSide = DS_None;
	*piSize = 0;
	*pbInWell = pGroup->IsWithin(mpWell);

	//Up to the first split, keeping the child of it the group is in.
	pChild = pGroup;
	pSplit = pChild->GetParentSplit();
	while (pSplit.IsNull())
	{
		return pGroup->IsWithin(mpArea);
	}
	uiIndex = pSplit->IndexOf(pChild);
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

	sSize = pChild->GetActualSize();
	*piSize = pSplit->IsHorizontal() ? sSize.x : sSize.y;
	if (pSplit->IsHorizontal())
	{
		*peSide = bAfter ? DS_Right : DS_Left;
	}
	else
	{
		*peSide = bAfter ? DS_Bottom : DS_Top;
	}

	//The well, if the node beside holds it, as it is never hidden; otherwise a pane in it.  Restore climbs from
	//there back up to the node beside.
	if (pSibling->ContainsDocumentWell())
	{
		pszNeighbour->Append(DOCK_WELL_NEIGHBOUR);
		return true;
	}
	pSiblingGroup = pSibling->GetFirstGroup();
	if (pSiblingGroup.IsNotNull() && (pSiblingGroup->NumPanes() > 0))
	{
		pNeighbour = pSiblingGroup->GetPane(0);
		pszNeighbour->Append(pNeighbour->GetID());
	}
	return true;
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
	Ptr<CDockPane>	pNeighbour;
	int32			iGroup;
	CChars			szNeighbour;
	EDockSide		eSide;
	int32			iSize;
	bool			bInWell;

	pGroup = pPane->GetGroup();
	if (pGroup.IsNull())
	{
		return;
	}
	uiIndex = pGroup->IndexOfPane(pPane);
	if (pGroup->NumPanes() > 1)
	{
		pNeighbour = pGroup->GetPane((uiIndex > 0) ? uiIndex - 1 : 1);
		pPane->SetRestore(pGroup->GetID(), (int32)uiIndex, pNeighbour->GetID(), DS_Centre, 0, pGroup->IsWithin(mpWell));
		return;
	}
	szNeighbour.Init();
	if (GetGroupRestore(pGroup, &iGroup, &szNeighbour, &eSide, &iSize, &bInWell))
	{
		pPane->SetRestore(iGroup, (int32)uiIndex, szNeighbour.Empty() ? NULL : szNeighbour.Text(), eSide, iSize, bInWell);
	}
	szNeighbour.Kill();
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
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pNeighbour;
	Ptr<CDockGroup>	pNeighbourGroup;
	Ptr<CDockGroup>	pNew;
	Ptr<CDockNode>	pBeside;
	EDockSide		eSide;
	int32			iSize;
	bool			bInWell;

	pGroup = FindGroup(pPane->GetRestoreGroup());
	if (pGroup.IsNotNull() && pGroup->IsWithin(mpArea) && (pGroup->IsDocumentGroup() || !pPane->IsDocument()))
	{
		pGroup->AddPane(pPane, (size)pPane->GetRestoreIndex());
		pPane->SetState(DPS_Docked);
		return;
	}

	eSide = pPane->GetRestoreSide();
	iSize = pPane->GetRestoreSize();
	if ((StringCompare(pPane->GetRestoreNeighbour(), DOCK_WELL_NEIGHBOUR) == 0) && (eSide != DS_None) && (eSide != DS_Centre))
	{
		pBeside = ClimbToSplit(mpWell, IsHorizontalDockSide(eSide));
		pNew = CreateGroup(pBeside->IsWithin(mpWell) && (pBeside != mpWell), pPane->GetRestoreGroup());
		pNew->AddPane(pPane);
		DockBeside(pBeside, pNew, eSide, iSize);
		pPane->SetState(DPS_Docked);
		return;
	}
	pNeighbour = GetPane(pPane->GetRestoreNeighbour());
	if (pNeighbour.IsNotNull() && (pNeighbour != pPane) && (pNeighbour->GetState() == DPS_Docked))
	{
		pNeighbourGroup = pNeighbour->GetGroup();
		if ((eSide == DS_Centre) || (eSide == DS_None))
		{
			if (pNeighbourGroup->IsDocumentGroup() || !pPane->IsDocument())
			{
				pNeighbourGroup->AddPane(pPane, (size)pPane->GetRestoreIndex());
				pPane->SetState(DPS_Docked);
				return;
			}
		}
		else
		{
			pBeside = ClimbToSplit(pNeighbourGroup, IsHorizontalDockSide(eSide));
			bInWell = pBeside->IsWithin(mpWell) && (pBeside != mpWell);
			pNew = CreateGroup(bInWell, pPane->GetRestoreGroup());
			pNew->AddPane(pPane);
			DockBeside(pBeside, pNew, eSide, iSize);
			pPane->SetState(DPS_Docked);
			return;
		}
	}
	if (pPane->IsRestoreInWell() || pPane->IsDocument())
	{
		AddToWell(pPane);
		return;
	}
	DockDefault(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// pStart, or the first node up from it that is in a split of the
// direction: what a group was docked beside when it was in such a split
// with pStart's node.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockManager::ClimbToSplit(Ptr<CDockNode> pStart, bool bHorizontal)
{
	Ptr<CDockNode>		pNode;
	Ptr<CDockSplit>		pSplit;
	Ptr<CComponent>		pParent;
	CDockNode*			pcParentNode;

	pNode = pStart;
	for (;;)
	{
		pSplit = pNode->GetParentSplit();
		if (pSplit.IsNotNull() && (pSplit->IsHorizontal() == bHorizontal))
		{
			return pNode;
		}
		pParent = pNode->GetParent();
		pcParentNode = dynamic_cast<CDockNode*>(&pParent);
		if (pcParentNode == NULL)
		{
			return pNode;
		}
		pNode = pcParentNode;
	}
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
			pWith->GetGroup()->AddPane(pPane);
			pPane->SetState(DPS_Docked);
			return;
		}
		if (pWith.IsNotNull() && (pWith != pPane) && (pWith->GetState() == DPS_Hidden || pWith->GetState() == DPS_Removed) && (pWith->GetDefaultKind() == DPL_BesideWell))
		{
			pGroup = CreateGroup(false);
			pGroup->AddPane(pPane);
			iSize = pWith->GetDefaultSize();
			DockBeside(mpWell, pGroup, pWith->GetDefaultSide(), (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
			pPane->SetState(DPS_Docked);
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
	pGroup->AddPane(pPane);
	iSize = pPane->GetDefaultSize();
	DockBeside(mpWell, pGroup, (pPane->GetDefaultKind() == DPL_BesideWell) ? pPane->GetDefaultSide() : DS_Left, (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
	pPane->SetState(DPS_Docked);
}


//////////////////////////////////////////////////////////////////////////
//
// The pane goes into the document well: into pGroup, or the group the
// last document went into, or the well's first group, or a new group.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::AddToWell(Ptr<CDockPane> pPane, Ptr<CDockGroup> pGroup)
{
	if (pGroup.IsNull() || !pGroup->IsWithin(mpWell))
	{
		pGroup = mpDocumentGroup;
	}
	if (pGroup.IsNull() || !pGroup->IsWithin(mpWell))
	{
		pGroup = mpWell->GetFirstGroup();
	}
	if (pGroup.IsNull())
	{
		pGroup = CreateGroup(true);
		mpWell->SetRoot(pGroup);
	}
	pGroup->AddPane(pPane);
	mpDocumentGroup = pGroup;
	pPane->SetState(DPS_Docked);
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pNode goes beside pTarget on eSide, iSize pixels across (0 for half).
// In a split of the same direction it goes next to pTarget; otherwise
// pTarget is split.  pNode must be of the target's window and not in a
// tree.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::DockBeside(Ptr<CDockNode> pTarget, Ptr<CDockNode> pNode, EDockSide eSide, int32 iSize)
{
	bool			bHorizontal;
	bool			bAfter;
	Ptr<CDockSplit>	pParent;
	Ptr<CDockSplit>	pSplit;
	size			uiIndex;
	SInt32Vec2		sSize;
	int32			iTarget;

	if ((eSide == DS_Centre) || (eSide == DS_None))
	{
		eSide = DS_Left;
	}
	bHorizontal = IsHorizontalDockSide(eSide);
	bAfter = (eSide == DS_Right) || (eSide == DS_Bottom);
	if (pNode->GetWindow() != pTarget->GetWindow())
	{
		pNode->MoveToWindow(pTarget->GetWindow());
	}

	sSize = pTarget->GetActualSize();
	iTarget = bHorizontal ? sSize.x : sSize.y;
	if (iSize <= 0)
	{
		iSize = (iTarget > 0) ? iTarget / 2 : DOCK_DEFAULT_SIZE;
	}
	pNode->SetDockSize(iSize);

	pParent = pTarget->GetParentSplit();
	if (pTarget->IsSplit() && (Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pTarget))->IsHorizontal() == bHorizontal))
	{
		//Beside a split of the same direction is its first or last node.
		pSplit = Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pTarget));
		pSplit->InsertNode(pNode, bAfter ? pSplit->NumNodes() : 0);
		Flatten(pSplit);
	}
	else if (pParent.IsNotNull() && (pParent->IsHorizontal() == bHorizontal))
	{
		uiIndex = pParent->IndexOf(pTarget);
		pParent->InsertNode(pNode, bAfter ? uiIndex + 1 : uiIndex);
		Flatten(pParent);
	}
	else
	{
		pSplit = OMalloc<CDockSplit>(pTarget->GetWindow(), Ptr<CDockManager>(this), bHorizontal, GetColour(DC_Sash));
		ReplaceInParent(pTarget, pSplit);
		pTarget->SetDockSize((iTarget > iSize) ? iTarget - iSize - DOCK_SASH_SIZE : 0);
		if (bAfter)
		{
			pSplit->AddNode(pTarget);
			pSplit->AddNode(pNode);
		}
		else
		{
			pSplit->AddNode(pNode);
			pSplit->AddNode(pTarget);
		}
		Flatten(pSplit);
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pNew takes pOld's place in its split, well or area.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::ReplaceInParent(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew)
{
	Ptr<CComponent>		pParent;
	CDockSplit*			pcSplit;
	CDocumentWell*		pcWell;
	CDockArea*			pcArea;

	pParent = pOld->GetParent();
	pcSplit = dynamic_cast<CDockSplit*>(&pParent);
	pcWell = dynamic_cast<CDocumentWell*>(&pParent);
	pcArea = dynamic_cast<CDockArea*>(&pParent);
	if (pcSplit)
	{
		pcSplit->ReplaceNode(pOld, pNew);
	}
	else if (pcWell)
	{
		pNew->SetDockSize(pOld->GetDockSize());
		pcWell->SetRoot(pNew);
	}
	else if (pcArea)
	{
		pNew->SetDockSize(pOld->GetDockSize());
		pcArea->SetRoot(pNew);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the node out of its split, well or area, and tidies what is left.
// A floating window left empty closes.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::RemoveNode(Ptr<CDockNode> pNode)
{
	Ptr<CComponent>				pParent;
	CDockSplit*					pcSplit;
	CDocumentWell*				pcWell;
	CDockArea*					pcArea;
	Ptr<CDockSplit>				pSplit;
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;

	pParent = pNode->GetParent();
	pcSplit = dynamic_cast<CDockSplit*>(&pParent);
	pcWell = dynamic_cast<CDocumentWell*>(&pParent);
	pcArea = dynamic_cast<CDockArea*>(&pParent);
	pNode->GetWindow()->GetFocus()->Forget(pNode);
	if (pcSplit)
	{
		pSplit = pcSplit;
		pSplit->RemoveNode(pNode);
		Tidy(pSplit);
	}
	else if (pcWell)
	{
		pcWell->SetRoot(NULL);
	}
	else if (pcArea)
	{
		pcArea->SetRoot(NULL);
		for (i = 0; i < maFloating.Size(); i++)
		{
			pFloating = maFloating.Get(i);
			if (&pFloating->GetArea() == pcArea)
			{
				if (pFloating != mpDragFloating)
				{
					pFloating->Close();
					maFloating.Remove(pFloating);
				}
				break;
			}
		}
	}
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A split with no nodes is removed; with one it is replaced by it; a
// split in a split of the same direction gives its nodes to its parent.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Tidy(Ptr<CDockNode> pNode)
{
	Ptr<CDockSplit>	pSplit;
	Ptr<CDockSplit>	pParent;
	Ptr<CDockNode>	pOnly;
	int32			iSize;

	if (pNode.IsNull() || !pNode->IsSplit())
	{
		return;
	}
	pSplit = Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pNode));
	if (pSplit->NumNodes() == 0)
	{
		RemoveNode(pSplit);
		return;
	}
	if (pSplit->NumNodes() == 1)
	{
		//The node left takes the split's place, and joins its new parent if it is a split of the same direction.
		pOnly = pSplit->GetNode(0);
		iSize = pSplit->GetDockSize();
		pSplit->RemoveNode(pOnly);
		ReplaceInParent(pSplit, pOnly);
		pOnly->SetDockSize(iSize);
		pParent = pOnly->GetParentSplit();
		if (pParent.IsNotNull())
		{
			Flatten(pParent);
		}
		return;
	}
	pParent = pSplit->GetParentSplit();
	Flatten(pParent.IsNotNull() ? pParent : pSplit);
}


//////////////////////////////////////////////////////////////////////////
//
// Every node of pSplit that is a split of the same direction gives its
// nodes to pSplit, in its place, and pSplit gives its own to its parent
// if that is one: a split never holds a split of its direction.  The
// nodes keep the sizes they were last laid out at, so nothing moves.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::Flatten(Ptr<CDockSplit> pSplit)
{
	Ptr<CDockSplit>	pParent;
	Ptr<CDockNode>	pNode;
	CDockSplit*		pcChild;
	size			i;

	if (pSplit.IsNull())
	{
		return;
	}
	for (i = 0; i < pSplit->NumNodes();)
	{
		pNode = pSplit->GetNode(i);
		pcChild = dynamic_cast<CDockSplit*>(&pNode);
		if (pcChild && (pcChild->IsHorizontal() == pSplit->IsHorizontal()))
		{
			//Its nodes take its place; they are looked at in turn, in case one is a split too.
			MergeSplit(pSplit, pcChild);
		}
		else
		{
			i++;
		}
	}
	pParent = pSplit->GetParentSplit();
	if (pParent.IsNotNull() && (pParent->IsHorizontal() == pSplit->IsHorizontal()))
	{
		MergeSplit(pParent, pSplit);
		Flatten(pParent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pChild, a node of pParent of the same direction, is replaced by its
// nodes.  A node laid out keeps its laid out size as its dock size.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::MergeSplit(Ptr<CDockSplit> pParent, Ptr<CDockSplit> pChild)
{
	Ptr<CDockNode>	pNode;
	size			uiIndex;
	size			i;
	int32			iSize;

	uiIndex = pParent->IndexOf(pChild);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return;
	}
	for (i = 0; i < pChild->NumNodes(); i++)
	{
		iSize = pChild->GetNodeSize(i);
		if (iSize > 0)
		{
			pChild->GetNode(i)->SetDockSize(iSize);
		}
	}
	pParent->RemoveNode(pChild);
	while (pChild->NumNodes() > 0)
	{
		pNode = pChild->GetNode(pChild->NumNodes() - 1);
		pChild->RemoveNode(pNode);
		pParent->InsertNode(pNode, uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the node takes the state, of a node docked in the main
// area or floating.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::SetStates(Ptr<CDockNode> pNode, EDockPaneState eState)
{
	CArray<CDockPane>	aPanes;
	size				i;

	aPanes.Init();
	pNode->GetPanes(&aPanes);
	for (i = 0; i < aPanes.Size(); i++)
	{
		aPanes.Get(i)->SetState(eState);
	}
	aPanes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A group of the kind holding every pane in pNode, which is let go.  A
// group of the kind already is returned as it is.  pNode isn't in a tree.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockManager::GatherInto(Ptr<CDockNode> pNode, bool bDocuments)
{
	Ptr<CDockGroup>		pGroup;
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockPane>		pPane;
	Ptr<CDockPane>		pSelected;

	if (pNode->IsGroup())
	{
		pGroup = pNode->GetFirstGroup();
		if (pGroup->IsDocumentGroup() == bDocuments)
		{
			return pGroup;
		}
		pSelected = pGroup->GetSelectedPane();
	}
	pGroup = CreateGroup(bDocuments);
	if (pNode->GetWindow() != mpWindow)
	{
		pGroup->MoveToWindow(pNode->GetWindow());
	}
	aPanes.Init();
	pNode->GetPanes(&aPanes);
	for (i = 0; i < aPanes.Size(); i++)
	{
		pPane = aPanes.Get(i);
		pPane->GetGroup()->TakePane(pPane);
		pGroup->AddPane(pPane);
	}
	aPanes.Kill();
	if (pSelected.IsNotNull())
	{
		pGroup->SelectPane(pSelected);
	}
	return pGroup;
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
	CArray<CComponent>	aChildren;

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
	pcDest->Init(cScreen.GetLeft() + 100, cScreen.GetTop() + 100, cScreen.GetLeft() + 100 + iWidth, cScreen.GetTop() + 100 + iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// A floating window, at pcScreenRectangle, not yet shown.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockFloatingWindow> CDockManager::CreateFloating(CInt32Rect* pcScreenRectangle)
{
	Ptr<CDockFloatingWindow>	pFloating;

	pFloating = OMalloc<CDockFloatingWindow>(Ptr<CDockManager>(this), pcScreenRectangle);
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
	int32						iGroup;
	CChars						szNeighbour;
	EDockSide					eSide;
	int32						iSize;
	bool						bInWell;
	size						i;
	Ptr<CDockPane>				pPane;

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
		szNeighbour.Init();
		if (GetGroupRestore(pGroup, &iGroup, &szNeighbour, &eSide, &iSize, &bInWell))
		{
			for (i = 0; i < pGroup->NumPanes(); i++)
			{
				pPane = pGroup->GetPane(i);
				pPane->SetRestore(iGroup, (int32)i, szNeighbour.Empty() ? NULL : szNeighbour.Text(), eSide, iSize, bInWell);
			}
		}
		szNeighbour.Kill();
	}
	if (mpDocumentGroup == pGroup)
	{
		mpDocumentGroup = NULL;
	}
	RemoveNode(pGroup);

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
		return SideOfWell(pGroup);
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
// The side of the document well the node is on, from the tree: in the
// first split up from it that also holds the well, before or after the
// well's part.  Left if it is in no such split.
//
//////////////////////////////////////////////////////////////////////////
EDockSide CDockManager::SideOfWell(Ptr<CDockNode> pNode)
{
	Ptr<CDockNode>		pChild;
	Ptr<CDockSplit>		pSplit;
	size				uiChild;
	size				i;

	pChild = pNode;
	pSplit = pChild->GetParentSplit();
	while (pSplit.IsNotNull())
	{
		if (pSplit->ContainsDocumentWell() && !pChild->ContainsDocumentWell())
		{
			uiChild = pSplit->IndexOf(pChild);
			for (i = 0; i < pSplit->NumNodes(); i++)
			{
				if (pSplit->GetNode(i)->ContainsDocumentWell())
				{
					if (pSplit->IsHorizontal())
					{
						return (uiChild < i) ? DS_Left : DS_Right;
					}
					return (uiChild < i) ? DS_Top : DS_Bottom;
				}
			}
		}
		pChild = pSplit;
		pSplit = pChild->GetParentSplit();
	}
	return DS_Left;
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
	if (pGroup->IsWithin(mpWell))
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
	int32					iGroup;
	CChars					szNeighbour;
	EDockSide				eRestoreSide;
	int32					iRestoreSize;
	bool					bInWell;
	bool					bRestore;
	Ptr<CAutoHideStrip>		pStrip;

	if (pGroup.IsNull() || pGroup->IsDocumentGroup() || !pGroup->IsWithin(mpArea) || pGroup->IsWithin(mpWell))
	{
		return;
	}
	eSide = NearestEdge(pGroup);
	sSize = pGroup->GetActualSize();
	iSize = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	szNeighbour.Init();
	bRestore = GetGroupRestore(pGroup, &iGroup, &szNeighbour, &eRestoreSide, &iRestoreSize, &bInWell);

	aPanes.Init();
	pGroup->GetPanes(&aPanes);
	pStrip = mpArea->GetStrip(eSide);
	for (i = 0; i < aPanes.Size(); i++)
	{
		pPane = aPanes.Get(i);
		TakeOut(pPane, false);
		if (bRestore)
		{
			pPane->SetRestore(iGroup, (int32)i, szNeighbour.Empty() ? NULL : szNeighbour.Text(), eRestoreSide, iRestoreSize, bInWell);
		}
		pPane->SetAutoHide(eSide, (iSize > DOCK_MINIMUM_SIZE) ? iSize : DOCK_DEFAULT_SIZE);
		pStrip->AddPane(pPane);
		pPane->SetState(DPS_AutoHidden);
		if (mpActivePane == pPane)
		{
			SetActivePane(NULL);
		}
	}
	aPanes.Kill();
	szNeighbour.Kill();
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
		pFloating->Close();
		maFloating.Remove(pFloating);
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
		if (pGroup.IsNotNull() && pGroup->IsWithin(mpWell))
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
		pFloating = maFloating.Get(maFloating.Size() - 1);
		pFloating->Close();
		maFloating.RemoveAt(maFloating.Size() - 1);
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
ARGB32						CDockManager::GetColour(EDockColour eColour) { return masColours[eColour]; }
void						CDockManager::SetColour(EDockColour eColour, ARGB32 sColour) { masColours[eColour] = sColour; }
Ptr<CImageCel>				CDockManager::GetImage(EDockImage eImage) { return maImages.Get(eImage); }
size						CDockManager::NumPanes(void) { return maPanes.Size(); }
Ptr<CDockPane>				CDockManager::GetPane(size uiIndex) { return maPanes.Get(uiIndex); }
Ptr<CDockPane>				CDockManager::GetActivePane(void) { return mpActivePane; }
Ptr<CDockPane>				CDockManager::GetActiveDocument(void) { return mpActiveDocument; }
size						CDockManager::NumFloating(void) { return maFloating.Size(); }
Ptr<CDockFloatingWindow>	CDockManager::GetFloating(size uiIndex) { return maFloating.Get(uiIndex); }
Ptr<CAutoHideFlyout>		CDockManager::GetFlyout(void) { return mpFlyout; }
SDockTarget*				CDockManager::GetTarget(void) { return &msTarget; }
void						CDockManager::SetNextGroupID(int32 iID) { miNextGroupID = iID; }
int32						CDockManager::GetNextGroupID(void) { return miNextGroupID; }

