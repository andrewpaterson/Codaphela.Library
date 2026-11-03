/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "ImageRecolourInvertBlackAndWhite.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolourInvertBlackAndWhite::Init(CInt32Rect* pcRectangle)
{
	PreInit();

	mcARGB0000.Init(0.0f,0.0f,0.0f,0.0f);
	mcARGB1111.Init(1.0f,1.0f,1.0f,1.0f);
	mcARGB0111.Init(0.0f,1.0f,1.0f,1.0f);

	CImageRecolour::Init(&mcARGB0000, &mcARGB1111, pcRectangle);
	AddRecolour(&mcARGB0111, &mcARGB0000);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolourInvertBlackAndWhite::Class(void)
{
	CImageRecolour::Class();
	U_Data(CImageColourARGB, mcARGB0000);
	U_Data(CImageColourARGB, mcARGB1111);
	U_Data(CImageColourARGB, mcARGB0111);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageColourARGB* CImageRecolourInvertBlackAndWhite::GetBlack(void)
{
	return &mcARGB0000;
}

