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
#include "Float32Plane.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Plane::Init(SFloat32Vec3* psUnitNormal, float32 fDistance)
{
	sNormal = *psUnitNormal;
	this->fDistance = fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Plane::Init(SFloat32Vec3* psNormal, SFloat32Vec3* psPoint)
{
	Float32Vec3Normalise(&sNormal, psNormal);
	fDistance = Float32Vec3Dot(&sNormal, psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Plane::Init(SFloat32Vec3* ps1, SFloat32Vec3* ps2, SFloat32Vec3* ps3)
{
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sCross;

	Float32Vec3Subtract(&sSide1, ps2, ps1);
	Float32Vec3Subtract(&sSide2, ps3, ps1);
	Float32Vec3Cross(&sCross, &sSide1, &sSide2);
	if (sCross.SquareMagnitude() == 0.0)
	{
		sNormal.Zero();
		fDistance = 0.0f;
		return false;
	}
	Init(&sCross, ps1);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SFloat32Plane::Distance(SFloat32Vec3* psPosition)
{
	return Float32Vec3Dot(&sNormal, psPosition) - fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFloat32Plane::Contains(SFloat32Vec3* psPosition)
{
	return Distance(psPosition) < 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Plane::ClosestPoint(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	float32		fAlong;

	fAlong = Distance(psPosition);
	psDest->Init(psPosition->x - sNormal.x * fAlong, psPosition->y - sNormal.y * fAlong, psPosition->z - sNormal.z * fAlong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFloat32Plane::Flip(void)
{
	sNormal.Init(-sNormal.x, -sNormal.y, -sNormal.z);
	fDistance = -fDistance;
}
