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
#include "DockGroup.h"
#include "DocumentWell.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, ARGB32 sBackground)
{
	PreInit();

	CDockNode::Init(pWindow, pManager);
	mpRoot = NULL;
	mpBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(sBackground));
	mpBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(mpBackground);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::Class(void)
{
	CDockNode::Class();

	M_Pointer(mpRoot);
	M_Pointer(mpBackground);
}


EDockNodeKind CDocumentWell::GetKind(void) { return DNK_DocumentWell; }
bool CDocumentWell::ContainsDocumentWell(void) { return true; }
Ptr<CDockNode> CDocumentWell::GetRoot(void) { return mpRoot; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::GetPanes(CArray<CDockPane>* paPanes)
{
	if (mpRoot.IsNotNull())
	{
		mpRoot->GetPanes(paPanes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDocumentWell::FindGroupAt(int x, int y)
{
	if (mpRoot.IsNotNull() && mpRoot->IsPointIn(x, y))
	{
		return mpRoot->FindGroupAt(x, y);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockGroup> CDocumentWell::GetFirstGroup(void)
{
	if (mpRoot.IsNotNull())
	{
		return mpRoot->GetFirstGroup();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::SetRequiredSize(void)
{
	msRequiredSize.Init(DOCK_MINIMUM_SIZE, DOCK_MINIMUM_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL leaves the well empty, showing its background.
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::SetRoot(Ptr<CDockNode> pRoot)
{
	RemoveAllComponents();
	mpRoot = pRoot;
	if (mpRoot.IsNotNull())
	{
		AddComponent(mpRoot);
	}
	else
	{
		AddComponent(mpBackground);
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDocumentWell::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SetActualBounds(sPosition, sAreaSize);
	SetContainerBounds(sPosition, sAreaSize);
	if (mpRoot.IsNotNull())
	{
		mpRoot->Layout(sPosition, sAreaSize);
	}
	else
	{
		mpBackground->Layout(sPosition, sAreaSize);
	}
}

