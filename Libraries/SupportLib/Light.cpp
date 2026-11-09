/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Light.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Init(void)
{
	PreInit();

	mszName.Init();
	meType = LT_Omni;
	msColour.Init(0, 0, 0, 0);
	meDecay = LDT_None;
	mfDecayStart = 0.0f;
	mfRange = 0.0f;
	mbCastShadows = false;
	mfIntensity = 0.0f;
	mpConnection = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Class(void)
{
	U_String(mszName);
	U_Enum(meType);
	U_Float32(msColour.mfAlpha);
	U_Float32(msColour.mfRed);
	U_Float32(msColour.mfGreen);
	U_Float32(msColour.mfBlue);
	U_Enum(meDecay);
	U_Float32(mfDecayStart);
	U_Float32(mfRange);
	U_Bool(mbCastShadows);
	U_Float32(mfIntensity);
	M_Pointer(mpConnection);
}

