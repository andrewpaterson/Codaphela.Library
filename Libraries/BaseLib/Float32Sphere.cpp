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
#include "Numbers.h"
#include "FloatHelper.h"
#include "GeometricTypes.h"
#include "Float32Triangle.h"
#include "Float32Sphere.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Sphere::Init(SFloat32Vec3* psCenter, float32 fRadius)
{
	sCenter = *psCenter;
	this->fRadius = fRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Sphere::Init(float32 x, float32 y, float32 z, float32 fRadius)
{
	sCenter.Init(x, y, z);
	this->fRadius = fRadius;
}


//////////////////////////////////////////////////////////////////////////
//
// No points make a sphere at the origin with no radius.
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Sphere::Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints)
{
	size			i;
	SFloat32Vec3	sOffset;
	float64			fLargest;
	float64			fSquare;

	sCenter.Zero();
	fRadius = 0.0f;
	if (uiNumPoints == 0)
	{
		return;
	}

	for (i = 0; i < uiNumPoints; i++)
	{
		sCenter += *GetPosition(psPoints, uiStride, i);
	}
	sCenter /= (float32)uiNumPoints;

	fLargest = 0.0;
	for (i = 0; i < uiNumPoints; i++)
	{
		Float32Vec3Subtract(&sOffset, GetPosition(psPoints, uiStride, i), &sCenter);
		fSquare = sOffset.SquareMagnitude();
		if (fSquare > fLargest)
		{
			fLargest = fSquare;
		}
	}
	fRadius = (float32)SquareRoot(fLargest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Sphere::Distance(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sOffset;

	Float32Vec3Subtract(&sOffset, psPosition, &sCenter);
	return sOffset.Magnitude() - fRadius;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Sphere::Contains(SFloat32Vec3* psPosition)
{
	return Distance(psPosition) < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Sphere::Contains(SFloat32Triangle* psTriangle)
{
	return Contains(&psTriangle->asPositions[0]) && Contains(&psTriangle->asPositions[1]) && Contains(&psTriangle->asPositions[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Sphere::Volume(void) const
{
	return ((4.0f / 3.0f) * N_PI) * fRadius * fRadius * fRadius;
}


//////////////////////////////////////////////////////////////////////////
//
// The volume of the lens where the two overlap, or the smaller one's if it's
// inside the other.
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Sphere::IntersectionVolume(SFloat32Sphere* psOther)
{
	float32			r1;
	float32			r2;
	float32			d;
	float32			v;
	SFloat32Vec3	sOffset;

	r1 = fRadius;
	r2 = psOther->fRadius;
	Float32Vec3Subtract(&sOffset, &sCenter, &psOther->sCenter);
	d = sOffset.Magnitude();
	if (d > r1 + r2)
	{
		return 0.0f;
	}

	if (d + minf(r2, r1) <= maxf(r2, r1))
	{
		if (r2 < r1)
		{
			return psOther->Volume();
		}
		else
		{
			return Volume();
		}
	}

	v = N_PI * Square(r1 + r2 - d) * (Square(d) + (2.0f * d * r2) - (3.0f * Square(r2)) + (2.0f * d * r1) + (6.0f * r2 * r1) - (3.0f * Square(r1)));
	return v / (12.0f * d);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Sphere::GetBounds(SFloat32Bounds* psDest)
{
	psDest->sMinimum.Init(sCenter.x - fRadius, sCenter.y - fRadius, sCenter.z - fRadius);
	psDest->sMaximum.Init(sCenter.x + fRadius, sCenter.y + fRadius, sCenter.z + fRadius);
}
