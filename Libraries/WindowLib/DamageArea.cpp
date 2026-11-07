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
#include "DamageArea.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::Init(void)
{
	muiNumRectangles = 0;
	mbAll = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::Kill(void)
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// An empty rectangle adds nothing.  Rectangles already there that it
// covers go.
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::Add(CInt32Rect* pcRectangle)
{
	size	i;
	size	uiKept;

	if (mbAll || (pcRectangle->GetWidth() <= 0) || (pcRectangle->GetHeight() <= 0))
	{
		return;
	}

	for (i = 0; i < muiNumRectangles; i++)
	{
		if (pcRectangle->Inside(&macRectangles[i]))
		{
			return;
		}
	}

	uiKept = 0;
	for (i = 0; i < muiNumRectangles; i++)
	{
		if (!macRectangles[i].Inside(pcRectangle))
		{
			macRectangles[uiKept] = macRectangles[i];
			uiKept++;
		}
	}
	muiNumRectangles = uiKept;

	if (muiNumRectangles == DAMAGE_AREA_MAX_RECTANGLES)
	{
		for (i = 1; i < muiNumRectangles; i++)
		{
			macRectangles[0].GrowToContain(&macRectangles[i]);
		}
		macRectangles[0].GrowToContain(pcRectangle);
		muiNumRectangles = 1;
		return;
	}

	macRectangles[muiNumRectangles].Init(pcRectangle);
	muiNumRectangles++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::AddAll(void)
{
	mbAll = true;
	muiNumRectangles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::Clear(void)
{
	mbAll = false;
	muiNumRectangles = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcRectangle overlaps the area.
//
//////////////////////////////////////////////////////////////////////////
bool CDamageArea::Intersects(CInt32Rect* pcRectangle)
{
	size	i;

	if ((pcRectangle->GetWidth() <= 0) || (pcRectangle->GetHeight() <= 0))
	{
		return false;
	}
	if (mbAll)
	{
		return true;
	}
	for (i = 0; i < muiNumRectangles; i++)
	{
		if (pcRectangle->Intersect(&macRectangles[i]))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle around every rectangle in the area.  Not meaningful when
// the area is all of the window or empty.
//
//////////////////////////////////////////////////////////////////////////
void CDamageArea::GetBounds(CInt32Rect* pcDest)
{
	size	i;

	pcDest->Init(0, 0, 0, 0);
	if (muiNumRectangles == 0)
	{
		return;
	}
	pcDest->Init(&macRectangles[0]);
	for (i = 1; i < muiNumRectangles; i++)
	{
		pcDest->GrowToContain(&macRectangles[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDamageArea::IsEmpty(void) { return !mbAll && (muiNumRectangles == 0); }
bool CDamageArea::IsAll(void) { return mbAll; }
size CDamageArea::NumRectangles(void) { return muiNumRectangles; }
CInt32Rect* CDamageArea::GetRectangle(size uiIndex) { return &macRectangles[uiIndex]; }

