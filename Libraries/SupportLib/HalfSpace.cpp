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
#include "HalfSpace.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Init(SFloat32Plane* psPlane)
{
	PreInit();
	CShape::Init();
	msPlane = *psPlane;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Class(void)
{
	CShape::Class();
	U_3Float32(msPlane.sNormal);
	U_Float32(msPlane.fDistance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Free(void)
{
	CShape::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CHalfSpace::Distance(SFloat32Vec3* psPosition)
{
	return msPlane.Distance(psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition)
{
	*psDest = msPlane.sNormal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHalfSpace::GetBounds(SFloat32Bounds* psDest)
{
	psDest->sMinimum.Init(-FLOAT32_BOUNDS_LARGEST, -FLOAT32_BOUNDS_LARGEST, -FLOAT32_BOUNDS_LARGEST);
	psDest->sMaximum.Init(FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST, FLOAT32_BOUNDS_LARGEST);
}
