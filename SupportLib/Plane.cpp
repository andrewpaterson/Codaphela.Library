/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Plane.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Init(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal)
{
	CHalfSpace::Init(psNormal);
	mpsPosition = psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(SFloat32Vec3* v1, SFloat32Vec3* v2, SFloat32Vec3* v3)
{
	SFloat32Vec3 side1; 
	SFloat32Vec3 side2;

	CHalfSpace::Set(v1, v2, v3);
	*mpsPosition = *v1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(SFloat32Vec3* psPoint, SFloat32Vec3* psNormal)
{
	CHalfSpace::Set(psPoint, psNormal);
	*mpsPosition = *psPoint;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(float32 x1, float32 y1, float32 z1, float32 x2, float32 y2, float32 z2, float32 x3, float32 y3, float32 z3)
{
	SFloat32Vec3 side1, side2;

	(*mpsPosition).Init(x1, y1, z1);
	side1.Init(x2, y2, z2);
	side1.Subtract(mpsPosition);
	side2.Init(x3, y3, z3);
	side2.Subtract(mpsPosition);

	CHalfSpace::Set2(mpsPosition, &side1, &side2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(void)
{
	CHalfSpace::D(mpsPosition, mpsNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Copy(CPlane* psSource)
{
	mpsPosition->Copy(psSource->mpsPosition);
	CHalfSpace::Copy(psSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPlane::Coplanar(CPlane* psOther)
{
	if (Parallel(psOther))
	{
		if (On(psOther->mpsPosition))
		{
			return true;
		}
	}
	return false;
}

