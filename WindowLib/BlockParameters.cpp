/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BlockParameters.h"
#include "SupportLib/ColourARGB.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlockParameters::Init(CGraphicsState* pcGraphicsState, CWorld* pcWorld, int r, int g, int b, int a)
{
	int					i;

	CQuadParameters::Init(pcGraphicsState);

	mpcGraphicsMaterial = CreateMaterial(0, pcWorld, FALSE);
	for (i = 0; i < MAX_TEX; i++)
	{
		macRects[i].Init();
	}
	SetColour(r, g, b, a);

	mpcGraphicsTexture = NULL;
	msSize.x = 0;
	msSize.y = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlockParameters::Kill(void)
{
	if (mpcGraphicsTexture != NULL)
	{
		mpcGraphicsTexture->Kill();
	}
	mpcGraphicsMaterial->Kill();
	CQuadParameters::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlockParameters::SetColour(int iColour)
{
	miColour = iColour;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlockParameters::SetColour(int r, int g, int b, int a)
{
	miColour = Set32BitColour(r, g, b, a);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsMaterial* CBlockParameters::GetGraphicsMaterial(void)
{
	return mpcGraphicsMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CBlockParameters::GetGraphicsTexture(void)
{
	return mpcGraphicsTexture;
}

