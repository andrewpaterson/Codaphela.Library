/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __MESH_UV_CONFORMAL_H__
#define __MESH_UV_CONFORMAL_H__
#include "BaseLib/Float32Vec2.h"
#include "BaseLib/Float32Vec3.h"
#include "MeshDefines.h"


//How near Float64SparseLeastSquares gets LSCM's answer: its residual against x = 0's.
#define MESH_UV_CONFORMAL_TOLERANCE		(1e-7)

//The most iterations it makes, for each unknown (two for each UV not pinned).
#define MESH_UV_CONFORMAL_ITERATIONS	4


//A chart laid flat by Least Squares Conformal Maps (Levy, Petitjean, Ray and Maillot 2002; UnwrapPlan.txt, Flatten): the UVs
//for which each triangle is as near as it can be to a turned and scaled copy of itself, so angles are kept.  The two UVs
//furthest apart stay where they are (pinned), as LSCM needs two.  Then the chart is scaled about 0, 0 so its area is its area
//in 3D at fTexelsPerUnit.
//
//pasUVs is the chart laid flat some other way (across its normal), which gives the pins and the solver's start, and gets the
//answer.  It has iNumUVs, pasPositions each one's position; iNumTriangles triangles have their corners three at a time in
//paiCorners, indices into both.  Returns false, the UVs as they were, if the solver doesn't get there or the chart comes out
//turned over.  It can come out with triangles turned over or overlapping where the chart isn't a disc or is very curved: the
//caller looks.
bool	FlattenConformal(SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int iNumUVs, int* paiCorners, int iNumTriangles, float32 fTexelsPerUnit);


#endif // __MESH_UV_CONFORMAL_H__

