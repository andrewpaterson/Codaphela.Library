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
#include "MeshTubeEditor.h"
#include "Tube.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTube::Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fInnerRadius, float32 fOuterRadius, float32 fLength)
{
	PreInit();
	CRevolvedShape::Init(psAxis, psStart, fLength);
	mfInnerRadius = fInnerRadius;
	mfOuterRadius = fOuterRadius;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTube::Class(void)
{
	CRevolvedShape::Class();
	U_Float32(mfInnerRadius);
	U_Float32(mfOuterRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTube::Free(void)
{
	CRevolvedShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CTube::ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient)
{
	return BoxProfileDistance(fRadial - (mfInnerRadius + mfOuterRadius) / 2.0f, fAlong - mfLength / 2.0f, (mfOuterRadius - mfInnerRadius) / 2.0f, mfLength / 2.0f, psGradient);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTube::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init();
	AddDisc(psDest, 0.0f, mfOuterRadius);
	AddDisc(psDest, mfLength, mfOuterRadius);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTube::AddMesh(CMeshEditor* pcMeshEditor, int iSegments)
{
	CMeshTubeEditor	cEditor;

	cEditor.Init(&msAxis, &msStart, mfInnerRadius, mfOuterRadius, mfLength, ShapeMeshSegments(iSegments), 1, 2);  //Its rings count its ends.
	cEditor.Generate(pcMeshEditor);
	cEditor.Kill();
	return true;
}

