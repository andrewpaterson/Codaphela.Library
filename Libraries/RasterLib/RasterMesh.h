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


//How much of a skinned vertex a matrix moves: RASTER_SKIN_ONE is all of it.
#define RASTER_SKIN_WEIGHT_BITS	15
#define RASTER_SKIN_ONE			(1 << RASTER_SKIN_WEIGHT_BITS)


//One of a skinned vertex's matrices, and how much of it that moves.
struct SRasterSkinWeight
{
	uint16	uiMatrix;
	uint16	uiWeight;
};


//A skinned vertex's weights: uiCount of them in the mesh's masSkinWeights from uiFirst.
struct SRasterSkinVertex
{
	uint32	uiFirst;
	uint32	uiCount;
};


typedef CArrayTemplate<SRasterVertex>		CArrayRasterVertex;
typedef CArrayTemplate<SRasterTriangle>		CArrayRasterTriangle;
typedef CArrayTemplate<SRasterSkinVertex>	CArrayRasterSkinVertex;
typedef CArrayTemplate<SRasterSkinWeight>	CArrayRasterSkinWeight;


//The integer copy of a mesh that CRasteriser draws.  RasterRenderLib's BuildRasterMesh makes one from a CMesh, as RasterLib
//knows nothing of the scene.  Clear empties it, and UpdateBounds sets msMin and msMax from its vertices.  It's a CObject so
//that anything can share one through a Ptr, with or without a scene.  SRasterInstance points at one, and whatever builds
//the instances holds the Ptrs, which keeps the meshes alive while they're drawn.
//
//RemoveDegenerate takes out triangles with no area, which BuildRasterMesh does.  What's exported from CAD is cleaned up and made
//simpler for how big it will be drawn by modifiers on the CMesh it's built from (see CMeshFixReversed, CMeshWeldCorners and
//CMeshEdgeCollapse).
//
//Building one from a big mesh takes seconds and WriteFile and ReadFile take none: uiSource is whatever says which source it
//was built from, and ReadFile refuses a file built from another.
//
//A skinned mesh's vertices are where it was bound, and each has its weights in masSkinVertices: most first, adding up to
//exactly RASTER_SKIN_ONE.  muiNumMatrices is how many matrices pose it (see CRasterPosed).  A rigid mesh has none of these.
class CRasterMesh : public CObject
{
CONSTRUCTABLE(CRasterMesh);
DESTRUCTABLE(CRasterMesh);
public:
	CArrayRasterVertex		masVertices;
	CArrayRasterTriangle	masOpaque;
	CArrayRasterTriangle	masTransparent;
	CArrayRasterSkinVertex	masSkinVertices;  //One for each vertex if it's skinned.
	CArrayRasterSkinWeight	masSkinWeights;
	uint32					muiNumMatrices;
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
	bool	IsSkinned(void);
};


#endif // __RASTER_MESH_H__

