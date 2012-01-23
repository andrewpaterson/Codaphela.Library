/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "LightInstance.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::Init(void)
{
	memset(&msLight, 0, sizeof(D3DLIGHT9));
	msLight.Type = D3DLIGHT_POINT;
	msLight.Diffuse.a = msLight.Diffuse.b = msLight.Diffuse.g = msLight.Diffuse.r = LIGHT_PLEASANT;
	msLight.Specular.a = msLight.Specular.b = msLight.Specular.g = msLight.Specular.r = LIGHT_PLEASANT;
	msLight.Ambient.a = msLight.Ambient.b = msLight.Ambient.g = msLight.Ambient.r = 0.0f;
	msLight.Range = 3000.0f;
	msLight.Attenuation1 = (1.0f/(msLight.Range*2.0f));
	miSlot = -1;
	mbEnabled = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::SetPoint(float fConstAtten, float fLinearAtten, float fQuadraticAtten, float fRange)
{
	msLight.Type = D3DLIGHT_POINT;
	msLight.Attenuation0 = fConstAtten;
	msLight.Attenuation1 = fLinearAtten;
	msLight.Attenuation2 = fQuadraticAtten;
	msLight.Range = fRange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::SetDirectional(float fConstAtten, float fRange)
{
	msLight.Type = D3DLIGHT_DIRECTIONAL;
	msLight.Attenuation0 = fConstAtten;
	msLight.Range = fRange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLightInstance::Use(int iSlot)
{
	if (mbEnabled)
	{
		msLight.Position.x = mpsMatrix->Pos()->x;
		msLight.Position.y = mpsMatrix->Pos()->y;
		msLight.Position.z = mpsMatrix->Pos()->z;
		msLight.Direction.x = -mpsMatrix->At()->x;
		msLight.Direction.y = -mpsMatrix->At()->y;
		msLight.Direction.z = -mpsMatrix->At()->z;

		miSlot = iSlot;

		return gcD3D.LightEnable(miSlot, &msLight);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::Unuse(void)
{
	miSlot = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::SetColour(float r, float g, float b)
{
	msLight.Diffuse.b = b;
	msLight.Diffuse.g = g;
	msLight.Diffuse.r = r;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::SetGrey(float g)
{
	msLight.Diffuse.b = g;
	msLight.Diffuse.g = g;
	msLight.Diffuse.r = g;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLightInstance::SetMatrix(SMatrix* psMatrix) { mpsMatrix = psMatrix; }
SMatrix* CLightInstance::GetMatrix(void) { return mpsMatrix; }
BOOL CLightInstance::IsEnabled(void) { return mbEnabled; }
void CLightInstance::SetEnabled(BOOL bEnabled) { mbEnabled = bEnabled; }