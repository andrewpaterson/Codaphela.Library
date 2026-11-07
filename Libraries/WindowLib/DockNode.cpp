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
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "DockManager.h"
#include "DockPane.h"
#include "DockGroup.h"
#include "DockSplit.h"
#include "DockArea.h"
#include "DocumentWell.h"
#include "DockNode.h"


static const char* gaszDockSides[] = { "Left", "Right", "Top", "Bottom", "Centre", "None" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockNode::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager)
{
	CContainer::Init(pWindow);
	mpManager = pManager;
	miDockSize = 0;
	SetAlignment(CSH_Stretch, CSV_Stretch);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockNode::Class(void)
{
	CContainer::Class();

	M_Pointer(mpManager);
	U_Int32(miDockSize);
}


//////////////////////////////////////////////////////////////////////////
//
// Layouts are written as XML by CDockLayoutWriter.
//
//////////////////////////////////////////////////////////////////////////
bool CDockNode::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockNode::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockNode::ContainsDocumentWell(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The deepest group whose bounds hold x, y in window client space.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockNode::FindGroupAt(int x, int y)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if any pane in it is a document.
//
//////////////////////////////////////////////////////////////////////////
bool CDockNode::HasDocuments(void)
{
	size	i;

	for (i = 0; i < CountPanes(); i++)
	{
		if (GetTreePane(i)->IsDocument())
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the node takes the state, docked in the main area or
// floating.
//
//////////////////////////////////////////////////////////////////////////
void CDockNode::SetPaneStates(EDockPaneState eState)
{
	size	i;

	for (i = 0; i < CountPanes(); i++)
	{
		GetTreePane(i)->SetState(eState);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every pane in the node remembers it floated at pcRectangle.
//
//////////////////////////////////////////////////////////////////////////
void CDockNode::SetPaneFloatRectangles(CInt32Rect* pcRectangle)
{
	size	i;

	for (i = 0; i < CountPanes(); i++)
	{
		GetTreePane(i)->SetFloatRectangle(pcRectangle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The split or well the node is in, NULL if its parent isn't a node (a
// dock area, or nothing).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockNode::GetParentNode(void)
{
	return Ptr<CDockNode>(dynamic_cast<CDockNode*>(&mpParent));
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the node's parent isn't a split.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockSplit> CDockNode::GetParentSplit(void)
{
	Ptr<CDockNode>	pParent;

	pParent = GetParentNode();
	return pParent.IsNotNull() ? pParent->AsSplit() : Ptr<CDockSplit>();
}


//////////////////////////////////////////////////////////////////////////
//
// The dock area the node is the root of, NULL if it isn't a root.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockArea> CDockNode::GetParentArea(void)
{
	return Ptr<CDockArea>(dynamic_cast<CDockArea*>(&mpParent));
}


//////////////////////////////////////////////////////////////////////////
//
// The split, well or area the node is in, NULL if it is in none.
//
//////////////////////////////////////////////////////////////////////////
CDockNodeHolder* CDockNode::GetHolder(void)
{
	return dynamic_cast<CDockNodeHolder*>(&mpParent);
}


//////////////////////////////////////////////////////////////////////////
//
// The node as a split, NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockSplit> CDockNode::AsSplit(void)
{
	return Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(this));
}


//////////////////////////////////////////////////////////////////////////
//
// The node as a group, NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDockNode::AsGroup(void)
{
	return Ptr<CDockGroup>(dynamic_cast<CDockGroup*>(this));
}


//////////////////////////////////////////////////////////////////////////
//
// The node as the document well, NULL if it isn't it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDocumentWell> CDockNode::AsWell(void)
{
	return Ptr<CDocumentWell>(dynamic_cast<CDocumentWell*>(this));
}


void				CDockNode::SetDockSize(int32 iSize) { miDockSize = iSize; }
int32				CDockNode::GetDockSize(void) { return miDockSize; }
Ptr<CDockManager>	CDockNode::GetManager(void) { return mpManager; }
bool				CDockNode::IsSplit(void) { return GetKind() == DNK_Split; }
bool				CDockNode::IsGroup(void) { return GetKind() == DNK_Group; }
bool				CDockNode::IsDocumentWell(void) { return GetKind() == DNK_DocumentWell; }


//////////////////////////////////////////////////////////////////////////
//
// Left and right are beside something in a horizontal split.
//
//////////////////////////////////////////////////////////////////////////
bool IsHorizontalDockSide(EDockSide eSide)
{
	return (eSide == DS_Left) || (eSide == DS_Right);
}


//////////////////////////////////////////////////////////////////////////
//
// Right and bottom come after what they are beside in its split.
//
//////////////////////////////////////////////////////////////////////////
bool IsAfterDockSide(EDockSide eSide)
{
	return (eSide == DS_Right) || (eSide == DS_Bottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* DockSideToString(EDockSide eSide)
{
	if ((eSide < DS_Left) || (eSide > DS_None))
	{
		return gaszDockSides[DS_None];
	}
	return gaszDockSides[eSide];
}


//////////////////////////////////////////////////////////////////////////
//
// DS_None for anything else.
//
//////////////////////////////////////////////////////////////////////////
EDockSide StringToDockSide(const char* szSide)
{
	int		i;

	if (szSide == NULL)
	{
		return DS_None;
	}
	for (i = DS_Left; i < DS_None; i++)
	{
		if (StringCompare(szSide, gaszDockSides[i]) == 0)
		{
			return (EDockSide)i;
		}
	}
	return DS_None;
}

