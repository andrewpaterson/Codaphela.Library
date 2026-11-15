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
#include "MeshCylinderEditor.h"
#include "Cylinder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCylinder::Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fRadius, float32 fLength)
{
	PreInit();
	CRevolvedShape::Init(psAxis, psStart, fLength);
	mfRadius = fRadius;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCylinder::Class(void)
{
	CRevolvedShape::Class();
	U_Float32(mfRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCylinder::Free(void)
{
	CRevolvedShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CCylinder::ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient)
{
	return BoxProfileDistance(fRadial, fAlong - mfLength / 2.0f, mfRadius, mfLength / 2.0f, psGradient);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCylinder::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init();
	AddDisc(psDest, 0.0f, mfRadius);
	AddDisc(psDest, mfLength, mfRadius);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCylinder::AddMesh(CMeshEditor* pcMeshEditor, int iSegments)
{
	CMeshCylinderEditor	cEditor;

	cEditor.Init(&msAxis, &msStart, mfRadius, mfLength, ShapeMeshSegments(iSegments), 1, 1);
	cEditor.Generate(pcMeshEditor);
	cEditor.Kill();
	return true;
}

