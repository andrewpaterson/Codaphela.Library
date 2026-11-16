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
#include "SolidMerge.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMerge::Init(ESolidMerge eMerge, Ptr<CSolidTexture> pFirst, Ptr<CSolidTexture> pSecond)
{
	PreInit();
	CSolidOperator::Init();
	mpFirst = pFirst;
	mpSecond = pSecond;
	meMerge = eMerge;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMerge::Class(void)
{
	CSolidOperator::Class();
	M_Pointer(mpFirst);
	M_Pointer(mpSecond);
	U_Enum(meMerge);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 SolidMergeChannel(ESolidMerge eMerge, float32 fFirst, float32 fSecond)
{
	switch (eMerge)
	{
	case SM_Add:
		return fFirst + fSecond;
	case SM_Multiply:
		return fFirst * fSecond;
	case SM_Lesser:
		return minf(fFirst, fSecond);
	case SM_Greater:
		return maxf(fFirst, fSecond);
	default:
		return fabsf(fFirst - fSecond);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMerge::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SSolidSample	sSecond;
	float32*		pafFirst;
	float32*		pafSecond;
	size			i;

	Input(mpFirst, psPosition, psNormal, fWidth, psSample);
	Input(mpSecond, psPosition, psNormal, fWidth, &sSecond);
	pafFirst = psSample->Channels();
	pafSecond = sSecond.Channels();
	for (i = 0; i < SOLID_SAMPLE_CHANNELS; i++)
	{
		pafFirst[i] = SolidMergeChannel(meMerge, pafFirst[i], pafSecond[i]);
	}
}

