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
#include "SolidNoise.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidNoise::Init(EFractalNoise eKind, size uiOctaves, uint32 uiSeed)
{
	PreInit();
	CSolidTexture::Init();
	meKind = eKind;
	muiOctaves = uiOctaves;
	muiSeed = uiSeed;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidNoise::Class(void)
{
	CSolidTexture::Class();
	U_Enum(meKind);
	U_Size(muiOctaves);
	U_UInt32(muiSeed);
}


//////////////////////////////////////////////////////////////////////////
//
// A sum is -1 to 1 about 0; the others are 0 to 1 already.
//
//////////////////////////////////////////////////////////////////////////
void CSolidNoise::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	float32	fNoise;

	fNoise = FractalNoise(meKind, psPosition, muiSeed, muiOctaves, fWidth);
	psSample->InitValue((meKind == FN_Sum) ? (0.5f + (0.5f * fNoise)) : fNoise);
}

