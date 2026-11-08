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
#ifndef __MESH_MERGE_NORMALS_H__
#define __MESH_MERGE_NORMALS_H__
#include "MeshModifier.h"


//Makes the normals that faces use at one corner one normal, their average, where they are within fMaxAngle of the first of
//them.  An angle of none merges only those exactly the same.  CAD exports give each face its own normals, so a corner can
//have many the same, and each costs a vertex to draw: merging those halves a board's vertices, and a larger angle adds little
//to that.  Nothing moves, and normals further apart stay apart, so a sharp edge stays sharp.  A face with no normal of its
//own at a corner keeps its flat normal.
class CMeshMergeNormals : public CMeshModifier
{
CONSTRUCTABLE(CMeshMergeNormals);
protected:
	float32		mfMaxAngle;  //In radians.

public:
	void		Init(float32 fMaxAngle);

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;
};


#endif // __MESH_MERGE_NORMALS_H__
