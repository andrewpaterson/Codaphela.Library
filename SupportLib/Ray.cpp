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
#include <stdio.h>
#include "Ray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromPositions(SFloat3* s, SFloat3* e)
{
	msStart = (*s);
	Float3Subtract((SFloat3*)&msDirection, (SFloat3*)e, (SFloat3*)s);
	Float3Normalize((SFloat3*)&msDirection, (SFloat3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromDirection(SFloat3* sStartOfRay, SFloat3* sDirection)
{
	msStart = (*sStartOfRay);
	msDirection = (*sDirection);
	Float3Normalize((SFloat3*)&msDirection, (SFloat3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromDirection(float xs, float ys, float zs, float xd, float yd, float zd)
{
	(SFloat3)msStart = SFloat3(xs, ys, zs);
	(SFloat3)msDirection = SFloat3(xd, yd, zd);
	Float3Normalize((SFloat3*)&msDirection, (SFloat3*)&msDirection);
}

