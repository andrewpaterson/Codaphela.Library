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
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Int16Vec3.h"
#include "BaseLib/Int32Vec3.h"


//Normals are fixed point: 1.0 is 16384.
#define RASTER_NORMAL_BITS		14

//The mesh is centred and scaled so that its largest half width is this many units.
#define RASTER_MESH_HALF_EXTENT	(1 << 20)


struct SRasterVertex
{
	SInt32Vec3	sPosition;
	SInt16Vec3	sNormal;
};


//A triangle has one colour, 0xAARRGGBB.  The colour of its first corner if the source mesh had more than one.
struct SRasterTriangle
{
	int32	aiVertex[3];
	uint32	uiColour;
};


typedef CArrayTemplate<SRasterVertex>	CArrayRasterVertex;
typedef CArrayTemplate<SRasterTriangle>	CArrayRasterTriangle;


class CMesh;


//The integer copy of a CMesh that CRasteriser draws.  Build is the only place in RasterLib that reads the mesh's floats.
//
//The other Build keeps the mesh where it is at a scale that's given, for meshes that are to be drawn together at their
//true sizes (see SRasterInstance).
//
//What's exported from CAD can be cleaned up and made simpler for how big it will be drawn:
//  RemoveDegenerate takes out triangles with no area, which can't be seen.
//  FixReversed turns round triangles that wind against their own normals, which back face culling would leave as holes.
//  Simplify merges everything of a colour closer than a cell, for a mesh that is always drawn so small that a cell is about a pixel.
class CRasterMesh
{
public:
	CArrayRasterVertex		masVertices;
	CArrayRasterTriangle	masOpaque;
	CArrayRasterTriangle	masTransparent;
	SInt32Vec3				msMin;  //The bounds of its vertices.
	SInt32Vec3				msMax;

	void	Init(void);
	void	Kill(void);

	void	Build(CMesh* pcMesh);
	void	Build(CMesh* pcMesh, int32 iScale);

	size	RemoveDegenerate(void);
	size	FixReversed(void);
	void	Simplify(int32 iCellSize);

	size	NumTriangles(void);

protected:
	bool	FindBounds(CMesh* pcMesh, SFloat32Vec3* psMin, SFloat32Vec3* psMax);
	void	BuildFrom(CMesh* pcMesh, SFloat32Vec3* psCentre, int32 iScale);
	void	UpdateBounds(void);
	void	MergeCells(int32 iCellSize);
	void	RemoveRepeatedTriangles(CArrayRasterTriangle* pasTriangles);
	void	RemoveUnusedVertices(void);
};


#endif // __RASTER_MESH_H__

