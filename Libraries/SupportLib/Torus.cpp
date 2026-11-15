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

** ------------------------------------------------------------------------ **/
#include "MeshTorusEditor.h"
#include "Torus.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTorus::Init(SFloat32Vec3* psAxis, SFloat32Vec3* psCenter, float32 fRadius, float32 fTubeRadius)
{
	PreInit();
	CRevolvedShape::Init(psAxis, psCenter, 0.0f);
	mfRadius = fRadius;
	mfTubeRadius = fTubeRadius;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTorus::Class(void)
{
	CRevolvedShape::Class();
	U_Float32(mfRadius);
	U_Float32(mfTubeRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTorus::Free(void)
{
	CRevolvedShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// From the circle mfRadius out that's the middle of its tube.
//
//////////////////////////////////////////////////////////////////////////
float32 CTorus::ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient)
{
	return PointProfileDistance(fRadial - mfRadius, fAlong, psGradient) - mfTubeRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTorus::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init();
	AddDisc(psDest, -mfTubeRadius, mfRadius + mfTubeRadius);
	AddDisc(psDest, mfTubeRadius, mfRadius + mfTubeRadius);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTorus::AddMesh(CMeshEditor* pcMeshEditor, int iSegments)
{
	CMeshTorusEditor	cEditor;

	cEditor.Init(&msAxis, &msStart, mfRadius, mfTubeRadius, ShapeMeshSegments(iSegments), ShapeMeshSegments(iSegments) / 2);
	cEditor.Generate(pcMeshEditor);
	cEditor.Kill();
	return true;
}

