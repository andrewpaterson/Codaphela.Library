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
#include "GraphicsState.h"
#include "DirectXLib/Direct3DGlobal.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::Init(void)
{
	miNumTextures = 0;
	miNumSamplers = 0;
	memset(mcStageState, -1, sizeof(SStageState)*MAX_TEX);
	memset(&mcSamplerState, -1, sizeof(SSamplerState)*MAX_SAMPLE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::Draw(void)
{
	int		i;
	int		iNumStages;

	//Sort out how many texture stage states there are.
	iNumStages = miNumTextures;
	if (iNumStages == MAX_TEX)
	{
		iNumStages--;
	}
	iNumStages++;

	for (i = 0; i < iNumStages; i++)
	{
		gcD3D.SetStageState(i, &mcStageState[i]);
	}

	for (i = 0; i < miNumSamplers; i++)
	{
		gcD3D.SetSamplerState(i, &mcSamplerState[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetNumTextures(int iNum)
{
	int		i;

	if (iNum > MAX_TEX)
	{
		iNum = MAX_TEX;
	}

	miNumTextures = iNum;
	if (iNum > 0)
	{
		for (i = 0; i < iNum; i++)
		{
			gcD3D.CreateStageState(&mcStageState[i]);
		}
		if (i != MAX_TEX)
		{
			memset(&mcStageState[i], 0, sizeof(SStageState));
			gcD3D.CreateStageState(&mcStageState[i]);
			mcStageState[i].iColourOp = D3DTOP_DISABLE;
			mcStageState[i].iAlphaOp = D3DTOP_DISABLE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetNumSamplers(int iNum)
{
	int		i;

	miNumSamplers = iNum;
	for (i = 0; i < iNum; i++)
	{
		gcD3D.CreateSamplerState(&mcSamplerState[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetZeroTextureDefault(void)
{
	SetNumTextures(0);
	SetNumSamplers(0);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetZeroTextureDefaultPartial(void)
{
	gcD3D.CreateStageState(&mcStageState[0]);
	mcStageState[0].iColourOp   = D3DTOP_SELECTARG1;
	mcStageState[0].iColourArg1 = D3DTA_DIFFUSE;
	mcStageState[0].iColourArg2 = D3DTA_DIFFUSE;
	mcStageState[0].iAlphaOp    = D3DTOP_SELECTARG1;
	mcStageState[0].iAlphaArg1  = D3DTA_DIFFUSE;
	mcStageState[0].iAlphaArg2  = D3DTA_DIFFUSE;
	mcStageState[0].iResultArg = D3DTA_CURRENT;
	mcStageState[0].iTexCoord = 0;

	gcD3D.CreateStageState(&mcStageState[1]);
	mcStageState[1].iColourOp   = D3DTOP_DISABLE;
	mcStageState[1].iColourArg1 = D3DTA_DIFFUSE;
	mcStageState[1].iColourArg2 = D3DTA_DIFFUSE;
	mcStageState[1].iAlphaOp    = D3DTOP_DISABLE;
	mcStageState[1].iAlphaArg1  = D3DTA_DIFFUSE;
	mcStageState[1].iAlphaArg2  = D3DTA_DIFFUSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetDefaults(int iNumTextureLayers, D3DTEXTUREFILTERTYPE iFilter)
{
	SetNumTextures(iNumTextureLayers);
	SetNumSamplers(iNumTextureLayers);

	SetDefaultTextureArgs();
	SetDefaultSamplerStates(iFilter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetDefaultSamplerStates(D3DTEXTUREFILTERTYPE iFilter)
{
	int i;

	for (i = 0; i < miNumSamplers; i++)
	{
		gcD3D.CreateSamplerState(&mcSamplerState[i]);
		mcSamplerState[i].iAddressU = D3DTADDRESS_WRAP;
		mcSamplerState[i].iAddressV = D3DTADDRESS_WRAP;
		mcSamplerState[i].iMagFilter = iFilter;
		mcSamplerState[i].iMinFilter = iFilter;
		if (iFilter != D3DTEXF_POINT)
		{
			mcSamplerState[i].iMipFilter = iFilter;
		}
		else
		{
			mcSamplerState[i].iMipFilter = D3DTEXF_NONE;
		}
		mcSamplerState[i].fMipmapLodBias = 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGraphicsState::SetDefaultTextureArgs(void)
{
	int i;

	if (miNumTextures == 0)
	{
		SetZeroTextureDefaultPartial();
		return;
	}

	gcD3D.CreateStageState(&mcStageState[0]);
	mcStageState[0].iColourOp = D3DTOP_MODULATE;
	mcStageState[0].iColourArg1 = D3DTA_TEXTURE;
	mcStageState[0].iColourArg2 = D3DTA_DIFFUSE;
	mcStageState[0].iAlphaOp = D3DTOP_SELECTARG1;
	mcStageState[0].iAlphaArg1 = D3DTA_TEXTURE;
	mcStageState[0].iAlphaArg2 = D3DTA_DIFFUSE;
	mcStageState[0].iResultArg = D3DTA_CURRENT;
	mcStageState[0].iTexCoord = 0;

	for (i = 1; i < miNumTextures; i++)
	{
		gcD3D.CreateStageState(&mcStageState[i]);
		mcStageState[i].iColourOp = D3DTOP_MODULATE;
		mcStageState[i].iColourArg1 = D3DTA_TEXTURE;
		mcStageState[i].iColourArg2 = D3DTA_CURRENT;
		mcStageState[i].iAlphaOp = D3DTOP_SELECTARG1;
		mcStageState[i].iAlphaArg1 = D3DTA_TEXTURE;
		mcStageState[i].iAlphaArg2 = D3DTA_CURRENT;
		mcStageState[i].iResultArg = D3DTA_CURRENT;
		mcStageState[i].iTexCoord = 0;
	}

	if (i < MAX_TEX)
	{
		gcD3D.CreateStageState(&mcStageState[i]);
		mcStageState[i].iColourOp   = D3DTOP_DISABLE;
		mcStageState[i].iColourArg1 = D3DTA_DIFFUSE;
		mcStageState[i].iColourArg2 = D3DTA_DIFFUSE;
		mcStageState[i].iAlphaOp    = D3DTOP_DISABLE;
		mcStageState[i].iAlphaArg1  = D3DTA_DIFFUSE;
		mcStageState[i].iAlphaArg2  = D3DTA_DIFFUSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSamplerState* CGraphicsState::GetSamplerState(int iSamplerState)
{
	return &mcSamplerState[iSamplerState];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SStageState* CGraphicsState::GetStageState(int iStageState)
{
	return &mcStageState[iStageState];
}

