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
#ifndef __RENDER_MESH_H__
#define __RENDER_MESH_H__
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "Renderer.h"


//What a renderer draws for a mesh, made from it by the renderer's CRenderFactory.  Each kind of device has its own.  It holds a
//Nes pointer to its renderer, so it's freed first and can still use the renderer in its Free.
class CRenderMesh : public CObject
{
CONSTRUCTABLE(CRenderMesh);
DESTRUCTABLE(CRenderMesh);
protected:
	Ptr<CRenderer>	mpRenderer;

public:
	void			Init(Ptr<CRenderer> pRenderer);
	void			Class(void) override;
	void			Free(void) override;

	Ptr<CRenderer>	GetRenderer(void);
};


#endif // __RENDER_MESH_H__

