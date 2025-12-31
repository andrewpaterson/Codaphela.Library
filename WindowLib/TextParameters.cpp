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
#include "StandardLib/ClassDefines.h"
#include "TextParameters.h"
#include "Window.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTextParameters> CTextParameters::Init(Ptr<CFont> pcFont, Ptr<CWindow> pWindow)
{
	PreInit();

	mpFont = pcFont;
	mpWindow = pWindow;
	miTabSpaceCount = 4;
	msColour = 0xffffffff;

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::Class(void)
{
	M_Pointer(mpFont);
	M_Pointer(mpWindow);
	U_UInt32(msColour);
	U_Int16(miTabSpaceCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextParameters::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextParameters::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextParameters::SetFont(CFont* pcFont) { mpFont = pcFont; }
CGlyph* CTextParameters::GetGlyph(uint16 c) { return mpFont->GetGlyph(c); }
int16 CTextParameters::GetAscent(void) { return mpFont->GetAscent(); }
int16 CTextParameters::GetDescent(void) { return mpFont->GetDescent(); }
Ptr<CFont> CTextParameters::GetFont(void) { return mpFont; }
int16 CTextParameters::GetTabSpaceCount(void) { return miTabSpaceCount; }
int16 CTextParameters::GetSpaceWidth(void) { return mpFont->GetSpace(); }

