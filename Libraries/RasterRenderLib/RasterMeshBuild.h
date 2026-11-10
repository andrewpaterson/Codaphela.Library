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
#ifndef __RASTER_MESH_BUILD_H__
#define __RASTER_MESH_BUILD_H__
#include "BaseLib/PrimitiveTypes.h"


class CMesh;
class CRasterMesh;


//Makes the integer copy of a CMesh that CRasteriser draws, replacing what pcRaster had.  This is where a mesh's floats become
//what RasterLib wants, so that RasterLib knows nothing of CMesh.
//
//The first centres the mesh on the origin and scales it so that its largest half width is RASTER_MESH_HALF_EXTENT.  The other
//keeps the mesh where it is at a scale that's given, for meshes that are to be drawn together at their true sizes (see
//SRasterInstance).
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh);
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh, int32 iScale);


#endif // __RASTER_MESH_BUILD_H__

