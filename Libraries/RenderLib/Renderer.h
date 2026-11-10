/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RENDERER_H__
#define __RENDERER_H__
#include "StandardLib/Object.h"


//One device that draws, and what its meshes share.  Each kind of device has its own, made by its CRenderFactory.  A render mesh
//holds a Nes pointer to its renderer, so every mesh is freed before the renderer is.
class CRenderer : public CObject
{
CONSTRUCTABLE(CRenderer);
DESTRUCTABLE(CRenderer);
public:
	void	Init(void);
	void	Class(void) override;
	void	Free(void) override;
};


#endif // __RENDERER_H__

