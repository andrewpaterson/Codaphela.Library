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
#include "MaterialImage.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialImage::Init(void)
{
	PreInit();

	mpImage = NULL;
	mszImageName.Init();
	meAddressU = UVA_Tile;
	meAddressV = UVA_Tile;
	meMagFilter = UVF_Nearest;
	meMinFilter = UVF_Nearest;
	meMipFilter = UVF_Nearest;
	meUVSource = UVS_0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialImage::Free(void)
{
	mszImageName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialImage::Class(void)
{
	M_Pointer(mpImage);
	U_String(mszImageName);
	U_Enum(meAddressU);
	U_Enum(meAddressV);
	U_Enum(meMagFilter);
	U_Enum(meMinFilter);
	U_Enum(meMipFilter);
	U_Enum(meUVSource);
}

