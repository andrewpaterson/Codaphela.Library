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
#ifndef __MESH_SHAPE_EDITOR_H__
#define __MESH_SHAPE_EDITOR_H__
#include "SupportLib/MeshSourceModifier.h"
#include "SupportLib/Shape.h"


#define MESH_SHAPE_SEGMENTS			32			//Around a primitive's circles.
#define MESH_SHAPE_SPLIT_ANGLE		(0.5236f)	//30 degrees, between the normals in a cell of the octree.


//Meshes any CShape (Isosurface.txt, step 8.7; ShapePlan.txt, section 5), as a source modifier, so it sits on a CMeshEditor's
//stack and holds its shape, and a change to the shape and a Touch mesh it again.  A primitive with a mesh editor of its own
//(CShape::AddMesh) is that editor's mesh, exactly, with miSegments around its circles.  Anything else is an isosurface: dual
//contouring on an octree over its bounds, from cells of mfLargest down to mfSmallest, split where its normals differ by more
//than mfSplitAngle or a cell's centre is more than mfCentreTolerance from what its corners predict.  A shape with endless
//bounds (a half space) and no editor makes nothing.  It makes faces only; CMeshShapeNormals after it gives them normals from the
//shape, smooth across curves however coarse the faces and sharp at creases.
//Design page: Docs/libraries/procedurallib/isosurface.md
class CMeshShapeEditor : public CMeshSourceModifier
{
CONSTRUCTABLE(CMeshShapeEditor);
DESTRUCTABLE(CMeshShapeEditor);
public:
	Ptr<CShape>	mpShape;
	float32		mfSmallest;
	float32		mfLargest;
	float32		mfSplitAngle;  //Radians.
	float32		mfCentreTolerance;
	int			miSegments;
	int			miFaceName;  //The isosurface's faces'.  A primitive's editor names its own.

	void	Init(Ptr<CShape> pShape, float32 fSmallest, float32 fLargest);  //A tenth of fSmallest as the centre tolerance.
	void	Class(void) override;

	int		ExpectedFaceChunkSize(void) override;
	int		ExpectedCornerChunkSize(void) override;

	void	Generate(CMeshEditor* pcMeshEditor) override;
};


#endif // __MESH_SHAPE_EDITOR_H__

