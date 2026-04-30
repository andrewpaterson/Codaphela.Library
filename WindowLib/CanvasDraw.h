#ifndef __CANVAS_DRAW_H__
#define __CANVAS_DRAW_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


/* The only import method in this class is

Draw()

Everything else just exists as a useful template to copy/paste objects from.

*/


class CCanvas;
class CCanvasDraw : public CObject
{
CONSTRUCTABLE(CCanvasDraw);
DESTRUCTABLE(CCanvasDraw);
protected:
public:
			void	Init(void);
			void	Class(void) override;
			void 	Free(void) override;

			bool	Save(CObjectWriter* pcFile) override;
			bool	Load(CObjectReader* pcFile) override;

	virtual bool	Draw(Ptr<CCanvas> pCanvas) =0;
};


#endif // __CANVAS_DRAW_H__

