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
#include <math.h>
#include "FloatHelper.h"
#include "GeometricTypes.h"
#include "Float32Bounds.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::Init(void)
{
	sMinimum.Init(FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST);
	sMaximum.Init(-FLOAT32_BOUNDS_LARGEST, -FLOAT32_BOUNDS_LARGEST, -FLOAT32_BOUNDS_LARGEST);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::Init(SFloat32Vec3* psMinimum, SFloat32Vec3* psMaximum)
{
	sMinimum = *psMinimum;
	sMaximum = *psMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints)
{
	size	i;

	Init();
	for (i = 0; i < uiNumPoints; i++)
	{
		Add(GetPosition(psPoints, uiStride, i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Bounds::IsEmpty(void) const
{
	return (sMinimum.x > sMaximum.x) || (sMinimum.y > sMaximum.y) || (sMinimum.z > sMaximum.z);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::Add(SFloat32Vec3* psPosition)
{
	Float32Vec3Minimize(&sMinimum, &sMinimum, psPosition);
	Float32Vec3Maximize(&sMaximum, &sMaximum, psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// An empty one adds nothing.
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::Add(SFloat32Bounds* psBounds)
{
	if (!psBounds->IsEmpty())
	{
		Add(&psBounds->sMinimum);
		Add(&psBounds->sMaximum);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::GetCenter(SFloat32Vec3* psDest)
{
	psDest->Init((sMinimum.x + sMaximum.x) / 2.0f, (sMinimum.y + sMaximum.y) / 2.0f, (sMinimum.z + sMaximum.z) / 2.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Bounds::GetSize(SFloat32Vec3* psDest)
{
	psDest->Init(sMaximum.x - sMinimum.x, sMaximum.y - sMinimum.y, sMaximum.z - sMinimum.z);
}


//////////////////////////////////////////////////////////////////////////
//
// Exact.  An empty one is as far as can be.
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Bounds::Distance(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sCenter;
	SFloat32Vec3	sHalfLengths;
	SFloat32Vec3	sOffset;

	if (IsEmpty())
	{
		return FLOAT32_BOUNDS_LARGEST;
	}

	GetCenter(&sCenter);
	GetSize(&sHalfLengths);
	sHalfLengths /= 2.0f;
	Float32Vec3Subtract(&sOffset, psPosition, &sCenter);
	return Float32CenteredBoxDistance(&sOffset, &sHalfLengths);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Bounds::Contains(SFloat32Vec3* psPosition)
{
	return Distance(psPosition) < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// Outside, the length of how far the point is beyond each side.  Inside,
// how far it is from the nearest side, negative.
//
//////////////////////////////////////////////////////////////////////////
float32 Float32CenteredBoxDistance(SFloat32Vec3* psOffset, SFloat32Vec3* psHalfLengths)
{
	SFloat32Vec3	sBeyond;
	SFloat32Vec3	sOutside;
	float32			fLargest;

	sBeyond.Init(fabsf(psOffset->x) - psHalfLengths->x, fabsf(psOffset->y) - psHalfLengths->y, fabsf(psOffset->z) - psHalfLengths->z);
	sOutside.Init(maxf(sBeyond.x, 0.0f), maxf(sBeyond.y, 0.0f), maxf(sBeyond.z, 0.0f));
	fLargest = maxf(sBeyond.x, maxf(sBeyond.y, sBeyond.z));
	return sOutside.Magnitude() + minf(fLargest, 0.0f);
}
