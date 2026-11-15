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


//Dual contouring (Ju, Losasso, Schaefer and Warren 2002) on a uniform grid (Isosurface.txt, step 8.5).  A cell the surface
//crosses has one vertex, where the planes at its edges' crossings best meet (SIsosurfaceFit), so a crease or corner inside the
//cell is where it is, not cut off.  Each crossed edge of the grid makes a quad of the four cells' vertices around it, from its
//inside end to its outside end, split into two triangles along the diagonal whose middle is nearer the surface (the one that
//follows a crease).  Each cell's vertex is one corner of the mesh, shared by every face on it.
class CDualContouring
{
public:
	CArrayLargeInt	maiCellCorners;  //Each cell's corner in the mesh, or -1.

	void	Init(void);
	void	Kill(void);

	void	Generate(CMeshEditor* pcMeshEditor, CIsosurfaceGrid* pcGrid, CShape* pcShape, int iFaceName = 0);

protected:
	lsize	CellIndex(CIsosurfaceGrid* pcGrid, int x, int y, int z);
	bool	FitCell(SFloat32Vec3* psDest, CIsosurfaceGrid* pcGrid, int x, int y, int z);
};


//A quad of corners anticlockwise seen from outside (along psNormal, the surface's at its edge's crossing), as two triangles: the
//split whose triangles both face along psNormal if only one does, else the one whose diagonal's middle is nearer the surface
//(across a box's edge, two of the quad's corners are on the edge, and the diagonal between them follows it), else the one whose
//smaller triangle is larger.
void	IsosurfaceAddQuad(CMeshEditor* pcMeshEditor, CShape* pcShape, int* paiCorners, SFloat32Vec3* psNormal, int iFaceName);


#endif // __DUAL_CONTOURING_H__

