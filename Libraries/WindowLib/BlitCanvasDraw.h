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
#ifndef __BLIT_CANVAS_DRAW_H__
#define __BLIT_CANVAS_DRAW_H__
#include "SupportLib/Image.h"
#include "CanvasDraw.h"


//A canvas draw that blits onto an image: the canvas's own if it has one, else an image of its own the canvas's size and
//colour format, copied onto the canvas when the blit is done.  A native canvas starts black, so a new image does too;
//on a transparent canvas, as it is cleared, the image is cleared before each draw.
class CBlitCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CBlitCanvasDraw);
DESTRUCTABLE(CBlitCanvasDraw);
protected:
	Ptr<CImage>		mpDestImage;  //NULL while the canvas has an image of its own.

public:
	void			Init(void);
	void			Class(void) override;

protected:
	Ptr<CImage>		StartBlit(Ptr<CCanvas> pCanvas);
	void			FinishBlit(Ptr<CCanvas> pCanvas);
};


#endif // __BLIT_CANVAS_DRAW_H__

