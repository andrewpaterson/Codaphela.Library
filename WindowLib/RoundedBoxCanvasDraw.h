#ifndef __ROUNDED_BOX_CANVAS_DRAW_H__
#define __ROUNDED_BOX_CANVAS_DRAW_H__
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
#include "ImageCanvasDraw.h"
#include "Insets.h"


//Draws a block of one colour with rounded corners, anti-aliased, inset from the canvas's sides, like the highlight on a
//menu's selected row.  What it leaves empty is transparent, so the canvas should be.  The block is drawn into a cel
//the size of the canvas, made again only when the canvas changes size, and an image draw puts the cel on the canvas.
//A radius of zero gives a square block, like a separator's line inset from the ends of its row.
class CRoundedBoxCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CRoundedBoxCanvasDraw);
DESTRUCTABLE(CRoundedBoxCanvasDraw);
protected:
	Ptr<CImageCanvasDraw>	mpCelDraw;
	ARGB32		msColour;
	int32		miRadius;
	SInsets		msInsets;
	SInt32Vec2	msCelSize;  //The canvas size the cel was made for, -1, -1 for none yet.

public:
	void		Init(ARGB32 sColour, int32 iRadius, SInsets* psInsets);
	void		Class(void) override;
	void 		Free(void) override;
	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	bool		Draw(Ptr<CCanvas> pCanvas) override;
	void		SetColour(ARGB32 sColour);
	ARGB32		GetColour(void);

protected:
	void		MakeCel(Ptr<CCanvas> pCanvas, SInt32Vec2 sSize);
};

#endif // __ROUNDED_BOX_CANVAS_DRAW_H__

