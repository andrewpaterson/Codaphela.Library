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
#include "SolidTexture.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSolidSample::Init(void)
{
	sColour.x = 0.0f;
	sColour.y = 0.0f;
	sColour.z = 0.0f;
	sColour.w = 1.0f;
	fHeight = 0.0f;
	fShine = 0.0f;
	fGloss = 0.0f;
	fGlow = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSolidSample::Init(ARGB32 uiColour)
{
	Init();
	sColour.x = Get8BitRedColour(uiColour) / 255.0f;
	sColour.y = Get8BitGreenColour(uiColour) / 255.0f;
	sColour.z = Get8BitBlueColour(uiColour) / 255.0f;
	sColour.w = Get8BitAlphaColour(uiColour) / 255.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSolidSample::InitValue(float32 fValue)
{
	Init();
	sColour.x = fValue;
	sColour.y = fValue;
	sColour.z = fValue;
	fHeight = fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 SSolidSample::Value(void)
{
	return (sColour.x + sColour.y + sColour.z) / 3.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32* SSolidSample::Channels(void)
{
	static_assert(sizeof(SSolidSample) == (SOLID_SAMPLE_CHANNELS * sizeof(float32)), "A sample is its channels and nothing else.");

	return &sColour.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SSolidSample::Blend(SSolidSample* psOther, float32 fAmount)
{
	float32*	pafChannels;
	float32*	pafOthers;
	size		i;

	pafChannels = Channels();
	pafOthers = psOther->Channels();
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		pafChannels[i] += (pafOthers[i] - pafChannels[i]) * fAmount;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint8 SolidSampleChannel(float32 fValue)
{
	if (!(fValue > 0.0f))
	{
		return 0;
	}
	if (fValue >= 1.0f)
	{
		return 255;
	}
	return (uint8)((fValue * 255.0f) + 0.5f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 SSolidSample::Colour(void)
{
	return Set32BitColour(SolidSampleChannel(sColour.x), SolidSampleChannel(sColour.y), SolidSampleChannel(sColour.z), SolidSampleChannel(sColour.w));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidTexture::Init(void)
{
	PreInit();
	msOrigin.Init(0, 0, 0);
	masAxes[0].Init(1, 0, 0);
	masAxes[1].Init(0, 1, 0);
	masAxes[2].Init(0, 0, 1);
	mfScale = 1.0f;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidTexture::Class(void)
{
	U_3Float32(msOrigin);
	U_3Float32Array(masAxes, 3);
	U_Float32(mfScale);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidTexture::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidTexture::Place(SFloat32Vec3* psOrigin, SFloat32Vec3* psXAxis, SFloat32Vec3* psYAxis, float32 fScale)
{
	msOrigin = *psOrigin;
	masAxes[0] = *psXAxis;
	masAxes[0].Normalize();
	Float32Vec3Cross(&masAxes[2], &masAxes[0], psYAxis);
	masAxes[2].Normalize();
	Float32Vec3Cross(&masAxes[1], &masAxes[2], &masAxes[0]);
	mfScale = fScale;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidTexture::Sample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;

	sOffset = *psPosition - msOrigin;
	sPosition.Init(Float32Vec3Dot(&sOffset, &masAxes[0]) / mfScale, Float32Vec3Dot(&sOffset, &masAxes[1]) / mfScale, Float32Vec3Dot(&sOffset, &masAxes[2]) / mfScale);
	sNormal.Init(Float32Vec3Dot(psNormal, &masAxes[0]), Float32Vec3Dot(psNormal, &masAxes[1]), Float32Vec3Dot(psNormal, &masAxes[2]));
	psSample->Init();
	TextureSample(&sPosition, &sNormal, fWidth / mfScale, psSample);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CSolidTexture::Colour(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth)
{
	SSolidSample	sSample;

	Sample(psPosition, psNormal, fWidth, &sSample);
	return sSample.Colour();
}

