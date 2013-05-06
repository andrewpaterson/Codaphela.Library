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
#include "WorldLib/World.h"
#include "WorldLib/TextureConverter.h"
#include "TextParameters.h"
#include "Viewport.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::Init(CFont* pcFont, CGraphicsState* pcGraphicsState, CWorld* pcWorld)
{
	CTextureConverter	cTextureConverter;

	CQuadParameters::Init(pcGraphicsState);
	mpcFont = pcFont;
	miTabSpaceCount = 4;
	miColour = 0xffffffff;

	mpcGraphicsTexture = NULL;
	cTextureConverter.Init(pcWorld);
	cTextureConverter.Convert(&mpcGraphicsTexture, pcFont->GetImage(), FALSE);
	cTextureConverter.Kill();
	mpcGraphicsMaterial = CreateMaterial(1, pcWorld, pcFont->GetImage()->HasChannel(IMAGE_OPACITY));
	mpcGraphicsMaterial->SetTexture(0, mpcGraphicsTexture);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::Kill(void)
{
	mpcGraphicsTexture->Kill();
	mpcGraphicsMaterial->Kill();
	CQuadParameters::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::SetFont(CFont* pcFont) { mpcFont = pcFont; }
CGlyph* CTextParameters::GetGlyph(char c) { return mpcFont->GetGlyph(c); }
int CTextParameters::GetAscent(void) { return mpcFont->GetAscent(); }
int CTextParameters::GetDescent(void) { return mpcFont->GetDescent(); }
CGraphicsTexture* CTextParameters::GetGraphicsTexture(void) { return mpcGraphicsTexture; }
CGraphicsMaterial* CTextParameters::GetGraphicsMaterial(void) { return mpcGraphicsMaterial; }
CFont* CTextParameters::GetFont(void) { return mpcFont; }
