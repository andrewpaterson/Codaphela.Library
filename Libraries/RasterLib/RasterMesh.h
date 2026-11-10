/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RASTER_MESH_H__
#define __RASTER_MESH_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/Int32Vec3.h"
#include "BaseLib/Vec3.h"
#include "StandardLib/Object.h"


//A part of a unit normal.
typedef SFixed<int16, int32, 14>	SRasterUnit;
typedef SVec3<SRasterUnit, int32>	SRasterUnitVec3;

//The mesh is centred and scaled so that its largest half width is this many units.
#define RASTER_MESH_HALF_EXTENT	(1 << 20)


struct SRasterVertex
{
	SInt32Vec3	sPosition;
	SRasterUnitVec3	sNormal;
};


//A triangle has one colour, 0xAARRGGBB.  The colour of its first corner if the source mesh had more than one.
struct SRasterTriangle
{
	int32	aiVertex[3];
	uint32	uiColour;
};


typedef CArrayTemplate<SRasterVertex>	CArrayRasterVertex;
typedef CArrayTemplate<SRasterTriangle>	CArrayRasterTriangle;


//The integer copy of a mesh that CRasteriser draws.  RasterRenderLib's BuildRasterMesh makes one from a CMesh, as RasterLib
//knows nothing of the scene.  Clear empties it, and UpdateBounds sets msMin and msMax from its vertices.  It's a CObject so
//that anything can share one through a Ptr, with or without a scene.  SRasterInstance points at one, and whatever builds
//the instances holds the Ptrs, which keeps the meshes alive while they're drawn.
//
//RemoveDegenerate takes out triangles with no area, which can't be seen.  What's exported from CAD is cleaned up and made
//simpler for how big it will be drawn by modifiers on the CMesh it's built from (see CMeshFixReversed, CMeshWeldCorners and
//CMeshEdgeCollapse).
//
//Building one from a big mesh takes seconds and WriteFile and ReadFile take none: uiSource is whatever says which source it
//was built from, and ReadFile refuses a file built from another.
class CRasterMesh : public CObject
{
CONSTRUCTABLE(CRasterMesh);
DESTRUCTABLE(CRasterMesh);
public:
	CArrayRasterVertex		masVertices;
	CArrayRasterTriangle	masOpaque;
	CArrayRasterTriangle	masTransparent;
	SInt32Vec3				msMin;  //The bounds of its vertices.
	SInt32Vec3				msMax;

	void	Init(void);
	void	Free(void) override;
	void	Class(void) override;

	void	Clear(void);
	void	UpdateBounds(void);

	size	RemoveDegenerate(void);

	bool	WriteFile(const char* szFilename, uint64 uiSource);
	bool	ReadFile(const char* szFilename, uint64 uiSource);

	size	NumTriangles(void);
};


#endif // __RASTER_MESH_H__

