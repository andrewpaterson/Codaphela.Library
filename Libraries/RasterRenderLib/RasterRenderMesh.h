/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_RENDER_MESH_H__
#define __RASTER_RENDER_MESH_H__
#include "RasterLib/RasterMesh.h"
#include "RenderLib/RenderMesh.h"
#include "RasterRenderer.h"


class CMesh;


//A mesh as the raster renderer draws it: a CRasterMesh built from a CMesh where it is, at the renderer's scale.
class CRasterRenderMesh : public CRenderMesh
{
CONSTRUCTABLE(CRasterRenderMesh);
DESTRUCTABLE(CRasterRenderMesh);
protected:
	Ptr<CRasterMesh>	mpRasterMesh;

public:
	void				Init(Ptr<CRasterRenderer> pRenderer, CMesh* pcMesh);
	void				Class(void) override;
	void				Free(void) override;

	Ptr<CRasterMesh>	GetRasterMesh(void);
};


#endif // __RASTER_RENDER_MESH_H__

