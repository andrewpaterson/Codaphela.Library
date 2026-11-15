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
#include "Shape.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShape::Init(void)
{
	PreInit();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShape::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CShape::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Central differences, a step of SHAPE_GRADIENT_STEP of the bounds'
// longest side either way along each axis, or of 1 when the bounds have no
// size or are as large as a float32 goes.
//
//////////////////////////////////////////////////////////////////////////
void CShape::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	SFloat32Bounds	sBounds;
	SFloat32Vec3	sSize;
	SFloat32Vec3	sAhead;
	SFloat32Vec3	sBehind;
	float32			fLongest;
	float32			fStep;
	float32			afGradient[3];
	int				i;

	GetBounds(&sBounds);
	fLongest = 0.0f;
	if (!sBounds.IsEmpty() && (sBounds.sMaximum.x < FLOAT32_BOUNDS_LARGEST) && (sBounds.sMinimum.x > -FLOAT32_BOUNDS_LARGEST))
	{
		sBounds.GetSize(&sSize);
		fLongest = sSize.x;
		if (sSize.y > fLongest)
		{
			fLongest = sSize.y;
		}
		if (sSize.z > fLongest)
		{
			fLongest = sSize.z;
		}
	}
	if ((fLongest <= 0.0f) || (fLongest >= FLOAT32_BOUNDS_LARGEST))
	{
		fLongest = 1.0f;
	}
	fStep = fLongest * SHAPE_GRADIENT_STEP;

	for (i = 0; i < 3; i++)
	{
		sAhead = *psPosition;
		sBehind = *psPosition;
		if (i == 0)
		{
			sAhead.x += fStep;
			sBehind.x -= fStep;
		}
		else if (i == 1)
		{
			sAhead.y += fStep;
			sBehind.y -= fStep;
		}
		else
		{
			sAhead.z += fStep;
			sBehind.z -= fStep;
		}
		afGradient[i] = (Distance(&sAhead) - Distance(&sBehind)) / (2.0f * fStep);
	}
	psDest->Init(afGradient[0], afGradient[1], afGradient[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShape::IsExact(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShape::AddMesh(CMeshEditor* pcMeshEditor, int iSegments)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int ShapeMeshSegments(int iSegments)
{
	if (iSegments < 6)
	{
		return 6;
	}
	return iSegments;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CShape::Contains(SFloat32Vec3* psPosition)
{
	return Distance(psPosition) < 0.0f;
}
