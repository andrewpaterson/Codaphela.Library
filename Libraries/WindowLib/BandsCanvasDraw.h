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
#ifndef __BANDS_CANVAS_DRAW_H__
#define __BANDS_CANVAS_DRAW_H__
#include "BaseLib/Int32Vec2.h"
#include "SupportLib/ColourARGB32.h"
#include "CanvasDraw.h"


//Fills bands across a canvas's width in one colour: each a top (x) and a height (y) in the canvas.  A list draws its
//selected rows with one when several can be selected.  The canvas should be transparent, as what is between the
//bands isn't drawn.
class CBandsCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CBandsCanvasDraw);
DESTRUCTABLE(CBandsCanvasDraw);
protected:
	ARGB32			msColour;
	CArrayInt32Vec2	masBands;

public:
	void	Init(ARGB32 sColour);
	void	Class(void) override;
	void	Free(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	void	SetColour(ARGB32 sColour);
	ARGB32	GetColour(void);
	void	ClearBands(void);
	void	AddBand(int32 iTop, int32 iHeight);
	size	NumBands(void);
	SInt32Vec2	GetBand(size uiIndex);
};


#endif // __BANDS_CANVAS_DRAW_H__

