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
#ifndef __RENDER_FACTORY_H__
#define __RENDER_FACTORY_H__
#include "BaseLib/Constructable.h"
#include "StandardLib/Pointer.h"
#include "CookedMesh.h"
#include "Renderer.h"
#include "RenderMesh.h"


//Makes a renderer, and what it draws, for one kind of device.  The app makes the factory, as it does a CNativeWindowFactory.
//It isn't an object in the graph, so it outlives everything it makes.  A render mesh is only ever made whole from a cooked
//mesh, and made again when the mesh changes.
class CRenderFactory : public CConstructable
{
CONSTRUCTABLE(CRenderFactory);
public:
	virtual Ptr<CRenderer>		CreateRenderer(void) =0;
	virtual Ptr<CRenderMesh>	CreateMesh(Ptr<CRenderer> pRenderer, CCookedMesh* pcCooked) =0;
};


#endif // __RENDER_FACTORY_H__

