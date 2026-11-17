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
#include "SolidChannel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidChannel::Init(ESolidChannel eChannel, Ptr<CSolidTexture> pInput)
{
	PreInit();

	CSolidOperator::Init();
	mpInput = pInput;
	meChannel = eChannel;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidChannel::Class(void)
{
	CSolidOperator::Class();

	M_Pointer(mpInput);
	U_Enum(meChannel);
}


//////////////////////////////////////////////////////////////////////////
//
// A channel there isn't is nothing: black.
//
//////////////////////////////////////////////////////////////////////////
void CSolidChannel::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SSolidSample	sInput;
	float32			fChannel;

	Input(mpInput, psPosition, psNormal, fWidth, &sInput);
	fChannel = ((size)meChannel < SOLID_SAMPLE_CHANNELS) ? sInput.Channels()[meChannel] : 0.0f;
	psSample->Init();
	psSample->sColour.x = fChannel;
	psSample->sColour.y = fChannel;
	psSample->sColour.z = fChannel;
}

