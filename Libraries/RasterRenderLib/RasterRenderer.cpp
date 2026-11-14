/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "SupportLib/Image.h"
#include "RasterMeshBuild.h"
#include "RasterRenderer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderer::Init(int32 iScale)
{
	PreInit();

	CRenderer::Init();
	miScale = iScale;
	mapTextures.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderer::Class(void)
{
	CRenderer::Class();
	U_Int32(miScale);
	M_Embedded(mapTextures);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterRenderer::Free(void)
{
	CRenderer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CRasterRenderer::GetScale(void)
{
	return miScale;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for no image and one that can't be a texture.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRasterTexture> CRasterRenderer::GetTexture(CImage* pcImage)
{
	Ptr<CRasterTexture>	pTexture;
	size				i;

	if (pcImage == NULL)
	{
		return pTexture;
	}
	for (i = 0; i < mapTextures.NumElements(); i++)
	{
		pTexture = mapTextures.Get(i);
		if (pTexture->GetSource() == (uint64)pcImage->GetIndex())
		{
			return pTexture;
		}
	}
	pTexture = MakeRasterTexture(pcImage);
	if (pTexture.IsNotNull())
	{
		mapTextures.Add(pTexture);
	}
	return pTexture;
}

