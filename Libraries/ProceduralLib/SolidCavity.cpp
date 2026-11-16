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
#include "BaseLib/FloatHelper.h"
#include "SolidCavity.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCavity::Init(Ptr<CShape> pShape, float32 fReach)
{
	PreInit();
	CSolidTexture::Init();
	mpShape = pShape;
	mfReach = fReach;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCavity::Class(void)
{
	CSolidTexture::Class();
	M_Pointer(mpShape);
	U_Float32(mfReach);
}


//////////////////////////////////////////////////////////////////////////
//
// A shape's distance may be a bound, less than the true distance, so each
// share is held to 0 to 1.
//
//////////////////////////////////////////////////////////////////////////
void CSolidCavity::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sOut;
	SFloat32Vec3	sIn;
	float32			fHollowed;
	float32			fEdged;

	if (mpShape.IsNull() || (mfReach <= 0.0f))
	{
		psSample->InitValue(0.5f);
		return;
	}

	FromOwn(&sPosition, &sNormal, psPosition, psNormal);
	sOut = sPosition + (sNormal * mfReach);
	sIn = sPosition - (sNormal * mfReach);
	fHollowed = 1.0f - (mpShape->Distance(&sOut) / mfReach);
	fEdged = 1.0f + (mpShape->Distance(&sIn) / mfReach);
	fHollowed = minf(maxf(fHollowed, 0.0f), 1.0f);
	fEdged = minf(maxf(fEdged, 0.0f), 1.0f);
	psSample->InitValue(0.5f + (0.5f * (fEdged - fHollowed)));
}

