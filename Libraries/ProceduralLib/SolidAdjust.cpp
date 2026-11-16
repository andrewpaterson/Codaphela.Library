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
#include "SolidAdjust.h"


#define SOLID_ADJUST_TURN		(6.2831853f)	//Radians.
#define SOLID_ADJUST_ROOT_THIRD	(0.5773503f)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidAdjust::Init(Ptr<CSolidTexture> pInput)
{
	PreInit();
	CSolidOperator::Init();
	mpInput = pInput;
	mbInvert = false;
	mfHue = 0.0f;
	mfSaturation = 1.0f;
	mfContrast = 1.0f;
	mfBrightness = 0.0f;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidAdjust::Class(void)
{
	CSolidOperator::Class();
	M_Pointer(mpInput);
	U_Bool(mbInvert);
	U_Float32(mfHue);
	U_Float32(mfSaturation);
	U_Float32(mfContrast);
	U_Float32(mfBrightness);
}


//////////////////////////////////////////////////////////////////////////
//
// Greys lie along the line red = green = blue, and a colour's hue is its
// direction round that line: turning the colour about it (Rodrigues'
// rotation, the axis (1, 1, 1) / root 3) turns the hue and keeps its
// brightness and saturation.  Red goes to green in a third of a turn.
//
//////////////////////////////////////////////////////////////////////////
static void SolidAdjustTurnHue(SFloat32Vec4* psColour, float32 fTurns)
{
	SFloat32Vec3	sColour;
	SFloat32Vec3	sAcross;
	float32			fGrey;
	float32			fCos;
	float32			fSin;

	fCos = cosf(fTurns * SOLID_ADJUST_TURN);
	fSin = sinf(fTurns * SOLID_ADJUST_TURN) * SOLID_ADJUST_ROOT_THIRD;
	sColour.Init(psColour->x, psColour->y, psColour->z);
	sAcross.Init(sColour.z - sColour.y, sColour.x - sColour.z, sColour.y - sColour.x);
	fGrey = (sColour.x + sColour.y + sColour.z) / 3.0f;
	psColour->x = fGrey + ((sColour.x - fGrey) * fCos) + (sAcross.x * fSin);
	psColour->y = fGrey + ((sColour.y - fGrey) * fCos) + (sAcross.y * fSin);
	psColour->z = fGrey + ((sColour.z - fGrey) * fCos) + (sAcross.z * fSin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidAdjust::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	float32*	pafColour;
	float32		fGrey;
	size		i;

	Input(mpInput, psPosition, psNormal, fWidth, psSample);
	pafColour = psSample->Channels();
	if (mbInvert)
	{
		for (i = 0; i < 3; i++)
		{
			pafColour[i] = 1.0f - pafColour[i];
		}
	}
	if (mfHue != 0.0f)
	{
		SolidAdjustTurnHue(&psSample->sColour, mfHue);
	}

	fGrey = psSample->Value();
	for (i = 0; i < 3; i++)
	{
		pafColour[i] = fGrey + ((pafColour[i] - fGrey) * mfSaturation);
		pafColour[i] = 0.5f + ((pafColour[i] - 0.5f) * mfContrast) + mfBrightness;
	}
}

