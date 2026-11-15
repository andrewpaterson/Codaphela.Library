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
#include <math.h>
#include "IsosurfaceFit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIsosurfaceFit::Init(SFloat32Vec3* psReference)
{
	sReference = *psReference;
	sATA.Init();
	sATb.Init(0, 0, 0);
	fBTB = 0.0f;
	sMassPointSum.Init(0, 0, 0);
	fWeight = 0.0f;
	iNumPoints = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A row n of A, with b = n . p, both times the square root of fWeight, so
// the plane counts fWeight times in the sums, and the point fWeight times
// in the mass point.
//
//////////////////////////////////////////////////////////////////////////
void SIsosurfaceFit::Add(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fPointWeight)
{
	SFloat32Vec3	sRelative;
	SFloat32Vec3	sRow;
	float32			fB;
	float32			fRoot;

	fRoot = sqrtf(fPointWeight);
	sRelative = *psPosition - sReference;
	sRow = *psNormal * fRoot;
	fB = Float32Vec3Dot(&sRow, &sRelative);
	sATA.AddOuter(&sRow);
	sATb += sRow * fB;
	fBTB += fB * fB;
	sMassPointSum += sRelative * fPointWeight;
	fWeight += fPointWeight;
	iNumPoints++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIsosurfaceFit::GetMassPoint(SFloat32Vec3* psDest)
{
	if (iNumPoints == 0)
	{
		*psDest = sReference;
		return;
	}
	*psDest = sReference + (sMassPointSum / fWeight);
}


//////////////////////////////////////////////////////////////////////////
//
// x'A'Ax - 2 x . A'b + b'b, with x relative to the reference.
//
//////////////////////////////////////////////////////////////////////////
float32 SIsosurfaceFit::Error(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sRelative;
	SFloat32Vec3	sProduct;

	sRelative = *psPosition - sReference;
	sATA.Multiply(&sProduct, &sRelative);
	return Float32Vec3Dot(&sRelative, &sProduct) - (2.0f * Float32Vec3Dot(&sRelative, &sATb)) + fBTB;
}


//////////////////////////////////////////////////////////////////////////
//
// x = m + (A'A)+ (A'b - A'A m), m the mass point.  Clamped into the cell
// if it's out of it, and the mass point instead if that fits better.
//
//////////////////////////////////////////////////////////////////////////
void SIsosurfaceFit::Solve(SFloat32Vec3* psDest, SFloat32Bounds* psCell)
{
	SFloat32Vec3	sMassPoint;
	SFloat32Vec3	sRelativeMass;
	SFloat32Vec3	sRight;
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sClamped;

	GetMassPoint(&sMassPoint);
	if (iNumPoints == 0)
	{
		*psDest = sMassPoint;
		return;
	}

	sRelativeMass = sMassPoint - sReference;
	sATA.Multiply(&sRight, &sRelativeMass);
	sRight = sATb - sRight;
	sATA.PseudoInverseMultiply(&sOffset, &sRight, ISOSURFACE_FIT_CUT_OFF);
	*psDest = sMassPoint + sOffset;

	if ((psCell != NULL) && !psCell->IsEmpty())
	{
		sClamped.Init(fminf(fmaxf(psDest->x, psCell->sMinimum.x), psCell->sMaximum.x),
					  fminf(fmaxf(psDest->y, psCell->sMinimum.y), psCell->sMaximum.y),
					  fminf(fmaxf(psDest->z, psCell->sMinimum.z), psCell->sMaximum.z));
		if (sClamped != *psDest)
		{
			*psDest = (Error(&sClamped) <= Error(&sMassPoint)) ? sClamped : sMassPoint;
		}
	}
}

