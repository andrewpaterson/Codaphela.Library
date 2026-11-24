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
#include "Sash.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSash::Init(Ptr<CWindow> pWindow, bool bHorizontal, ARGB32 sColour)
{
	PreInit();

	CCanvas::Init(pWindow, OMalloc<CClearCanvasDraw>(sColour));
	mbDragging = false;
	miPressed = 0;
	mcMoved.Init();
	mcReleased.Init();
	SetCanGetFocus(true);
	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetHorizontal(bHorizontal);
	AddInputAction(CIC_ActivateComponent, this, &CSash::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CSash::ExecuteComponent);
	AddInputAction(CIC_ContDragSelect, this, &CSash::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CSash::StopDragSelect);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSash::Class(void)
{
	CCanvas::Class();

	U_Bool(mbHorizontal);
	U_Bool(mbDragging);
	U_Int32(miPressed);
	U_Unknown(CSashMovedEvent, mcMoved);
	U_Unknown(CSashEvent, mcReleased);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSash::Free(void)
{
	mcReleased.Kill();
	mcMoved.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSash::Save(CObjectWriter* pcFile) { return false; }
bool CSash::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// A horizontal sash is upright and moved left and right.
//
//////////////////////////////////////////////////////////////////////////
void CSash::SetHorizontal(bool bHorizontal)
{
	mbHorizontal = bHorizontal;
	SetCursor(bHorizontal ? NC_SizeWestEast : NC_SizeNorthSouth);
}


bool CSash::IsHorizontal(void) const { return mbHorizontal; }
bool CSash::IsDragging(void) const { return mbDragging; }
CSashMovedEvent* CSash::GetMovedEvent(void) { return &mcMoved; }
CSashEvent* CSash::GetReleasedEvent(void) { return &mcReleased; }


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CSash::ActivateComponent(int x, int y)
{
	SInt32Vec2	sPosition;

	sPosition = GetPosition();
	miPressed = mbHorizontal ? sPosition.x + x : sPosition.y + y;
	mbDragging = true;
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSash::ExecuteComponent(int x, int y)
{
	if (mbDragging)
	{
		mpWindow->GetFocus()->StopDrag(FD_Select);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the sash.
//
//////////////////////////////////////////////////////////////////////////
void CSash::ContDragSelect(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iAlong;

	if (!mbDragging)
	{
		return;
	}
	sPosition = GetPosition();
	iAlong = mbHorizontal ? sPosition.x + x : sPosition.y + y;
	if (iAlong != miPressed)
	{
		mcMoved.Call(this, iAlong - miPressed);
		miPressed = iAlong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSash::StopDragSelect(int x, int y)
{
	mbDragging = false;
	mcReleased.Call(this);
}

