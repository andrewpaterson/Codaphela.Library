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
#include "SolidWarp.h"


//Where the warp texture is read for the y and z axes' moves, from the point: far enough that a noise there has nothing to do
//with the noise here.
static const float32 gaafSolidWarpElsewhere[2][3] =
{
	{ 31.4f, -17.3f, 47.2f},
	{-27.8f,  91.1f, 13.6f},
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidWarp::Init(Ptr<CSolidTexture> pInput, Ptr<CSolidTexture> pWarp, float32 fAmount)
{
	PreInit();
	CSolidOperator::Init();
	mpInput = pInput;
	mpWarp = pWarp;
	mfAmount = fAmount;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidWarp::Class(void)
{
	CSolidOperator::Class();
	M_Pointer(mpInput);
	M_Pointer(mpWarp);
	U_Float32(mfAmount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidWarp::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sElsewhere;
	size			i;

	sPosition = *psPosition;
	if (mpWarp.IsNotNull() && (mfAmount != 0.0f))
	{
		mpWarp->Sample(psPosition, psNormal, fWidth, psSample);
		sPosition.x += mfAmount * ((2.0f * psSample->Value()) - 1.0f);
		for (i = 0; i < 2; i++)
		{
			sElsewhere.Init(psPosition->x + gaafSolidWarpElsewhere[i][0], psPosition->y + gaafSolidWarpElsewhere[i][1], psPosition->z + gaafSolidWarpElsewhere[i][2]);
			mpWarp->Sample(&sElsewhere, psNormal, fWidth, psSample);
			(&sPosition.y)[i] += mfAmount * ((2.0f * psSample->Value()) - 1.0f);
		}
	}
	Input(mpInput, &sPosition, psNormal, fWidth, psSample);
}

