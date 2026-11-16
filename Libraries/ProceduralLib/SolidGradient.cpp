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
#include "SolidGradient.h"


//The narrowest width a gradient is averaged over.  Below it a point's value is as good, and averaging over less loses
//float32's precision (the difference of two nearly equal numbers over the width).
#define SOLID_GRADIENT_LEAST_WIDTH	(1e-3f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidGradient::Init(ESolidGradient eFrom, ESolidRepeat eRepeat)
{
	PreInit();
	CSolidTexture::Init();
	meFrom = eFrom;
	meRepeat = eRepeat;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidGradient::Class(void)
{
	CSolidTexture::Class();
	U_Enum(meFrom);
	U_Enum(meRepeat);
}


//////////////////////////////////////////////////////////////////////////
//
// The repeat's value at a distance.
//
//////////////////////////////////////////////////////////////////////////
static float32 SolidGradientRepeat(ESolidRepeat eRepeat, float32 fDistance)
{
	float32	fPart;

	if (eRepeat == SR_Once)
	{
		return (fDistance < 0.0f) ? 0.0f : ((fDistance > 1.0f) ? 1.0f : fDistance);
	}

	fPart = fDistance - floorf(fDistance);
	if (eRepeat == SR_Saw)
	{
		return fPart;
	}
	return 1.0f - fabsf((2.0f * fPart) - 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// The area under the repeat's value from 0 to a distance.  A saw and a
// wave both gather a half each whole number, and then their part's own.
//
//////////////////////////////////////////////////////////////////////////
static float32 SolidGradientArea(ESolidRepeat eRepeat, float32 fDistance)
{
	float32	fWhole;
	float32	fPart;

	if (eRepeat == SR_Once)
	{
		return (fDistance < 0.0f) ? 0.0f : ((fDistance > 1.0f) ? (fDistance - 0.5f) : (fDistance * fDistance * 0.5f));
	}

	fWhole = floorf(fDistance);
	fPart = fDistance - fWhole;
	if (eRepeat == SR_Saw)
	{
		return (fWhole * 0.5f) + (fPart * fPart * 0.5f);
	}
	return (fWhole * 0.5f) + ((fPart < 0.5f) ? (fPart * fPart) : ((2.0f * fPart) - (fPart * fPart) - 0.5f));
}


//////////////////////////////////////////////////////////////////////////
//
// The mean over the width is the area between its ends over the width.
//
//////////////////////////////////////////////////////////////////////////
float32 CSolidGradient::Value(SFloat32Vec3* psPosition, float32 fWidth)
{
	float32	fDistance;

	if (meFrom == SG_Plane)
	{
		fDistance = psPosition->x;
	}
	else if (meFrom == SG_Axis)
	{
		fDistance = sqrtf((psPosition->x * psPosition->x) + (psPosition->y * psPosition->y));
	}
	else
	{
		fDistance = sqrtf(Float32Vec3Dot(psPosition, psPosition));
	}

	if (fWidth < SOLID_GRADIENT_LEAST_WIDTH)
	{
		return SolidGradientRepeat(meRepeat, fDistance);
	}
	return (SolidGradientArea(meRepeat, fDistance + (fWidth * 0.5f)) - SolidGradientArea(meRepeat, fDistance - (fWidth * 0.5f))) / fWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidGradient::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	psSample->InitValue(Value(psPosition, fWidth));
}

