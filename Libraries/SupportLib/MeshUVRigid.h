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
#ifndef __MESH_UV_RIGID_H__
#define __MESH_UV_RIGID_H__
#include "BaseLib/Float32Vec2.h"
#include "BaseLib/Float32Vec3.h"
#include "MeshDefines.h"


//How many rounds FlattenRigid makes.
#define MESH_UV_RIGID_ROUNDS	10


//A chart laid flat as rigidly as possible (ARAP: Liu, Zhang, Xu, Gotsman and Gortler 2008; UnwrapPlan.txt, step 8): each
//triangle as near as it can be to a turned copy of itself at fTexelsPerUnit, so its area is kept as well as its angles and the
//texel density is even across the chart, where LSCM keeps only angles.  From pasUVs (LSCM's), each round finds every triangle's
//nearest turn of itself (the local step), then the UVs nearest all those turned triangles (the global step: a least squares
//problem whose matrix doesn't change, so it's factored once).  The arguments are FlattenConformal's.  Returns false, the UVs as
//they were, if the problem can't be factored (a UV only in triangles with no area).  The caller looks for folds.
bool	FlattenRigid(SFloat32Vec2* pasUVs, SFloat32Vec3* pasPositions, int iNumUVs, int* paiCorners, int iNumTriangles, float32 fTexelsPerUnit);


#endif // __MESH_UV_RIGID_H__

