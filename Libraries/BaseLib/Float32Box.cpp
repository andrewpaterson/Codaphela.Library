/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Float32Box.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Box::Init(SFloat32Vec3* psCenter, SFloat32Vec3* psAxis0, float32 fHalfLength0, SFloat32Vec3* psAxis1, float32 fHalfLength1, SFloat32Vec3* psAxis2, float32 fHalfLength2)
{
	sCenter = *psCenter;
	asAxes[0] = *psAxis0;
	asAxes[1] = *psAxis1;
	asAxes[2] = *psAxis2;
	afHalfLengths[0] = fHalfLength0;
	afHalfLengths[1] = fHalfLength1;
	afHalfLengths[2] = fHalfLength2;
}


//////////////////////////////////////////////////////////////////////////
//
// Of axes that are the same length, x before y before z.
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Box::Init(SFloat32Bounds* psBounds)
{
	SFloat32Vec3	sSize;
	SFloat32Vec3	sAxis;
	float32			fHalfLength;
	int				i;
	int				j;

	psBounds->GetCenter(&sCenter);
	psBounds->GetSize(&sSize);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	afHalfLengths[0] = sSize.x / 2.0f;
	afHalfLengths[1] = sSize.y / 2.0f;
	afHalfLengths[2] = sSize.z / 2.0f;

	for (i = 1; i < 3; i++)
	{
		for (j = i; (j > 0) && (afHalfLengths[j] > afHalfLengths[j - 1]); j--)
		{
			fHalfLength = afHalfLengths[j];
			afHalfLengths[j] = afHalfLengths[j - 1];
			afHalfLengths[j - 1] = fHalfLength;
			sAxis = asAxes[j];
			asAxes[j] = asAxes[j - 1];
			asAxes[j - 1] = sAxis;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Box::Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints)
{
	SFloat32Bounds	sBounds;

	sBounds.Init(psPoints, uiStride, uiNumPoints);
	Init(&sBounds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Box::Distance(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sLocal;
	SFloat32Vec3	sHalfLengths;

	Float32Vec3Subtract(&sOffset, psPosition, &sCenter);
	sLocal.Init(Float32Vec3Dot(&sOffset, &asAxes[0]), Float32Vec3Dot(&sOffset, &asAxes[1]), Float32Vec3Dot(&sOffset, &asAxes[2]));
	sHalfLengths.Init(afHalfLengths[0], afHalfLengths[1], afHalfLengths[2]);
	return Float32CenteredBoxDistance(&sLocal, &sHalfLengths);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Box::Contains(SFloat32Vec3* psPosition)
{
	return Distance(psPosition) < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Box::Volume(void)
{
	return 8.0f * afHalfLengths[0] * afHalfLengths[1] * afHalfLengths[2];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Box::GetCorner(SFloat32Vec3* psDest, int iCorner)
{
	int		i;
	float32	fAlong;

	*psDest = sCenter;
	for (i = 0; i < 3; i++)
	{
		fAlong = (iCorner & (1 << i)) ? afHalfLengths[i] : -afHalfLengths[i];
		*psDest += asAxes[i] * fAlong;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Box::GetBounds(SFloat32Bounds* psDest)
{
	SFloat32Vec3	sCorner;
	int				i;

	psDest->Init();
	for (i = 0; i < 8; i++)
	{
		GetCorner(&sCorner, i);
		psDest->Add(&sCorner);
	}
}
