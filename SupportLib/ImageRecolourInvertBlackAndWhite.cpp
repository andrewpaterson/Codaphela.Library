/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ImageRecolourInvertBlackAndWhite.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolourInvertBlackAndWhite::Init(CRectangle* pcRectangle)
{
	cARGB0000.Init(0.0f,0.0f,0.0f,0.0f);
	cARGB1111.Init(1.0f,1.0f,1.0f,1.0f);
	cARGB0111.Init(0.0f,1.0f,1.0f,1.0f);

	CImageRecolour::Init(&cARGB0000, &cARGB1111, pcRectangle);
	AddRecolour(&cARGB0111, &cARGB0000);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageRecolourInvertBlackAndWhite::Kill(void)
{
	CImageRecolour::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImageColourARGB* CImageRecolourInvertBlackAndWhite::GetBlack(void)
{
	return &cARGB0000;
}

