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
#include "BaseLib/Noise.h"
#include "SolidCells.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCells::Init(ESolidCells eKind, uint32 uiSeed)
{
	PreInit();
	CSolidTexture::Init();
	meKind = eKind;
	muiSeed = uiSeed;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCells::Class(void)
{
	CSolidTexture::Class();
	U_Enum(meKind);
	U_UInt32(muiSeed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCells::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SCellularNoise	sNoise;

	CellularNoise(&sNoise, psPosition, muiSeed, fWidth);
	if (meKind == SC_Nearest)
	{
		psSample->InitValue(sNoise.fNearest);
	}
	else if (meKind == SC_Edge)
	{
		psSample->InitValue(sNoise.fSecond - sNoise.fNearest);
	}
	else
	{
		psSample->InitValue(sNoise.fCell);
	}
}

