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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "SolidChecker.h"


//The narrowest width a checker is averaged over, as a fraction of a cube.  Below it a point's colour is as good, and averaging
//over less loses float32's precision (the difference of two nearly equal numbers over the width).
#define SOLID_CHECKER_LEAST_WIDTH	(1e-3f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidChecker::Init(float32 fSize, ARGB32 uiColour1, ARGB32 uiColour2)
{
	PreInit();
	CSolidTexture::Init();
	mfSize = fSize;
	muiColour1 = uiColour1;
	muiColour2 = uiColour2;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidChecker::Class(void)
{
	CSolidTexture::Class();
	U_Float32(mfSize);
	U_UInt32(muiColour1);
	U_UInt32(muiColour2);
}


//////////////////////////////////////////////////////////////////////////
//
// The square wave +1 on [0, 1) and -1 on [1, 2), repeating, averaged over
// [x - w / 2, x + w / 2]: the difference of its integral (a triangle
// wave) at the two ends, over w.
//
//////////////////////////////////////////////////////////////////////////
static float32 SolidCheckerWave(float32 x, float32 w)
{
	float32	fA;
	float32	fB;

	if (w < SOLID_CHECKER_LEAST_WIDTH)
	{
		fA = x * 0.5f;
		return ((fA - floorf(fA)) < 0.5f) ? 1.0f : -1.0f;
	}
	fA = (x - (w * 0.5f)) * 0.5f;
	fB = (x + (w * 0.5f)) * 0.5f;
	return 2.0f * (fabsf((fA - floorf(fA)) - 0.5f) - fabsf((fB - floorf(fB)) - 0.5f)) / w;
}


//////////////////////////////////////////////////////////////////////////
//
// In cubes: the product of the axes' waves is +1 in a cube of the first
// colour and -1 in one of the second.
//
//////////////////////////////////////////////////////////////////////////
float32 CSolidChecker::Mix(SFloat32Vec3* psPosition, float32 fWidth)
{
	float32	fCubes;
	float32	fProduct;

	fCubes = fWidth / mfSize;
	fProduct = SolidCheckerWave(psPosition->x / mfSize, fCubes) * SolidCheckerWave(psPosition->y / mfSize, fCubes) * SolidCheckerWave(psPosition->z / mfSize, fCubes);
	fProduct = (fProduct > 1.0f) ? 1.0f : ((fProduct < -1.0f) ? -1.0f : fProduct);
	return 0.5f - (0.5f * fProduct);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidChecker::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SSolidSample	sSecond;

	psSample->Init(muiColour1);
	sSecond.Init(muiColour2);
	psSample->Blend(&sSecond, Mix(psPosition, fWidth));
}

