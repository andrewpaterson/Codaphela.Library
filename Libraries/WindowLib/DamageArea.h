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
#ifndef __DAMAGE_AREA_H__
#define __DAMAGE_AREA_H__
#include "SupportLib/Int32Rect.h"


#define DAMAGE_AREA_MAX_RECTANGLES	8


//The parts of a window that must be drawn again: a few rectangles in window client space, right and bottom
//exclusive, or all of it.  A rectangle inside one already there adds nothing; one more than the most kept merges them
//all into their bounds, so the area never misses what was added, it only grows.
class CDamageArea
{
protected:
	CInt32Rect	macRectangles[DAMAGE_AREA_MAX_RECTANGLES];
	size		muiNumRectangles;
	bool		mbAll;

public:
	void		Init(void);
	void		Kill(void);

	void		Add(CInt32Rect* pcRectangle);
	void		AddAll(void);
	void		Clear(void);

	bool		IsEmpty(void);
	bool		IsAll(void);
	size		NumRectangles(void);
	CInt32Rect*	GetRectangle(size uiIndex);
	bool		Intersects(CInt32Rect* pcRectangle);
	void		GetBounds(CInt32Rect* pcDest);
};


#endif // __DAMAGE_AREA_H__

