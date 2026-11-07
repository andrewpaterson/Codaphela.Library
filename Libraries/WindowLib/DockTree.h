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
#ifndef __DOCK_TREE_H__
#define __DOCK_TREE_H__
#include "StandardLib/Array.h"
#include "DockNode.h"


class CDockManager;
class CDockState;
struct SDockTarget;


//The edits of a dock manager's trees of splits, groups and the document well: docking a node at a target or beside
//another, taking one out, and keeping the trees tidy (a split never holds one node, nor a split of its own direction).
//One of its manager's parts, held by the manager's state (CDockState), which holds the trees.
class CDockTree
{
protected:
	CDockManager*	mpcManager;
	CDockState*		mpcState;  //The manager's, shared with its other parts.

public:
	void				Init(CDockManager* pcManager, CDockState* pcState);
	void				Kill(void);

	bool				DockAt(Ptr<CDockNode> pNode, SDockTarget* psTarget);
	void				DockBeside(Ptr<CDockNode> pTarget, Ptr<CDockNode> pNode, EDockSide eSide, int32 iSize);
	void				RemoveNode(Ptr<CDockNode> pNode);
	void				ReplaceInParent(Ptr<CDockNode> pOld, Ptr<CDockNode> pNew);
	void				Tidy(Ptr<CDockNode> pNode);
	void				Flatten(Ptr<CDockSplit> pSplit);
	Ptr<CDockNode>		ClimbToSplit(Ptr<CDockNode> pStart, EDockSide eSide);
	EDockSide			SideOfWell(Ptr<CDockNode> pNode);
	Ptr<CDockGroup>		GatherInto(Ptr<CDockNode> pNode, bool bDocuments);

protected:
	bool				DockAtKind(Ptr<CDockNode> pNode, SDockTarget* psTarget, int32 iSize);
	void				SplitBeside(Ptr<CDockNode> pTarget, Ptr<CDockNode> pNode, EDockSide eSide, int32 iTarget);
	bool				DockAtAreaEdge(Ptr<CDockNode> pNode, Ptr<CDockArea> pArea, EDockSide eSide, int32 iSize);
	bool				DockAtGroup(Ptr<CDockNode> pNode, Ptr<CDockGroup> pGroup, EDockSide eSide, int32 iSize);
	bool				DockBesideWell(Ptr<CDockNode> pNode, EDockSide eSide, int32 iSize);
	bool				DockInWell(Ptr<CDockNode> pNode);
	void				MovePanesInto(Ptr<CDockNode> pNode, Ptr<CDockGroup> pGroup);
	void				SetStates(CArray<CDockPane>* paPanes, EDockPaneState eState);
	void				MergeSplit(Ptr<CDockSplit> pParent, Ptr<CDockSplit> pChild);
};


#endif // __DOCK_TREE_H__

