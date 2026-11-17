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
#ifndef __CEL_CANVAS_DRAW_H__
#define __CEL_CANVAS_DRAW_H__
#include "SupportLib/ImageDraw.h"
#include "ImageCanvasDraw.h"


//A canvas draw that paints with a CImageDraw (its anti-aliased shapes) into a cel the size of the canvas, which an image
//draw puts on the canvas.  The cel is only painted again when the canvas's size changes or Repaint says what's painted
//has.  What isn't painted is see-through, for a transparent canvas (CCanvas::SetTransparent).
class CCelCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CCelCanvasDraw);
DESTRUCTABLE(CCelCanvasDraw);
protected:
	Ptr<CImageCanvasDraw>	mpCelDraw;
	SInt32Vec2				msCelSize;  //The canvas size the cel was painted for, -1, -1 to paint it again.

public:
	void	Init(void);
	void	Class(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;
	void	Repaint(void);
	bool	IsPainted(void);

protected:
	virtual void	Paint(CImageDraw* pcDraw, SInt32Vec2 sSize) =0;  //Positions are pixels from the canvas's top left.
};


#endif // __CEL_CANVAS_DRAW_H__

