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
#ifndef __ROUNDED_CORNER_CANVAS_DRAW_H__
#define __ROUNDED_CORNER_CANVAS_DRAW_H__
#include "SupportLib/ColourARGB32.h"
#include "ImageCanvasDraw.h"


enum ERoundedCorner
{
	RC_TopLeft,
	RC_TopRight,
	RC_BottomLeft,
	RC_BottomRight,
	RC_NumCorners,
};


//Fills what lies outside a quarter circle in one corner of a square canvas, anti-aliased, with the colour around
//something with rounded corners.  The circle's centre is the canvas's opposite corner and its radius the canvas's
//width, so the canvas is as big as the radius.  Inside the circle is left transparent, so the canvas should be.  The
//piece is drawn into a cel, made again only when the canvas changes size, as CRoundedBoxCanvasDraw's block is.
class CRoundedCornerCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CRoundedCornerCanvasDraw);
DESTRUCTABLE(CRoundedCornerCanvasDraw);
protected:
	Ptr<CImageCanvasDraw>	mpCelDraw;
	ARGB32					msColour;
	ERoundedCorner			meCorner;
	SInt32Vec2				msCelSize;  //The canvas size the cel was made for, -1, -1 for none yet.

public:
	void			Init(ARGB32 sColour, ERoundedCorner eCorner);
	void			Class(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			Draw(Ptr<CCanvas> pCanvas) override;

	void			SetColour(ARGB32 sColour);
	ARGB32			GetColour(void) const;
	ERoundedCorner	GetCorner(void) const;

protected:
	void			MakeCel(Ptr<CCanvas> pCanvas, int32 iRadius);
};


#endif // __ROUNDED_CORNER_CANVAS_DRAW_H__

