/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DUAL_CONTOURING_H__
#define __DUAL_CONTOURING_H__
#include "SupportLib/MeshEditor.h"
#include "IsosurfaceGrid.h"
#include "IsosurfaceFit.h"
#include "MarchingCubes.h"


//Which piece of a cell's surface each of its edges' crossings is on (SMarchingCubesCase::aiEdgePieces).
struct SDualContouringPieces
{
	int8	aiEdgePieces[MARCHING_CUBES_EDGES];
};


typedef CArrayLargeTemplate<SDualContouringPieces>	CArrayDualContouringPieces;


//Dual contouring (Ju, Losasso, Schaefer and Warren 2002) on a uniform grid (Isosurface.txt, step 8.5).  A cell the surface
//crosses has a vertex for each sheet of surface in it (manifold dual contouring, Schaefer, Ju and Warren 2007, step 8.8):
//marching cubes' pieces for its corners, its ambiguous faces decided by the asymptotic decider, joined where they aren't two
//sheets (IsosurfaceJoinPieces).  Each vertex is where the planes at its piece's crossings best meet (SIsosurfaceFit), so a crease
//or corner inside the cell is where it is, not cut off, and two sheets in one cell don't meet at one vertex.  Each crossed edge
//of the grid makes a quad of the vertices of its piece in the four cells around it, from its inside end to its outside end, split
//into two triangles along the diagonal whose middle is nearer the surface (the one that follows a crease).  Each vertex is one
//corner of the mesh, shared by every face on it.
//Design page: Docs/libraries/procedurallib/isosurface.md
class CDualContouring
{
public:
	CArrayLargeInt				maiCellCorners;  //Each cell's first corner in the mesh (one for each piece of surface in it), or -1.
	CArrayDualContouringPieces	masCellPieces;  //Each cell's edges' pieces.

	void	Init(void);
	void	Kill(void);

	void	Generate(CMeshEditor* pcMeshEditor, CIsosurfaceGrid* pcGrid, CShape* pcShape, int iFaceName = 0);

protected:
	lsize	CellIndex(CIsosurfaceGrid* pcGrid, int x, int y, int z);
	void	CellCase(SMarchingCubesCase* psDest, CIsosurfaceGrid* pcGrid, int x, int y, int z);
	void	FitCell(SFloat32Vec3* psDest, CIsosurfaceGrid* pcGrid, int x, int y, int z, SMarchingCubesCase* psCase, int iPiece);
};


//Two pieces of surface in a cell whose average normals are further apart than this (about 100 degrees) are two sheets, either
//side of something thinner than the cell, and keep a vertex each.
#define ISOSURFACE_SHEETS_COSINE	(-0.2f)


//Joins the pieces of a cell's surface (psCase's aiEdgePieces and iNumPieces) that aren't two sheets, by their crossings' normals
//(pasEdgeNormals, for each of the cell's edges that crosses).  Marching cubes' pieces are only what the corners show: a box's edge
//just outside a cell can leave two faces as two pieces, and one vertex on the edge between them is better than a vertex each.
void	IsosurfaceJoinPieces(SMarchingCubesCase* psCase, SFloat32Vec3* pasEdgeNormals);


//A quad of corners anticlockwise seen from outside (along psNormal, the surface's at its edge's crossing), as two triangles: the
//split whose triangles both face along psNormal if only one does, else the one whose diagonal's middle is nearer the surface
//(across a box's edge, two of the quad's corners are on the edge, and the diagonal between them follows it), else the one whose
//smaller triangle is larger.
void	IsosurfaceAddQuad(CMeshEditor* pcMeshEditor, CShape* pcShape, int* paiCorners, SFloat32Vec3* psNormal, int iFaceName);


#endif // __DUAL_CONTOURING_H__

