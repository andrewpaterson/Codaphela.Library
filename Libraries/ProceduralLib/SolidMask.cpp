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
#include "SolidMask.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMask::Init(Ptr<CSolidTexture> pFirst, Ptr<CSolidTexture> pSecond, Ptr<CSolidTexture> pMask)
{
	PreInit();
	CSolidOperator::Init();
	mpFirst = pFirst;
	mpSecond = pSecond;
	mpMask = pMask;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMask::Class(void)
{
	CSolidOperator::Class();
	M_Pointer(mpFirst);
	M_Pointer(mpSecond);
	M_Pointer(mpMask);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidMask::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	SSolidSample	sSecond;
	float32			fAmount;

	Input(mpMask, psPosition, psNormal, fWidth, psSample);
	fAmount = psSample->Value();
	if (fAmount >= 1.0f)
	{
		Input(mpSecond, psPosition, psNormal, fWidth, psSample);
		return;
	}

	Input(mpFirst, psPosition, psNormal, fWidth, psSample);
	if (fAmount > 0.0f)
	{
		Input(mpSecond, psPosition, psNormal, fWidth, &sSecond);
		psSample->Blend(&sSecond, fAmount);
	}
}

