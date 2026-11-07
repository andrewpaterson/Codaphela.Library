/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "CanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvasDraw::Init(void)
{
	PreInit();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvasDraw::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCanvasDraw::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvasDraw::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCanvasDraw::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CCanvasDraw::GetDesiredSize(void)
{
	SInt32Vec2	sSize;

	//-1, -1 means this draw has no size of its own.
	sSize.Init(-1, -1);
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the draw only fills the canvas with one colour, psColour.  An
// opaque canvas drawn so fills its part of the window itself, without a
// native canvas of its own to draw into and copy (see CCanvas::Draw).
//
//////////////////////////////////////////////////////////////////////////
bool CCanvasDraw::GetSolidColour(ARGB32* psColour)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the draw can draw only part of its canvas (CCanvas::InvalidateArea):
// it draws only through the canvas's DrawBox and DrawPixel, which keep to
// its paint clip, or keeps to the paint clip (CCanvas::GetPaintClip)
// itself.  A draw that can't is drawn whole.
//
//////////////////////////////////////////////////////////////////////////
bool CCanvasDraw::CanDrawPart(void)
{
	return false;
}

