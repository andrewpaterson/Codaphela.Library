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
#include "Float32Triangle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Triangle::Init(SFloat32Vec3* ps1, SFloat32Vec3* ps2, SFloat32Vec3* ps3)
{
	asPositions[0] = *ps1;
	asPositions[1] = *ps2;
	asPositions[2] = *ps3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Triangle::GetNormal(SFloat32Vec3* psDest)
{
	SFloat32Plane	sPlane;

	if (GetPlane(&sPlane))
	{
		*psDest = sPlane.sNormal;
		return true;
	}
	psDest->Zero();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Triangle::GetPlane(SFloat32Plane* psDest)
{
	return psDest->Init(&asPositions[0], &asPositions[1], &asPositions[2]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Triangle::Area(void)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sCross;

	Float32Vec3Subtract(&sSide1, &asPositions[1], &asPositions[0]);
	Float32Vec3Subtract(&sSide2, &asPositions[2], &asPositions[0]);
	Float32Vec3Cross(&sCross, &sSide1, &sSide2);
	return sCross.Magnitude() / 2.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Triangle::GetCenter(SFloat32Vec3* psDest)
{
	*psDest = asPositions[0];
	*psDest += asPositions[1];
	*psDest += asPositions[2];
	*psDest /= 3.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Triangle::GetBounds(SFloat32Bounds* psDest)
{
	psDest->Init(asPositions, sizeof(SFloat32Vec3), 3);
}


//////////////////////////////////////////////////////////////////////////
//
// The point of the triangle nearest psPosition: a corner, a point on an
// edge or a point inside it, found by which side of each edge and corner
// psPosition is (Ericson, Real-Time Collision Detection, 5.1.5).
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Triangle::ClosestPoint(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sAB;
	SFloat32Vec3	sAC;
	SFloat32Vec3	sAP;
	SFloat32Vec3	sBP;
	SFloat32Vec3	sCP;
	float32			d1;
	float32			d2;
	float32			d3;
	float32			d4;
	float32			d5;
	float32			d6;
	float32			fVA;
	float32			fVB;
	float32			fVC;
	float32			fV;
	float32			fW;
	float32			fSum;

	Float32Vec3Subtract(&sAB, &asPositions[1], &asPositions[0]);
	Float32Vec3Subtract(&sAC, &asPositions[2], &asPositions[0]);

	Float32Vec3Subtract(&sAP, psPosition, &asPositions[0]);
	d1 = Float32Vec3Dot(&sAB, &sAP);
	d2 = Float32Vec3Dot(&sAC, &sAP);
	if ((d1 <= 0.0f) && (d2 <= 0.0f))
	{
		*psDest = asPositions[0];
		return;
	}

	Float32Vec3Subtract(&sBP, psPosition, &asPositions[1]);
	d3 = Float32Vec3Dot(&sAB, &sBP);
	d4 = Float32Vec3Dot(&sAC, &sBP);
	if ((d3 >= 0.0f) && (d4 <= d3))
	{
		*psDest = asPositions[1];
		return;
	}

	fVC = d1 * d4 - d3 * d2;
	if ((fVC <= 0.0f) && (d1 >= 0.0f) && (d3 <= 0.0f))
	{
		fV = d1 / (d1 - d3);
		*psDest = asPositions[0] + sAB * fV;
		return;
	}

	Float32Vec3Subtract(&sCP, psPosition, &asPositions[2]);
	d5 = Float32Vec3Dot(&sAB, &sCP);
	d6 = Float32Vec3Dot(&sAC, &sCP);
	if ((d6 >= 0.0f) && (d5 <= d6))
	{
		*psDest = asPositions[2];
		return;
	}

	fVB = d5 * d2 - d1 * d6;
	if ((fVB <= 0.0f) && (d2 >= 0.0f) && (d6 <= 0.0f))
	{
		fW = d2 / (d2 - d6);
		*psDest = asPositions[0] + sAC * fW;
		return;
	}

	fVA = d3 * d6 - d5 * d4;
	if ((fVA <= 0.0f) && ((d4 - d3) >= 0.0f) && ((d5 - d6) >= 0.0f))
	{
		fW = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		*psDest = asPositions[1] + (asPositions[2] - asPositions[1]) * fW;
		return;
	}

	fSum = fVA + fVB + fVC;
	if (fSum == 0.0f)
	{
		//No area and no edge region took it: its corners are all in one place.
		*psDest = asPositions[0];
		return;
	}
	fV = fVB / fSum;
	fW = fVC / fSum;
	*psDest = asPositions[0] + sAB * fV + sAC * fW;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Triangle::Distance(SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sClosest;
	SFloat32Vec3	sOffset;

	ClosestPoint(&sClosest, psPosition);
	Float32Vec3Subtract(&sOffset, psPosition, &sClosest);
	return sOffset.Magnitude();
}
