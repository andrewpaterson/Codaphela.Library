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
#include <stdio.h>
#include "Ray.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::Init(SFloat32Vec3* psStart, SFloat32Vec3* psDirection)
{
	msStart.Copy(psStart);
	msDirection.Copy(psDirection);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromPositions(SFloat32Vec3* s, SFloat32Vec3* e)
{
	msStart = (*s);
	Float3Subtract((SFloat32Vec3*)&msDirection, (SFloat32Vec3*)e, (SFloat32Vec3*)s);
	Float3Normalize((SFloat32Vec3*)&msDirection, (SFloat32Vec3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromDirection(SFloat32Vec3* sStartOfRay, SFloat32Vec3* sDirection)
{
	msStart = (*sStartOfRay);
	msDirection = (*sDirection);
	Float3Normalize((SFloat32Vec3*)&msDirection, (SFloat32Vec3*)&msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::SetFromDirection(float32 xs, float32 ys, float32 zs, float32 xd, float32 yd, float32 zd)
{
	msStart.Init(xs, ys, zs);
	msDirection.Init(xd, yd, zd);
	Float3Normalize(&msDirection, &msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRay::Copy(CRay* pcSource)
{
	msStart.Copy(&pcSource->msStart);
	msDirection.Copy(&pcSource->msDirection);
}

