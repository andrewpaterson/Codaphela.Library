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
// Level 0's texels are white, and GetTexels is where they go.  A texture
// is at least one texel each way.
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Init(int32 iWidth, int32 iHeight, uint64 uiSource)
{
	PreInit();

	muiSource = uiSource;
	mauiTexels.Init();
	masLevels.Init();
	AddLevel(iWidth, iHeight);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Free(void)
{
	masLevels.Kill();
	mauiTexels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterTexture::Class(void)
{
	U_Unknown(CArrayRasterTexel, mauiTexels);
	U_Unknown(CArrayRasterTextureLevel, masLevels);
	U_UInt64(muiSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32* CRasterTexture::AddLevel(int32 iWidth, int32 iHeight)
{
	SRasterTextureLevel*	psLevel;
	size					uiNumTexels;

	psLevel = masLevels.Add();
	psLevel->uiStart = mauiTexels.NumElements();
	psLevel->iWidth = (iWidth > 0) ? iWidth : 1;
	psLevel->iHeight = (iHeight > 0) ? iHeight : 1;
	psLevel->iMaskX = ((psLevel->iWidth & (psLevel->iWidth - 1)) == 0) ? psLevel->iWidth - 1 : -1;
	psLevel->iMaskY = ((psLevel->iHeight & (psLevel->iHeight - 1)) == 0) ? psLevel->iHeight - 1 : -1;

	uiNumTexels = (size)psLevel->iWidth * (size)psLevel->iHeight;
	mauiTexels.Resize(psLevel->uiStart + uiNumTexels);
	memset(mauiTexels.GetData() + psLevel->uiStart, 0xFF, uiNumTexels * sizeof(uint32));
	return mauiTexels.GetData() + psLevel->uiStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CRasterTexture::NumLevels(void) const { return (int32)masLevels.NumElements(); }
uint32* CRasterTexture::GetTexels(int32 iLevel) { return mauiTexels.GetData() + Level(iLevel)->uiStart; }
int32 CRasterTexture::GetWidth(int32 iLevel) { return Level(iLevel)->iWidth; }
int32 CRasterTexture::GetHeight(int32 iLevel) { return Level(iLevel)->iHeight; }
uint64 CRasterTexture::GetSource(void) const { return muiSource; }

