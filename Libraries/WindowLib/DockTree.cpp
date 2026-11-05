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
#include "DockManager.h"
#include "DockDrag.h"
#include "DockTree.h"


//////////////////////////////////////////////////////////////////////////
//
// pcManager holds the tree: its area, its well and its groups.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::Init(CDockManager* pcManager)
{
	mpcManager = pcManager;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Docks pNode, which isn't in a tree, at the target: along the area's
// edge, beside or into a group, beside the document well or into the
// empty well.  Anything going into the well becomes a document group.
// Its panes are docked, or floating in a floating window's area, and its
// selected pane stays selected.  Returns false for a target pNode can't
// go to: documents only go with documents.
//
//////////////////////////////////////////////////////////////////////////
bool CDockTree::DockAt(Ptr<CDockNode> pNode, SDockTarget* psTarget)
{
	Ptr<CDockArea>		pArea;
	Ptr<CDockGroup>		pFirst;
	Ptr<CDockPane>		pSelected;
	CArray<CDockPane>	aPanes;
	int32				iSize;
	bool				bResult;

	if (pNode.IsNull() || !psTarget->IsValid())
	{
		return false;
	}
	pArea = psTarget->pcArea;
	if (pNode->GetWindow() != pArea->GetWindow())
	{
		pNode->MoveToWindow(pArea->GetWindow());
	}
	//The panes are kept: pNode may be let go as they are moved into a group.
	aPanes.Init();
	pNode->GetPanes(&aPanes);
	pFirst = pNode->GetFirstGroup();
	if (pFirst.IsNotNull())
	{
		pSelected = pFirst->GetSelectedPane();
	}
	iSize = IsHorizontalDockSide(psTarget->eSide) ? psTarget->cPreview.GetWidth() : psTarget->cPreview.GetHeight();

	switch (psTarget->eKind)
	{
	case DTK_AreaEdge:
		bResult = DockAtAreaEdge(pNode, pArea, psTarget->eSide, iSize);
		break;
	case DTK_Group:
		bResult = DockAtGroup(pNode, psTarget->pcNode->AsGroup(), psTarget->eSide, iSize);
		break;
	case DTK_WellOuter:
		bResult = DockBesideWell(pNode, psTarget->eSide, iSize);
		break;
	case DTK_WellCentre:
		bResult = DockInWell(pNode);
		break;
	default:
		bResult = false;
		break;
	}

	if (bResult)
	{
		SetStates(&aPanes, pArea->IsMain() ? DPS_Docked : DPS_Floating);
		if (pSelected.IsNotNull() && pSelected->GetGroup().IsNotNull())
		{
			pSelected->GetGroup()->SelectPane(pSelected);
		}
		mpcManager->LayoutChanged();
	}
	aPanes.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Along the area's edge, outside everything else in it.
//
//////////////////////////////////////////////////////////////////////////
bool CDockTree::DockAtAreaEdge(Ptr<CDockNode> pNode, Ptr<CDockArea> pArea, EDockSide eSide, int32 iSize)
{
	Ptr<CDockNode>	pRoot;

	pRoot = pArea->GetRoot();
	if (pRoot.IsNull())
	{
		pArea->SetRoot(pNode);
	}
	else
	{
		DockBeside(pRoot, pNode, eSide, iSize);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Into the group (the centre) or beside it.  Beside a group in the well
// it goes as a document group of its own; documents only go beside
// document groups.
//
//////////////////////////////////////////////////////////////////////////
bool CDockTree::DockAtGroup(Ptr<CDockNode> pNode, Ptr<CDockGroup> pGroup, EDockSide eSide, int32 iSize)
{
	bool	bInWell;

	bInWell = mpcManager->IsInWell(pGroup);
	if (eSide == DS_Centre)
	{
		MovePanesInto(pNode, pGroup);
		if (bInWell)
		{
			mpcManager->mpDocumentGroup = pGroup;
		}
		return true;
	}
	if (bInWell)
	{
		DockBeside(pGroup, GatherInto(pNode, true), eSide, iSize);
		return true;
	}
	if (!pNode->HasDocuments() || pGroup->IsDocumentGroup())
	{
		DockBeside(pGroup, pNode, eSide, iSize);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Beside the whole document well: tools only.
//
//////////////////////////////////////////////////////////////////////////
bool CDockTree::DockBesideWell(Ptr<CDockNode> pNode, EDockSide eSide, int32 iSize)
{
	if (pNode->HasDocuments())
	{
		return false;
	}
	DockBeside(mpcManager->mpWell, pNode, eSide, iSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Into the document well: its root if it is empty, else its first group.
//
//////////////////////////////////////////////////////////////////////////
bool CDockTree::DockInWell(Ptr<CDockNode> pNode)
{
	Ptr<CDockGroup>	pGroup;

	if (mpcManager->mpWell->GetRoot().IsNull())
	{
		pGroup = GatherInto(pNode, true);
		mpcManager->mpWell->SetRoot(pGroup);
	}
	else
	{
		pGroup = mpcManager->mpWell->GetFirstGroup();
		MovePanesInto(pNode, pGroup);
	}
	mpcManager->mpDocumentGroup = pGroup;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in pNode, which isn't in a tree and is let go, becomes a tab
// of pGroup.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::MovePanesInto(Ptr<CDockNode> pNode, Ptr<CDockGroup> pGroup)
{
	CArray<CDockPane>	aPanes;
	size				i;
	Ptr<CDockPane>		pPane;

	aPanes.Init();
	pNode->GetPanes(&aPanes);
	for (i = 0; i < aPanes.Size(); i++)
	{
		pPane = aPanes.Get(i);
		pPane->GetGroup()->TakePane(pPane);
		pGroup->AddPane(pPane);
	}
	aPanes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::SetStates(CArray<CDockPane>* paPanes, EDockPaneState eState)
{
	size	i;

	for (i = 0; i < paPanes->Size(); i++)
	{
		paPanes->Get(i)->SetState(eState);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pNode goes beside pTarget on eSide, iSize pixels across (0 for half).
// In a split of the same direction it goes next to pTarget; otherwise
// pTarget is split.  pNode must be of the target's window and not in a
// tree.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::DockBeside(Ptr<CDockNode> pTarget, Ptr<CDockNode> pNode, EDockSide eSide, int32 iSize)
{
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
	bAfter = IsAfterDockSide(eSide);
	if (pNode->GetWindow() != pTarget->GetWindow())
	{
		pNode->MoveToWindow(pTarget->GetWindow());
	}

	sSize = pTarget->GetActualSize();
	iTarget = IsHorizontalDockSide(eSide) ? sSize.x : sSize.y;
	if (iSize <= 0)
	{
		iSize = (iTarget > 0) ? iTarget / 2 : DOCK_DEFAULT_SIZE;
	}
	pNode->SetDockSize(iSize);

	pParent = pTarget->GetParentSplit();
	pSplit = pTarget->AsSplit();
	if (pSplit.IsNotNull() && pSplit->IsAlong(eSide))
	{
		//Beside a split of the same direction is its first or last node.
		pSplit->InsertNode(pNode, bAfter ? pSplit->NumNodes() : 0);
		Flatten(pSplit);
	}
	else if (pParent.IsNotNull() && pParent->IsAlong(eSide))
	{
		uiIndex = pParent->IndexOf(pTarget);
		pParent->InsertNode(pNode, bAfter ? uiIndex + 1 : uiIndex);
		Flatten(pParent);
	}
	else
	{
		pSplit = OMalloc<CDockSplit>(pTarget->GetWindow(), Ptr<CDockManager>(mpcManager), IsHorizontalDockSide(eSide), mpcManager->GetColour(DC_Sash));
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
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pNew takes pOld's place in its split, well or area.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::ReplaceInParent(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew)
{
	CDockNodeHolder*	pcHolder;

	pcHolder = pOld->GetHolder();
	if (pcHolder)
	{
		pcHolder->ReplaceNode(pOld, pNew);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the node out of its split, well or area, and tidies what is left.
// A floating window left empty closes.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::RemoveNode(Ptr<CDockNode> pNode)
{
	CDockNodeHolder*			pcHolder;
	Ptr<CDockSplit>				pSplit;
	Ptr<CDockArea>				pArea;

	pcHolder = pNode->GetHolder();
	if (pcHolder == NULL)
	{
		return;
	}
	pSplit = pNode->GetParentSplit();
	pArea = pNode->GetParentArea();
	pNode->GetWindow()->GetFocus()->Forget(pNode);
	pcHolder->RemoveNode(pNode);
	if (pSplit.IsNotNull())
	{
		Tidy(pSplit);
	}
	else if (pArea.IsNotNull() && pArea->GetFloating().IsNotNull() && (pArea->GetFloating() != mpcManager->mpDrag->GetFloating()))
	{
		mpcManager->RemoveFloating(pArea->GetFloating());
	}
	mpcManager->LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A split with no nodes is removed; with one it is replaced by it; a
// split in a split of the same direction gives its nodes to its parent.
//
//////////////////////////////////////////////////////////////////////////
void CDockTree::Tidy(Ptr<CDockNode> pNode)
{
	Ptr<CDockSplit>	pSplit;
	Ptr<CDockSplit>	pParent;
	Ptr<CDockNode>	pOnly;
	int32			iSize;

	if (pNode.IsNull() || !pNode->IsSplit())
	{
		return;
	}
	pSplit = pNode->AsSplit();
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
void CDockTree::Flatten(Ptr<CDockSplit> pSplit)
{
	Ptr<CDockSplit>	pParent;
	Ptr<CDockNode>	pNode;
	Ptr<CDockSplit>	pChild;
	size			i;

	if (pSplit.IsNull())
	{
		return;
	}
	for (i = 0; i < pSplit->NumNodes();)
	{
		pNode = pSplit->GetNode(i);
		pChild = pNode->AsSplit();
		if (pSplit->IsSameDirection(pChild))
		{
			//Its nodes take its place; they are looked at in turn, in case one is a split too.
			MergeSplit(pSplit, pChild);
		}
		else
		{
			i++;
		}
	}
	pParent = pSplit->GetParentSplit();
	if (pSplit->IsSameDirection(pParent))
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
void CDockTree::MergeSplit(Ptr<CDockSplit> pParent, Ptr<CDockSplit> pChild)
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
// The first node up from pStart that is in a split of the direction: what
// a group was docked beside when it was in such a split with pStart's
// node.  pStart if there is none: the split went when the group left.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockTree::ClimbToSplit(Ptr<CDockNode> pStart, EDockSide eSide)
{
	Ptr<CDockNode>		pNode;
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockNode>		pParent;

	pNode = pStart;
	for (;;)
	{
		pSplit = pNode->GetParentSplit();
		if (pSplit.IsNotNull() && pSplit->IsAlong(eSide))
		{
			return pNode;
		}
		pParent = pNode->GetParentNode();
		if (pParent.IsNull())
		{
			//No split of the direction: beside pStart itself, which was what the group was beside.
			return pStart;
		}
		pNode = pParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The side of the document well the node is on, from the tree: in the
// first split up from it that also holds the well, before or after the
// well's part.  Left if it is in no such split.
//
//////////////////////////////////////////////////////////////////////////
EDockSide CDockTree::SideOfWell(Ptr<CDockNode> pNode)
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
// A group of the kind holding every pane in pNode, which is let go.  A
// group of the kind already is returned as it is.  pNode isn't in a tree.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockTree::GatherInto(Ptr<CDockNode> pNode, bool bDocuments)
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
	pGroup = mpcManager->CreateGroup(bDocuments);
	if (pNode->GetWindow() != mpcManager->mpWindow)
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


