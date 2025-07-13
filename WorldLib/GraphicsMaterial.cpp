/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/Unknowns.h"
#include "GraphicsTexture.h"
#include "GraphicsMaterial.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::Init(void)
{
	int		i;

	gcD3D.CreateMaterial(&msMaterial);
	masRenderStates.Init();

	msMaterial.Zero();
	for (i = 0; i < MAX_TEX; i++)
	{
		mapsTexture[i] = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::Kill(void)
{
	masRenderStates.Kill();
	gcD3D.ReleaseMaterial(&msMaterial);
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::Draw(void)
{
	int				i;

	//Set the textures.
	for (i = 0; i < miNumTextures; i++)
	{
		if (mapsTexture[i])
		{
			gcD3D.SetTexture(i, mapsTexture[i]->GetTexture());
		}
		else
		{
			gcD3D.SetTexture(i, gcD3D.GetNullTexture());
		}
	}
	for (; i < MAX_TEX; i++)
	{
		gcD3D.SetTexture(i, gcD3D.GetNullTexture());
	}

	//Set the material.
	gcD3D.SetMaterial(&msMaterial);

	//Set any unusual render states this material might use.
	SetRenderStates();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetRenderState(D3DRENDERSTATETYPE iD3DRenderState, DWORD dwValue)
{
	int				iRenderStateIndex;
	SRenderState*	psRenderState;

	iRenderStateIndex = masRenderStates.FindWithIntKey(iD3DRenderState, 0);
	if (iRenderStateIndex == -1)
	{
		psRenderState = masRenderStates.Add();
		psRenderState->dwValue = dwValue;
		psRenderState->dwPrior = -1;
		psRenderState->iD3DRenderState = iD3DRenderState;
	}
	else
	{
		psRenderState = masRenderStates.Get(iRenderStateIndex);
		psRenderState->dwValue = dwValue;
		psRenderState->dwPrior = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetRenderStates(void)
{
	SRenderState*	psRenderState;
	size			i;

	for (i = 0; i < masRenderStates.NumElements(); i++)
	{
		psRenderState = masRenderStates.Get(i);
		psRenderState->dwPrior = gcD3D.GetRenderState(psRenderState->iD3DRenderState);
		gcD3D.SetRenderState(psRenderState->iD3DRenderState, psRenderState->dwValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::UnsetRenderStates(void)
{
	size			i;
	SRenderState*	psRenderState;

	//Remove any unusual render states this material uses.
	for (i = 0; i < masRenderStates.NumElements(); i++)
	{
		psRenderState = masRenderStates.Get(i);
		gcD3D.SetRenderState((D3DRENDERSTATETYPE)psRenderState->iD3DRenderState, psRenderState->dwPrior);
		psRenderState->dwPrior = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetTexture(int iTexureNum, CGraphicsTexture* pcTexture)
{
	if (iTexureNum < MAX_TEX)
	{
		mapsTexture[iTexureNum] =  pcTexture;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetGreyDefaults(int iNumTextures)
{
	int	i;

	miNumTextures = iNumTextures;
	for (i = 0; i < iNumTextures; i++)
	{
		mapsTexture[i] = NULL;
	}

	gcD3D.CreateMaterial(&msMaterial);
	msMaterial.sD3DMaterial.Diffuse.r  = 0.7f;
	msMaterial.sD3DMaterial.Diffuse.g  = 0.7f;
	msMaterial.sD3DMaterial.Diffuse.b  = 0.7f;
	msMaterial.sD3DMaterial.Diffuse.a  = 0.7f;
	msMaterial.sD3DMaterial.Specular.r = 1.0f;
	msMaterial.sD3DMaterial.Specular.g = 1.0f;
	msMaterial.sD3DMaterial.Specular.b = 1.0f;
	msMaterial.sD3DMaterial.Specular.a = 1.0f;
	msMaterial.sD3DMaterial.Ambient.r  = 0.0f;
	msMaterial.sD3DMaterial.Ambient.g  = 0.0f;
	msMaterial.sD3DMaterial.Ambient.b  = 0.0f;
	msMaterial.sD3DMaterial.Ambient.a  = 0.0f;
	msMaterial.sD3DMaterial.Emissive.r = 0.0f;
	msMaterial.sD3DMaterial.Emissive.g = 0.0f;
	msMaterial.sD3DMaterial.Emissive.b = 0.0f;
	msMaterial.sD3DMaterial.Emissive.a = 0.0f;
	msMaterial.sD3DMaterial.Power      = 16.0f;
	SetRenderState(D3DRS_LIGHTING, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetWhiteDefaults(int iNumTextures)
{
	int	i;

	miNumTextures = iNumTextures;
	for (i = 0; i < iNumTextures; i++)
	{
		mapsTexture[i] = NULL;
	}

	gcD3D.CreateMaterial(&msMaterial);
	msMaterial.sD3DMaterial.Diffuse.r  = 1.0f;
	msMaterial.sD3DMaterial.Diffuse.g  = 1.0f;
	msMaterial.sD3DMaterial.Diffuse.b  = 1.0f;
	msMaterial.sD3DMaterial.Diffuse.a  = 1.0f;
	msMaterial.sD3DMaterial.Specular.r = 0.0f;
	msMaterial.sD3DMaterial.Specular.g = 0.0f;
	msMaterial.sD3DMaterial.Specular.b = 0.0f;
	msMaterial.sD3DMaterial.Specular.a = 0.0f;
	msMaterial.sD3DMaterial.Ambient.r  = 1.0f;
	msMaterial.sD3DMaterial.Ambient.g  = 1.0f;
	msMaterial.sD3DMaterial.Ambient.b  = 1.0f;
	msMaterial.sD3DMaterial.Ambient.a  = 1.0f;
	msMaterial.sD3DMaterial.Emissive.r = 0.0f;
	msMaterial.sD3DMaterial.Emissive.g = 0.0f;
	msMaterial.sD3DMaterial.Emissive.b = 0.0f;
	msMaterial.sD3DMaterial.Emissive.a = 0.0f;
	msMaterial.sD3DMaterial.Power      = 0.0f;
	SetRenderState(D3DRS_LIGHTING, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CGraphicsMaterial::GetTexture(int iTexureNum)
{
	if ((iTexureNum >= 0) && (iTexureNum < miNumTextures))
	{
		return mapsTexture[iTexureNum];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGraphicsMaterial::GetNumTextures(void)
{
	return miNumTextures;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsMaterial::SetNumTextures(int iNumTextures)
{
	miNumTextures = iNumTextures;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CopyToD3DCOLORVALUE(D3DCOLORVALUE* psDest, CImageColourARGB* psSource)
{
	psDest->a = psSource->a;
	psDest->r = psSource->r;
	psDest->g = psSource->g;
	psDest->b = psSource->b;
}


void CGraphicsMaterial::SetDiffuse(CImageColourARGB* psfColour)
{
	CopyToD3DCOLORVALUE(&msMaterial.sD3DMaterial.Diffuse, psfColour);
}
void CGraphicsMaterial::SetAmbient(CImageColourARGB* psfColour)
{
	CopyToD3DCOLORVALUE(&msMaterial.sD3DMaterial.Ambient, psfColour);
}
void CGraphicsMaterial::SetSpecular(CImageColourARGB* psfColour)
{
	CopyToD3DCOLORVALUE(&msMaterial.sD3DMaterial.Specular, psfColour);
}
void CGraphicsMaterial::SetEmissive(CImageColourARGB* psfColour)
{
	CopyToD3DCOLORVALUE(&msMaterial.sD3DMaterial.Emissive, psfColour);
}
void CGraphicsMaterial::SetPower(float fPower)
{
	msMaterial.sD3DMaterial.Power = fPower;
}
void CGraphicsMaterial::ModulateSpecular(float fValue)
{
	msMaterial.sD3DMaterial.Specular.r *= fValue;
	msMaterial.sD3DMaterial.Specular.g *= fValue;
	msMaterial.sD3DMaterial.Specular.b *= fValue;
}
