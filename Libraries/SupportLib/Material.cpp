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
#include "Material.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Init(void)
{
	PreInit();

	mszName.Init();
	msProperties.Init();
	msDiffuse.Init();
	msDiffuseLevel.Init();
	msSpecular.Init();
	msIllumination.Init();
	msOpacity.Init();
	msBump.Init();
	msDetail.Init();
	msDecal.Init();
	msReflection.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Free(void)
{
	msProperties.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterial::Class(void)
{
	U_String(mszName);
	U_Float32(msProperties.mcDiffuseColour.mfAlpha);
	U_Float32(msProperties.mcDiffuseColour.mfRed);
	U_Float32(msProperties.mcDiffuseColour.mfGreen);
	U_Float32(msProperties.mcDiffuseColour.mfBlue);
	U_Float32(msProperties.mcAmbientColour.mfAlpha);
	U_Float32(msProperties.mcAmbientColour.mfRed);
	U_Float32(msProperties.mcAmbientColour.mfGreen);
	U_Float32(msProperties.mcAmbientColour.mfBlue);
	U_Float32(msProperties.mcEmissiveColour.mfAlpha);
	U_Float32(msProperties.mcEmissiveColour.mfRed);
	U_Float32(msProperties.mcEmissiveColour.mfGreen);
	U_Float32(msProperties.mcEmissiveColour.mfBlue);
	U_Float32(msProperties.mcSpecularColour.mfAlpha);
	U_Float32(msProperties.mcSpecularColour.mfRed);
	U_Float32(msProperties.mcSpecularColour.mfGreen);
	U_Float32(msProperties.mcSpecularColour.mfBlue);
	U_Float32(msProperties.mfShininessPower);
	U_Float32(msProperties.mfShininessBase);
	U_Bool(msProperties.mbLit);
	U_Enum(msProperties.meOpacity);
	M_Embedded(msDiffuse);
	M_Embedded(msDiffuseLevel);
	M_Embedded(msSpecular);
	M_Embedded(msIllumination);
	M_Embedded(msOpacity);
	M_Embedded(msBump);
	M_Embedded(msDetail);
	M_Embedded(msDecal);
	M_Embedded(msReflection);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMaterialImage* CMaterial::GetImage(EMaterialImage eImage)
{
	switch (eImage)
	{
	case MI_Diffuse:
		return &msDiffuse;
	case MI_DiffuseLevel:
		return &msDiffuseLevel;
	case MI_Specular:
		return &msSpecular;
	case MI_Illumination:
		return &msIllumination;
	case MI_Opacity:
		return &msOpacity;
	case MI_Bump:
		return &msBump;
	case MI_Detail:
		return &msDetail;
	case MI_Decal:
		return &msDecal;
	case MI_Reflection:
		return &msReflection;
	default:
		return NULL;
	}
}

