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
#include "DockRestorer.h"


//////////////////////////////////////////////////////////////////////////
//
// pcManager holds the panes, groups and windows.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::Init(CDockManager* pcManager)
{
	mpcManager = pcManager;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A docked pane leaving its group remembers it: the group and its place
// in it, and a pane left in the group or, if it was the last, the group's
// place beside its neighbour.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::Record(Ptr<CDockPane> pPane)
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
		sRestore.bInWell = mpcManager->IsInWell(pGroup);
		pPane->SetRestore(&sRestore);
	}
	else if (GetForGroup(pGroup, &sRestore))
	{
		sRestore.iIndex = (int32)uiIndex;
		pPane->SetRestore(&sRestore);
	}
	sRestore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where the group would go back to as a whole: its ID and the pane in the
// node beside it in its split, with the side of that node it was on.  A
// group beside the document well remembers the well's side.  Returns
// false for a group that isn't docked in the main area.
//
//////////////////////////////////////////////////////////////////////////
bool CDockRestorer::GetForGroup(Ptr<CDockGroup> pGroup, SDockRestore* psDest)
{
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockNode>		pSibling;
	size				uiIndex;
	bool				bAfter;
	SInt32Vec2			sSize;

	psDest->iGroup = pGroup->GetID();
	psDest->iIndex = 0;
	psDest->SetNeighbour(NULL);
	psDest->eSide = DS_None;
	psDest->iSize = 0;
	psDest->bInWell = mpcManager->IsInWell(pGroup);

	pSplit = pGroup->GetParentSplit();
	if (pSplit.IsNull())
	{
		return pGroup->IsWithin(mpcManager->mpArea);
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
	SetNeighbour(psDest, pSibling);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The neighbour remembered for the node beside: the well, if the node
// holds it, as it is never hidden; otherwise a pane in it.  Restore
// climbs from there back up to the node beside.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::SetNeighbour(SDockRestore* psDest, Ptr<CDockNode> pSibling)
{
	Ptr<CDockGroup>		pSiblingGroup;

	if (pSibling->ContainsDocumentWell())
	{
		psDest->SetNeighbour(DOCK_WELL_NEIGHBOUR);
		return;
	}
	pSiblingGroup = pSibling->GetFirstGroup();
	if (pSiblingGroup.IsNotNull() && (pSiblingGroup->NumPanes() > 0))
	{
		psDest->SetNeighbour(pSiblingGroup->GetPane(0)->GetID());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the group remembers the group's place, as a whole, with
// its own tab's place in it: so the first docked back brings the group
// back and the others join it.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::GiveToGroup(Ptr<CDockGroup> pGroup)
{
	SDockRestore	sRestore;
	size			i;

	sRestore.Init();
	if (GetForGroup(pGroup, &sRestore))
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
// Puts a pane that isn't shown back where it was last docked: its group
// if it is still docked, else beside or with its neighbour, else in the
// well if it was there, else its default place.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::Restore(Ptr<CDockPane> pPane)
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
void CDockRestorer::RestoreFrom(Ptr<CDockPane> pPane, Ptr<CDockPane> pFrom, int iDepth)
{
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pNeighbour;
	Ptr<CDockGroup>	pNeighbourGroup;
	EDockSide		eSide;
	SDockRestore*	psFrom;

	psFrom = pFrom->GetRestore();
	pGroup = mpcManager->FindGroup(psFrom->iGroup);
	if (pGroup.IsNotNull() && pGroup->IsWithin(mpcManager->mpArea) && (pGroup->IsDocumentGroup() || !pPane->IsDocument()))
	{
		mpcManager->PlaceInGroup(pGroup, pPane, (size)pPane->GetRestore()->iIndex);
		return;
	}

	eSide = psFrom->eSide;
	if (psFrom->IsBesideWell())
	{
		DockBeside(pPane, mpcManager->mpWell, psFrom);
		return;
	}
	pNeighbour = mpcManager->GetPane(psFrom->szNeighbour.Text());
	if (pNeighbour.IsNotNull() && (pNeighbour != pPane) && (pNeighbour->GetState() == DPS_Docked))
	{
		pNeighbourGroup = pNeighbour->GetGroup();
		if ((eSide == DS_Centre) || (eSide == DS_None))
		{
			if (pNeighbourGroup->IsDocumentGroup() || !pPane->IsDocument())
			{
				mpcManager->PlaceInGroup(pNeighbourGroup, pPane, (size)pPane->GetRestore()->iIndex);
				return;
			}
		}
		else
		{
			DockBeside(pPane, pNeighbourGroup, psFrom);
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
		mpcManager->AddToWell(pPane);
		return;
	}
	DockDefault(pPane);
}


//////////////////////////////////////////////////////////////////////////
//
// pPane in a new group, with the ID of the group psFrom remembers, docked
// on psFrom's side of the node in the split of that direction that pNear
// is in.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::DockBeside(Ptr<CDockPane> pPane, Ptr<CDockNode> pNear, SDockRestore* psFrom)
{
	Ptr<CDockNode>	pBeside;
	Ptr<CDockGroup>	pNew;

	pBeside = mpcManager->mcTree.ClimbToSplit(pNear, psFrom->eSide);
	pNew = mpcManager->CreateGroup(mpcManager->IsInWell(pBeside), psFrom->iGroup);
	mpcManager->PlaceInGroup(pNew, pPane);
	mpcManager->mcTree.DockBeside(pBeside, pNew, psFrom->eSide, psFrom->iSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Shows a pane in its default place.
//
//////////////////////////////////////////////////////////////////////////
void CDockRestorer::DockDefault(Ptr<CDockPane> pPane)
{
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pWith;
	int32			iSize;

	switch (pPane->GetDefaultKind())
	{
	case DPL_WithPane:
		pWith = mpcManager->GetPane(pPane->GetDefaultPane());
		if (pWith.IsNotNull() && (pWith != pPane) && (pWith->GetState() == DPS_Docked) && (pWith->GetGroup()->IsDocumentGroup() || !pPane->IsDocument()))
		{
			mpcManager->PlaceInGroup(pWith->GetGroup(), pPane);
			return;
		}
		if (pWith.IsNotNull() && (pWith != pPane) && (pWith->GetState() == DPS_Hidden || pWith->GetState() == DPS_Removed) && (pWith->GetDefaultKind() == DPL_BesideWell))
		{
			pGroup = mpcManager->CreateGroup(false);
			mpcManager->PlaceInGroup(pGroup, pPane);
			iSize = pWith->GetDefaultSize();
			mpcManager->mcTree.DockBeside(mpcManager->mpWell, pGroup, pWith->GetDefaultSide(), (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
			return;
		}
		break;
	case DPL_InWell:
		mpcManager->AddToWell(pPane);
		return;
	case DPL_Hidden:
		pPane->SetState(DPS_Hidden);
		return;
	default:
		break;
	}
	if (pPane->IsDocument())
	{
		mpcManager->AddToWell(pPane);
		return;
	}
	pGroup = mpcManager->CreateGroup(false);
	mpcManager->PlaceInGroup(pGroup, pPane);
	iSize = pPane->GetDefaultSize();
	mpcManager->mcTree.DockBeside(mpcManager->mpWell, pGroup, (pPane->GetDefaultKind() == DPL_BesideWell) ? pPane->GetDefaultSide() : DS_Left, (iSize > 0) ? iSize : DOCK_DEFAULT_SIZE);
}

