/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "RasterTexture.h"


//////////////////////////////////////////////////////////////////////////
//
// Its texels are white, and GetTexels is where they go.  A texture is at
// least one texel each way.
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Init(int32 iWidth, int32 iHeight, uint64 uiSource)
{
	size	uiNumTexels;

	PreInit();

	miWidth = (iWidth > 0) ? iWidth : 1;
	miHeight = (iHeight > 0) ? iHeight : 1;
	miMaskX = ((miWidth & (miWidth - 1)) == 0) ? miWidth - 1 : -1;
	miMaskY = ((miHeight & (miHeight - 1)) == 0) ? miHeight - 1 : -1;
	muiSource = uiSource;

	uiNumTexels = (size)miWidth * (size)miHeight;
	mauiTexels.Init();
	mauiTexels.Resize(uiNumTexels);
	memset(mauiTexels.GetData(), 0xFF, uiNumTexels * sizeof(uint32));

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Free(void)
{
	mauiTexels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Class(void)
{
	U_Unknown(CArrayRasterTexel, mauiTexels);
	U_Int32(miWidth);
	U_Int32(miHeight);
	U_Int32(miMaskX);
	U_Int32(miMaskY);
	U_UInt64(muiSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32* CRasterTexture::GetTexels(void) { return mauiTexels.GetData(); }
int32 CRasterTexture::GetWidth(void) { return miWidth; }
int32 CRasterTexture::GetHeight(void) { return miHeight; }
uint64 CRasterTexture::GetSource(void) { return muiSource; }

