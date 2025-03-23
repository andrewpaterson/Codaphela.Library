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
#include "WorldLib/World.h"
#include "SupportLib/ColourARGB.h"
#include "Block.h"
#include "BlockParameters.h"
#include "Viewport.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::Init(CViewport* pcViewport, CBlockParameters* pcBlockParameters)
{
	CDrawable::Init(pcViewport);

	mpcBlockParameters = pcBlockParameters;
	mfAlpha = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::Kill(void)
{
	CDrawable::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBlock::Draw(void)
{
	CreateQuad();
	return CDrawable::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::CreateQuad(void)
{
	SFloat2		sTopLeft;
	SFloat2		sBottomRight;
	int			i;
	CQuad2D*	pcQuad;

	pcQuad = mpcViewport->mcMap2D.CreateQuad(mpcBlockParameters->GetGraphicsMaterial(), mpcBlockParameters->GetGraphicsState(), mpcGraphicsViewport, (int)msActualSize.x, (int)msActualSize.y, (int)msPosition.x, (int)msPosition.y);

	for (i = 0; i < mpcBlockParameters->GetGraphicsMaterial()->GetNumTextures(); i++)
	{
		sTopLeft = mpcBlockParameters->macRects[i].GetUVCoordinatesTopLeft(GetTexture(i)->GetWidth(), GetTexture(i)->GetHeight());
		sBottomRight = mpcBlockParameters->macRects[i].GetUVCoordinatesBottomRight(GetTexture(i)->GetWidth(), GetTexture(i)->GetHeight());
		pcQuad->SetUVCoords(i, sTopLeft.x, sTopLeft.y, sBottomRight.x, sBottomRight.y);
	}

	AdjustColour(pcQuad);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::SetRequiredSize(void)
{
	msRequiredSize.x = mpcBlockParameters->msSize.x;
	msRequiredSize.y = mpcBlockParameters->msSize.y;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::AdjustColour(CQuad2D* pcQuad)
{
	float			fAlpha;
	unsigned int		iColour;

	fAlpha = Convert8BitColourToFloat(Get8BitAlphaColour(mpcBlockParameters->miColour));
	fAlpha *= mfAlpha;
	iColour = mpcBlockParameters->miColour | Set8BitAlphaColour(ConvertFloatTo8BitColour(fAlpha));
	pcQuad->SetColour(iColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlock::SetAlpha(float fAlpha)
{
	mfAlpha = fAlpha;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CBlock::GetTexture(int iLayer)
{
	return mpcBlockParameters->GetGraphicsMaterial()->GetTexture(iLayer);
}

