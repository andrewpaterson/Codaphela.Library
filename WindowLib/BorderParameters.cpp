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
#include "SupportLib/ImageReader.h"
#include "WorldLib/World.h"
#include "WorldLib/TextureConverter.h"
#include "BorderParameters.h"
#include "Viewport.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::Init(int eStyle, EBorderEdgeType eEdgeType, CGraphicsState* pcGraphicsState, CWorld* pcWorld, char* szTexture, int iCornerWidth, int iConerHeight, int iEdgeWidth)
{
	CImage				cImage;

	CTextureConverter	cTextureConverter;

	CQuadParameters::Init(pcGraphicsState);
	meBorderStyle = eStyle;
	meEdgeType = eEdgeType;

	if (!ReadImage(&cImage, szTexture))
	{
		return;
	}

	mpcGraphicsTexture = NULL;
	cTextureConverter.Init(pcWorld);
	cTextureConverter.Convert(&mpcGraphicsTexture, &cImage, FALSE);
	cTextureConverter.Kill();

	mpcGraphicsMaterial = CreateMaterial(1, pcWorld, cImage.HasChannel(IMAGE_OPACITY));
	mpcGraphicsMaterial->SetTexture(0, mpcGraphicsTexture);
	cImage.Kill();

	SetCornerSizes(iCornerWidth, iConerHeight);
	SetEdgeWidths(iEdgeWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::Kill(void)
{
	mpcGraphicsTexture->Kill();
	mpcGraphicsMaterial->Kill();
	CQuadParameters::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::SetEdgeWidths(int iWidth1, int iWidth2, int iWidth3, int iWidth4)
{
	maiEdgeWidths[0] = iWidth1;
	maiEdgeWidths[1] = iWidth2;
	maiEdgeWidths[2] = iWidth3;
	maiEdgeWidths[3] = iWidth4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::SetEdgeWidths(int iWidth1)
{
	maiEdgeWidths[0] = iWidth1;
	maiEdgeWidths[1] = iWidth1;
	maiEdgeWidths[2] = iWidth1;
	maiEdgeWidths[3] = iWidth1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::SetCornerSizes(int iWidth, int iHeight)
{
	masCornerSizes[0].x = iWidth;
	masCornerSizes[0].y = iHeight;
	masCornerSizes[1].x = iWidth;
	masCornerSizes[1].y = iHeight;
	masCornerSizes[2].x = iWidth;
	masCornerSizes[2].y = iHeight;
	masCornerSizes[3].x = iWidth;
	masCornerSizes[3].y = iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::SetCornerSizes(int iWidth1, int iHeight1, int iWidth2, int iHeight2, int iWidth3, int iHeight3, int iWidth4, int iHeight4)
{
	masCornerSizes[0].x = iWidth1;
	masCornerSizes[0].y = iHeight1;
	masCornerSizes[1].x = iWidth2;
	masCornerSizes[1].y = iHeight2;
	masCornerSizes[2].x = iWidth3;
	masCornerSizes[2].y = iHeight3;
	masCornerSizes[3].x = iWidth4;
	masCornerSizes[3].y = iHeight4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBorderParameters::SetCornerSizes(int iSize1, int iSize2, int iSize3, int iSize4)
{
	masCornerSizes[0].x = iSize1;
	masCornerSizes[0].y = iSize1;
	masCornerSizes[1].x = iSize2;
	masCornerSizes[1].y = iSize2;
	masCornerSizes[2].x = iSize3;
	masCornerSizes[2].y = iSize3;
	masCornerSizes[3].x = iSize4;
	masCornerSizes[3].y = iSize4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CBorderParameters::GetGraphicsMaterial(void)
{
	return mpcGraphicsMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CBorderParameters::GetGraphicsTexture(void)
{
	return mpcGraphicsTexture;
}
