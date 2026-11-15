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
#include "IsosurfaceOctree.h"
#include "MeshShapeEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShapeEditor::Init(Ptr<CShape> pShape, float32 fSmallest, float32 fLargest)
{
	PreInit();
	CMeshSourceModifier::Init();
	mpShape = pShape;
	mfSmallest = fSmallest;
	mfLargest = fLargest;
	mfSplitAngle = MESH_SHAPE_SPLIT_ANGLE;
	mfCentreTolerance = fSmallest * 0.1f;
	miSegments = MESH_SHAPE_SEGMENTS;
	miFaceName = 0;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshShapeEditor::Class(void)
{
	CMeshSourceModifier::Class();
	M_Pointer(mpShape);
	U_Float32(mfSmallest);
	U_Float32(mfLargest);
	U_Float32(mfSplitAngle);
	U_Float32(mfCentreTolerance);
	U_SInt(miSegments);
	U_SInt(miFaceName);
}


//////////////////////////////////////////////////////////////////////////
//
// Not known until it's meshed.
//
//////////////////////////////////////////////////////////////////////////
int CMeshShapeEditor::ExpectedFaceChunkSize(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMeshShapeEditor::ExpectedCornerChunkSize(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The isosurface's octree covers the shape's bounds and a smallest cell
// more, so the surface is inside it.
//
//////////////////////////////////////////////////////////////////////////
void CMeshShapeEditor::Generate(CMeshEditor* pcMeshEditor)
{
	CIsosurfaceOctree	cOctree;
	SFloat32Bounds		sBounds;
	SFloat32Vec3		sMargin;

	if (mpShape.IsNull())
	{
		return;
	}
	if (mpShape->AddMesh(pcMeshEditor, miSegments))
	{
		return;
	}

	mpShape->GetBounds(&sBounds);
	if (sBounds.IsEmpty() || (sBounds.sMaximum.x >= FLOAT32_BOUNDS_LARGEST) || (sBounds.sMinimum.x <= -FLOAT32_BOUNDS_LARGEST) ||
		(sBounds.sMaximum.y >= FLOAT32_BOUNDS_LARGEST) || (sBounds.sMinimum.y <= -FLOAT32_BOUNDS_LARGEST) ||
		(sBounds.sMaximum.z >= FLOAT32_BOUNDS_LARGEST) || (sBounds.sMinimum.z <= -FLOAT32_BOUNDS_LARGEST))
	{
		return;
	}
	sMargin.Init(mfSmallest, mfSmallest, mfSmallest);
	sBounds.sMinimum -= sMargin;
	sBounds.sMaximum += sMargin;

	cOctree.Init(&sBounds, mfSmallest, mfLargest, mfSplitAngle, mfCentreTolerance);
	cOctree.Build(&mpShape);
	cOctree.Generate(pcMeshEditor, &mpShape, miFaceName);
	cOctree.Kill();
}

