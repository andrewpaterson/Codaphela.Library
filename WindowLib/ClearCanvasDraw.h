#ifndef __CLEAR_CANVAS_DRAW_H__
#define __CLEAR_CANVAS_DRAW_H__
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
#include "SupportLib/ColourARGB32.h"
#include "CanvasDraw.h"


//Fills the whole canvas with one colour.  Given to a window it clears the window before its components are drawn
//so nothing is left behind where a component used to be.
class CClearCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CClearCanvasDraw);
DESTRUCTABLE(CClearCanvasDraw);
protected:
	ARGB32	msColour;

public:
	void	Init(ARGB32 sColour);
	void	Class(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	void	SetColour(ARGB32 sColour);
	ARGB32	GetColour(void);
};


#endif // __CLEAR_CANVAS_DRAW_H__

