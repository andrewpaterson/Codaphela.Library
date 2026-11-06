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
#ifndef __MAPS_CANVAS_DRAW_H__
#define __MAPS_CANVAS_DRAW_H__
#include "SupportLib/Maps.h"
#include "BlitCanvasDraw.h"


class CMapsCanvasDraw : public CBlitCanvasDraw
{
CONSTRUCTABLE(CMapsCanvasDraw);
DESTRUCTABLE(CMapsCanvasDraw);
protected:
	Ptr<CMaps>		mpMaps;

public:
	void	Init(Ptr<CMaps> pMaps);
	void	Class(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;
};


#endif // __MAPS_CANVAS_DRAW_H__

