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
#ifndef __MARCHING_CUBES_H__
#define __MARCHING_CUBES_H__
#include "SupportLib/MeshEditor.h"
#include "IsosurfaceGrid.h"


//A cube's corners are numbered by their x, y and z as bits 0, 1 and 2, and its edges by axis: edge (iAxis * 4) + k runs along
//iAxis from the corner whose other two axes, (iAxis + 1) % 3 and (iAxis + 2) % 3, are bits 0 and 1 of k.
#define MARCHING_CUBES_CORNERS		8
#define MARCHING_CUBES_EDGES		12
#define MARCHING_CUBES_CASES		256
#define MARCHING_CUBES_MAX_FACES	12


//Which of a cube's edges each case's triangles have their corners on, anticlockwise seen from outside.  A case is a cube's
//inside corners as bits.
struct SMarchingCubesCase
{
	int		iNumFaces;
	int8	aaiFaces[MARCHING_CUBES_MAX_FACES][3];
};


//Marching cubes (Lorensen and Cline 1987), the isosurface's baseline (Isosurface.txt, step 8.3).  Each crossing of a
//CIsosurfaceGrid is one corner of the mesh, shared by every triangle on it.  The cases are made in Init rather than typed in
//from Bourke's tables: each face of the cube is walked anticlockwise seen from outside, and where the walk goes into the
//inside a line runs to where it next comes out.  The lines join into loops, which are fanned into triangles.  As a face's lines
//depend on its four corners alone, the two cubes either side of it agree and nothing cracks, and a face with two inside corners
//across it from each other keeps them apart, whichever cube it's in.
class CMarchingCubes
{
public:
	SMarchingCubesCase	masCases[MARCHING_CUBES_CASES];

	void		Init(void);
	void		Kill(void);

	void		Generate(CMeshEditor* pcMeshEditor, CIsosurfaceGrid* pcGrid, int iFaceName = 0);

	static int	EdgeStartCorner(int iEdge);
	static int	EdgeEndCorner(int iEdge);
	static int	CornersEdge(int iCorner1, int iCorner2);

protected:
	void		MakeCase(SMarchingCubesCase* psCase, int iInside);
};


#endif // __MARCHING_CUBES_H__

