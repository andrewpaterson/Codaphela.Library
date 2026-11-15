/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Sphere.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Init(SFloat32Vec3* psCenter, float32 fRadius)
{
	PreInit();
	CShape::Init();
	msSphere.Init(psCenter, fRadius);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Init(SFloat32Sphere* psSphere)
{
	Init(&psSphere->sCenter, psSphere->fRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Class(void)
{
	CShape::Class();
	U_3Float32(msSphere.sCenter);
	U_Float32(msSphere.fRadius);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CSphere::Distance(SFloat32Vec3* psPosition)
{
	return msSphere.Distance(psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	Float32Vec3Subtract(psDest, psPosition, &msSphere.sCenter);
	psDest->Normalize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSphere::GetBounds(SFloat32Bounds* psDest)
{
	msSphere.GetBounds(psDest);
}
