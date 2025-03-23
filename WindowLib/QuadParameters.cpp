/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "QuadParameters.h"
#include "Viewport.h"
#include "SupportLib/ImageReader.h"
#include "WorldLib/World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuadParameters::Init(CGraphicsState* pcGraphicsState)
{
	mpcGraphicsState = pcGraphicsState;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuadParameters::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CQuadParameters::CreateMaterial(int iNumTextures, CWorld* pcWorld, BOOL bAlpha)
{
	CGraphicsMaterial*	pcMaterial;

	//All alpha values and no z buffer reads/writes.
	pcMaterial = pcWorld->CreateGraphicsMaterial();
	pcMaterial->SetWhiteDefaults(iNumTextures);
	if (bAlpha)
	{
		pcMaterial->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pcMaterial->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pcMaterial->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	pcMaterial->SetRenderState(D3DRS_ZENABLE, FALSE);
	pcMaterial->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	return pcMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsState* CQuadParameters::GetGraphicsState(void)
{
	return mpcGraphicsState;
}

