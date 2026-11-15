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
#include <math.h>
#include "MeshBoxEditor.h"
#include "Box.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::Init(SFloat32Box* psBox)
{
	PreInit();
	CShape::Init();
	msBox = *psBox;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::Class(void)
{
	CShape::Class();
	U_3Float32(msBox.sCenter);
	U_3Float32Array(msBox.asAxes, 3);
	U_Float32Array(msBox.afHalfLengths, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CBox::Distance(SFloat32Vec3* psPosition)
{
	return msBox.Distance(psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// Along each axis, how far beyond its face the point is.  Beyond any, the
// gradient is how far beyond each, made a unit; beyond none, it's out
// through the face the point is nearest.  The point's side of the centre
// says which of each axis's two faces.
//
//////////////////////////////////////////////////////////////////////////
void CBox::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sOffset;
	float32			afAlong[3];
	float32			afBeyond[3];
	float32			afLocal[3];
	float32			fSign;
	int				i;
	int				iNearest;
	bool			bOutside;

	Float32Vec3Subtract(&sOffset, psPosition, &msBox.sCenter);
	bOutside = false;
	iNearest = 0;
	for (i = 0; i < 3; i++)
	{
		afAlong[i] = Float32Vec3Dot(&sOffset, &msBox.asAxes[i]);
		afBeyond[i] = fabsf(afAlong[i]) - msBox.afHalfLengths[i];
		bOutside |= afBeyond[i] > 0.0f;
		if (afBeyond[i] > afBeyond[iNearest])
		{
			iNearest = i;
		}
	}

	for (i = 0; i < 3; i++)
	{
		fSign = (afAlong[i] < 0.0f) ? -1.0f : 1.0f;
		if (bOutside)
		{
			afLocal[i] = (afBeyond[i] > 0.0f) ? afBeyond[i] * fSign : 0.0f;
		}
		else
		{
			afLocal[i] = (i == iNearest) ? fSign : 0.0f;
		}
	}

	*psDest = msBox.asAxes[0] * afLocal[0] + msBox.asAxes[1] * afLocal[1] + msBox.asAxes[2] * afLocal[2];
	psDest->Normalize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBox::GetBounds(SFloat32Bounds* psDest)
{
	msBox.GetBounds(psDest);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBox::AddMesh(CMeshEditor* pcMeshEditor, int iSegments)
{
	CMeshBoxEditor	cEditor;
	SFloat32Vec3	asSides[3];
	SFloat32Vec3	sStart;
	SFloat32Vec3	sCross;
	int				i;

	sStart = msBox.sCenter;
	for (i = 0; i < 3; i++)
	{
		asSides[i] = msBox.asAxes[i] * (2.0f * msBox.afHalfLengths[i]);
		sStart -= msBox.asAxes[i] * msBox.afHalfLengths[i];
	}

	//The editor's sides go right handed; a box whose axes don't starts from the far end of its last.
	Float32Vec3Cross(&sCross, &asSides[0], &asSides[1]);
	if (Float32Vec3Dot(&sCross, &asSides[2]) < 0.0f)
	{
		sStart += asSides[2];
		asSides[2] = -asSides[2];
	}

	cEditor.Init(&asSides[0], &asSides[1], &asSides[2], &sStart, 1, 1, 1);
	cEditor.Generate(pcMeshEditor);
	cEditor.Kill();
	return true;
}

