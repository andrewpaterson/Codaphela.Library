/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
void CPlane::Init(SFloat3* psPosition, SFloat3* psNormal)
{
	CHalfSpace::Init(psNormal);
	mpsPosition = psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(SFloat3* v1, SFloat3* v2, SFloat3* v3)
{
	SFloat3 side1; 
	SFloat3 side2;

	CHalfSpace::Set(v1, v2, v3);
	*mpsPosition = *v1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(SFloat3* psPoint, SFloat3* psNormal)
{
	CHalfSpace::Set(psPoint, psNormal);
	*mpsPosition = *psPoint;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPlane::Set(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
{
	SFloat3 side1, side2;

	*mpsPosition = SFloat3(x1, y1, z1);
	side1 = SFloat3(x2, y2, z2) - *mpsPosition;
	side2 = SFloat3(x3, y3, z3) - *mpsPosition;
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

