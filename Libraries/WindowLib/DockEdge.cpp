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
#include "DockManager.h"
#include "DockDrag.h"
#include "DockEdge.h"


//////////////////////////////////////////////////////////////////////////
//
// uiEdges is the DOCK_EDGE_* it moves.
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager, CDockEdgeOwner* pcOwner, uint8 uiEdges, ARGB32 sColour)
{
	bool	bHorizontal;
	bool	bVertical;

	PreInit();

	CCanvas::Init(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	mpcOwner = pcOwner;
	muiEdges = uiEdges;
	mbDragging = false;
	mpManager = pManager;
	SetCanGetFocus(true);
	SetAlignment(CSH_Stretch, CSV_Stretch);

	bHorizontal = (uiEdges & (DOCK_EDGE_LEFT | DOCK_EDGE_RIGHT)) != 0;
	bVertical = (uiEdges & (DOCK_EDGE_TOP | DOCK_EDGE_BOTTOM)) != 0;
	if (bHorizontal && bVertical)
	{
		if (((uiEdges & DOCK_EDGE_LEFT) && (uiEdges & DOCK_EDGE_TOP)) || ((uiEdges & DOCK_EDGE_RIGHT) && (uiEdges & DOCK_EDGE_BOTTOM)))
		{
			SetCursor(NC_SizeNorthWestSouthEast);
		}
		else
		{
			SetCursor(NC_SizeNorthEastSouthWest);
		}
	}
	else if (bHorizontal)
	{
		SetCursor(NC_SizeWestEast);
	}
	else
	{
		SetCursor(NC_SizeNorthSouth);
	}

	AddInputAction(CIC_ActivateComponent, this, &CDockEdge::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CDockEdge::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CDockEdge::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CDockEdge::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::Class(void)
{
	CCanvas::Class();

	U_Pointer(mpcOwner);
	U_UInt8(muiEdges);
	U_Bool(mbDragging);
	M_Pointer(mpManager);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockEdge::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockEdge::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::ActivateComponent(int x, int y)
{
	SInt32Vec2	sScreen;

	mbDragging = true;
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
	mpWindow->GetPointerScreenPosition(&sScreen);
	mpcOwner->EdgeDragStarted(this, sScreen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::ExecuteComponent(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::ContDragSelect(int x, int y)
{
	SInt32Vec2	sScreen;

	if (mbDragging)
	{
		mpWindow->GetPointerScreenPosition(&sScreen);
		mpcOwner->EdgeDragged(this, sScreen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockEdge::StopDragSelect(int x, int y)
{
	if (mbDragging)
	{
		mbDragging = false;
		mpcOwner->EdgeReleased(this);
		if (mpManager.IsNotNull())
		{
			mpManager->RestoreFocus(mpWindow);
		}
	}
}


uint8	CDockEdge::GetEdges(void) { return muiEdges; }
bool	CDockEdge::IsDragging(void) { return mbDragging; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::Init(Ptr<CWindow> pWindow, Ptr<CDockManager> pManager)
{
	PreInit();

	CComponent::Init(pWindow);
	mpManager = pManager;
	SetCanGetFocus(true);
	SetCursor(NC_Move);
	AddInputAction(CIC_ExecuteComponent, this, &CDockDragTracker::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CDockDragTracker::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CDockDragTracker::StopDragSelect);
	AddInputAction(CIC_ClearSelection, this, &CDockDragTracker::Cancel);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::Class(void)
{
	CComponent::Class();

	M_Pointer(mpManager);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDragTracker::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockDragTracker::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Button one released: the drag stops, which drops what is dragged.
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::ExecuteComponent(int x, int y)
{
	mpWindow->GetFocus()->StopDrag(FD_Select);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::ContDragSelect(int x, int y)
{
	mpManager->GetDrag()->Moved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::StopDragSelect(int x, int y)
{
	mpManager->GetDrag()->Dropped();
}


//////////////////////////////////////////////////////////////////////////
//
// Escape puts what is dragged back where it was.
//
//////////////////////////////////////////////////////////////////////////
void CDockDragTracker::Cancel(void)
{
	mpManager->GetDrag()->Cancelled();
	mpWindow->GetFocus()->StopDrag(FD_Select);
}

