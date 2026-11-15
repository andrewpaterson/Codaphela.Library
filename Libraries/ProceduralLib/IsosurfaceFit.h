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
#ifndef __ISOSURFACE_FIT_H__
#define __ISOSURFACE_FIT_H__
#include "BaseLib/Float32Bounds.h"
#include "BaseLib/Float32Symmetric3x3.h"


//The fraction of A'A's largest eigenvalue under which a direction counts as unconstrained.  Ju et al. (2002) drop A's singular
//values under a tenth of the largest, and A'A's eigenvalues are their squares, so it's a hundredth.  A tenth here drops a face
//crossed at one or two of a cell's edges when another is crossed at many, and the vertex slides off past their edge.
#define ISOSURFACE_FIT_CUT_OFF		(0.01f)


//What a cell's vertex is fitted to (Isosurface.txt, Placing a cell's vertex): the planes through its crossings along their
//normals, the point x minimising the sum of (n . (x - p))^2.  Kept as A'A, A'b and b'b, with positions relative to sReference
//(the cell's centre), so a far away cell loses nothing to numbers cancelling.  Solve works from the crossings' mass point, so
//along a direction no plane fixes (along an edge, across a flat face, anywhere on a sphere's cell) the vertex stays there.
class SIsosurfaceFit
{
public:
	SFloat32Vec3			sReference;
	SFloat32Symmetric3x3	sATA;
	SFloat32Vec3			sATb;
	float32					fBTB;
	SFloat32Vec3			sMassPointSum;  //Relative to sReference.
	int						iNumPoints;

	void	Init(SFloat32Vec3* psReference);
	void	Add(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal);  //psNormal a unit.

	void	GetMassPoint(SFloat32Vec3* psDest);
	float32	Error(SFloat32Vec3* psPosition);
	void	Solve(SFloat32Vec3* psDest, SFloat32Bounds* psCell);  //Clamped into psCell, or the mass point where that fits worse.
};


#endif // __ISOSURFACE_FIT_H__

