#ifndef __IMAGE_CANVAS_DRAW_H__
#define __IMAGE_CANVAS_DRAW_H__
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
#include "SupportLib/ImageCelBlitterCache.h"
#include "WindowLib/CanvasDraw.h"


class CImageCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CImageCanvasDraw);
DESTRUCTABLE(CImageCanvasDraw);
protected:
	Ptr<CImageCel>				mpCel;
	Ptr<CImageCelBlitterCache>	mpBlitterCache;
	Ptr<CImageBlitter>			mpBlitter;
	Ptr<CImage>					mpDestImage;

public:
	void		Init(Ptr<CImageCel> pCel);
	void		Class(void) override;
	void 		Free(void) override;

	bool		Save(CObjectWriter* pcFile) override;
	bool		Load(CObjectReader* pcFile) override;

	bool		Draw(Ptr<CCanvas> pCanvas) override;
	SInt32Vec2	GetDesiredSize(void) override;
};


#endif // __IMAGE_CANVAS_DRAW_H__

