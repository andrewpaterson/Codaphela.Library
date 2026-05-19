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
#include "MultiImageCopier.h"
#include "TileLayerCel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Init(Ptr<CTileMap> pTileMap, const char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition)
{
	PreInit();

	int			i;
	int			iSize;

	CTileLayer::Init(pTileMap, szTileType, sMapSize, sCelSize, sPosition);

	maTiles.Init();

	iSize = sMapSize.x * sMapSize.y;
	for (i = 0; i < iSize; i++)
	{
		maTiles.Add(NULL);
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Free(void)
{
	CTileLayer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Class(void)
{
	CTileLayer::Class();
	M_Embedded(maTiles);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::Load(CObjectReader* pcFile)
{
	return CTileLayer::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::Save(CObjectWriter* pcFile)
{
	return CTileLayer::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTile(int x, int y, Ptr<CImageCel> pTile)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maTiles.Set(x + iYOffset, pTile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(size uiIndex)
{
	return maTiles.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(int x, int y)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maTiles.Get(x + iYOffset);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTiles(int x, int y, Ptr<CArrayImageCel> paCels, size uiIndices ...)
{
	va_list			vaMarker;
	size			iIndex;
	size			uiNumCels;
	int				iMaxX;
	Ptr<CImageCel>	pCel;
	bool			bResult;

	uiNumCels = paCels->NumElements();
	iMaxX = msMapSize.x;
	bResult = true;

	va_start(vaMarker, uiIndices);
	iIndex = uiIndices;
	while ((iIndex < uiNumCels) && (x < iMaxX))
	{
		pCel = paCels->Get(iIndex);
		bResult &= pCel.IsNotNull();
		bResult &= SetTile(x, y, pCel);
		x++;
		iIndex = va_arg(vaMarker, char);
	}
	va_end(vaMarker);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTileLayerCel::WriteToImage(void)
{
	size				uiNumTiles;
	size				uiTile;
	Ptr<CImageCel>		pCel;
	Ptr<CImage>			pDestImage;
	CMultiImageCopier	cCopier;
	int32				x;
	int32				y;

	pDestImage = OMalloc<CImage>(msMapSize.x * msCelSize.x, msMapSize.y * msCelSize.y, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO);
	if (pDestImage.IsNull())
	{
		return NULL;
	}

	pDestImage->Clear();

	cCopier.Init(pDestImage);

	uiNumTiles = maTiles.NumElements();
	for (uiTile = 0; uiTile < uiNumTiles; uiTile++)
	{
		pCel = maTiles.Get(uiTile);
		if (pCel.IsNotNull())
		{
			cCopier.AddAccessor(pCel);
		}
	}

	for (y = 0; y < msMapSize.y; y++)
	{
		for (x = 0; x < msMapSize.x; x++)
		{
			pCel = GetTile(x, y);
			if (pCel.IsNotNull())
			{
				cCopier.Copy(x * msCelSize.x, y * msCelSize.y, pCel);
			}
		}
	}
	cCopier.Kill();

	return pDestImage;
}

