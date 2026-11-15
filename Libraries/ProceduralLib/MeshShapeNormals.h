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
#ifndef __MESH_SHAPE_NORMALS_H__
#define __MESH_SHAPE_NORMALS_H__
#include "SupportLib/MeshModifier.h"
#include "SupportLib/Shape.h"


#define MESH_SHAPE_NORMALS_CREASE		(0.7853982f)	//45 degrees: a face further than this from the shape's gradient is across a crease.
#define MESH_SHAPE_NORMALS_INSIDE		(0.1f)			//How far toward a face's middle a face corner's own side's gradient is taken.


//Normals for a mesh of a shape (CMeshShapeEditor's) from the shape itself rather than from its faces, so a curve is smooth however
//coarse its faces.  At a corner whose faces' sides of it (the gradient a tenth of the way in from the corner toward each face's
//middle) are within mfCreaseAngle of each other, every face shares the shape's gradient at the corner.  At a crease (a box's edge
//or corner, a cylinder's rim, where the gradient at the corner is only one side's, or a blend) each face corner has its own
//side's, so creases stay sharp and a cylinder's side is round right up to its rim.  A face corner whose normal would be further
//than mfCreaseAngle from its face keeps its flat normal.  Faces are kept as they are; a mesh
//with UVs, skin or materials is left as it was (CanRebuild).  CMeshMergeNormals at an angle split a corner wherever coarse faces
//around it fanned through more than the angle.
class CMeshShapeNormals : public CMeshModifier
{
CONSTRUCTABLE(CMeshShapeNormals);
DESTRUCTABLE(CMeshShapeNormals);
public:
	Ptr<CShape>	mpShape;
	float32		mfCreaseAngle;  //Radians.

	void		Init(Ptr<CShape> pShape, float32 fCreaseAngle = MESH_SHAPE_NORMALS_CREASE);
	void		Class(void) override;

	Ptr<CMesh>	Modify(Ptr<CMesh> pMesh) override;

};


#endif // __MESH_SHAPE_NORMALS_H__

