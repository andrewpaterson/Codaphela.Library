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
#include "SolidSlope.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidSlope::Init(void)
{
	SFloat32Vec3	sUp;

	sUp.Init(0, 0, 1);
	Init(&sUp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidSlope::Init(SFloat32Vec3* psDirection)
{
	PreInit();
	CSolidTexture::Init();
	msDirection = *psDirection;
	msDirection.Normalize();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidSlope::Class(void)
{
	CSolidTexture::Class();
	U_3Float32(msDirection);
}


//////////////////////////////////////////////////////////////////////////
//
// The direction is a unit, as Init makes it.
//
//////////////////////////////////////////////////////////////////////////
void CSolidSlope::Changed(void)
{
	CSolidTexture::Changed();
	msDirection.Normalize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidSlope::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	psSample->InitValue(0.5f + (0.5f * Float32Vec3Dot(psNormal, &msDirection)));
}

