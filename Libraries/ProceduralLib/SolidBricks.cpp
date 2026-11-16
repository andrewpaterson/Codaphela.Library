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
#include <math.h>
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Noise.h"
#include "SolidBricks.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidBricks::Init(ESolidBricks eKind, SFloat32Vec3* psSize, float32 fMortar, float32 fOffset, uint32 uiSeed)
{
	PreInit();
	CSolidTexture::Init();
	meKind = eKind;
	msSize = *psSize;
	mfMortar = fMortar;
	mfOffset = fOffset;
	muiSeed = uiSeed;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidBricks::Class(void)
{
	CSolidTexture::Class();
	U_Enum(meKind);
	U_3Float32(msSize);
	U_Float32(mfMortar);
	U_Float32(mfOffset);
	U_UInt32(muiSeed);
}


//////////////////////////////////////////////////////////////////////////
//
// The point in bricks: which layer (y) and row (z) it's in, then along x
// from where that layer and row start.  Its whole part is the brick, and
// its part, times the brick's size, how far the point is from the brick's
// lower faces; from the nearer of each pair of faces, less half the
// mortar, the least is how far inside it is.
//
//////////////////////////////////////////////////////////////////////////
void CSolidBricks::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SFloat32Vec3	sBricks;
	SFloat32Vec3	sBrick;
	SFloat32Vec3	sPart;
	float32			fInside;

	sBricks.y = psPosition->y / msSize.y;
	sBricks.z = psPosition->z / msSize.z;
	sBrick.y = floorf(sBricks.y);
	sBrick.z = floorf(sBricks.z);
	sBricks.x = (psPosition->x / msSize.x) - (mfOffset * (sBrick.y + sBrick.z));
	sBrick.x = floorf(sBricks.x);

	if (meKind == SB_Brick)
	{
		psSample->InitValue(NoiseHashUnit(NoiseHash((int32)sBrick.x, (int32)sBrick.y, (int32)sBrick.z, muiSeed)));
		return;
	}

	sPart = sBricks - sBrick;
	fInside = minf(minf(sPart.x, 1.0f - sPart.x) * msSize.x, minf(minf(sPart.y, 1.0f - sPart.y) * msSize.y, minf(sPart.z, 1.0f - sPart.z) * msSize.z)) - (mfMortar * 0.5f);
	psSample->InitValue(maxf(fInside, 0.0f));
}

