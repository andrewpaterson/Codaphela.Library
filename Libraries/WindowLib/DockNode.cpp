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
	CArray<CDockPane>	aPanes;
	size				i;
	bool				bDocuments;

	aPanes.Init();
	GetPanes(&aPanes);
	bDocuments = false;
	for (i = 0; i < aPanes.Size(); i++)
	{
		if (aPanes.Get(i)->IsDocument())
		{
			bDocuments = true;
			break;
		}
	}
	aPanes.Kill();
	return bDocuments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockNode::IsEmpty(void)
{
	CArray<CDockPane>	aPanes;
	bool				bEmpty;

	aPanes.Init();
	GetPanes(&aPanes);
	bEmpty = aPanes.Size() == 0;
	aPanes.Kill();
	return bEmpty;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the node's parent isn't a split.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockSplit> CDockNode::GetParentSplit(void)
{
	return Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&mpParent));
}


void				CDockNode::SetDockSize(int32 iSize) { miDockSize = iSize; }
int32				CDockNode::GetDockSize(void) { return miDockSize; }
Ptr<CDockManager>	CDockNode::GetManager(void) { return mpManager; }
bool				CDockNode::IsSplit(void) { return GetKind() == DNK_Split; }
bool				CDockNode::IsGroup(void) { return GetKind() == DNK_Group; }
bool				CDockNode::IsDocumentWell(void) { return GetKind() == DNK_DocumentWell; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EDockSide OppositeDockSide(EDockSide eSide)
{
	switch (eSide)
	{
	case DS_Left:
		return DS_Right;
	case DS_Right:
		return DS_Left;
	case DS_Top:
		return DS_Bottom;
	case DS_Bottom:
		return DS_Top;
	default:
		return eSide;
	}
}


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

