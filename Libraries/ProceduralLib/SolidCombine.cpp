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
#include "SolidCombine.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCombine::Init(Ptr<CSolidTexture> pColour, Ptr<CSolidTexture> pHeight, Ptr<CSolidTexture> pShine, Ptr<CSolidTexture> pGloss, Ptr<CSolidTexture> pGlow)
{
	PreInit();

	CSolidOperator::Init();
	mpColour = pColour;
	mpHeight = pHeight;
	mpShine = pShine;
	mpGloss = pGloss;
	mpGlow = pGlow;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSolidCombine::Class(void)
{
	CSolidOperator::Class();

	M_Pointer(mpColour);
	M_Pointer(mpHeight);
	M_Pointer(mpShine);
	M_Pointer(mpGloss);
	M_Pointer(mpGlow);
}


//////////////////////////////////////////////////////////////////////////
//
// The brightness of an input's colour.
//
//////////////////////////////////////////////////////////////////////////
float32 CSolidCombine::InputValue(Ptr<CSolidTexture>& pInput, SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth)
{
	SSolidSample	sInput;

	Input(pInput, psPosition, psNormal, fWidth, &sInput);
	return sInput.Value();
}


//////////////////////////////////////////////////////////////////////////
//
// Only the colour input's colour is taken: its own height, shine, gloss
// and glow are left behind.
//
//////////////////////////////////////////////////////////////////////////
void CSolidCombine::TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample)
{
	Input(mpColour, psPosition, psNormal, fWidth, psSample);
	psSample->fHeight = InputValue(mpHeight, psPosition, psNormal, fWidth);
	psSample->fShine = InputValue(mpShine, psPosition, psNormal, fWidth);
	psSample->fGloss = InputValue(mpGloss, psPosition, psNormal, fWidth);
	psSample->fGlow = InputValue(mpGlow, psPosition, psNormal, fWidth);
}

