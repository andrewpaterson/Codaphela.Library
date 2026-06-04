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
#include "ImageCelBlitterCache.h"
#include "TileLayerCel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Init(Ptr<CTileMap> pTileMap, const char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition)
{
	PreInit();

	size	i;
	size	iSize;

	CTileLayer::Init(pTileMap, szTileType, sMapSize, sCelSize, sPosition);

	mpCache = NULL;
	mpViewport = NULL;

	maTiles.Init();
	maBlitters.Init();

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
	M_Embedded(maBlitters);
	M_Pointer(mpCache);
	M_Pointer(mpViewport);
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
bool CTileLayerCel::SetTile(size x, size y, Ptr<CImageCel> pTile)
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
Ptr<CImageCel> CTileLayerCel::GetTile(size x, size y)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maTiles.Get(x + iYOffset);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTiles(size x, size y, Ptr<CArrayImageCel> paCels, size uiIndices ...)
{
	va_list			vaMarker;
	size			iIndex;
	size			uiNumCels;
	size				iMaxX;
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
	Ptr<CImageCel>				pCel;
	int32						x;
	int32						y;
	Ptr<CImageCelBlitterCache>	pOldCache;
	Ptr<CImage>					pOldViewport;
	Ptr<CImage>					pImage;
	Ptr<CImageBlitter>			pBlitter;

	pOldViewport = mpViewport;
	pOldCache = mpCache;

	pImage = CreateViewportImage();
	mpViewport = pImage;
	mpCache = CreateBlitterCache();
	CreateCelBlitters();

	for (y = 0; y < msMapSize.y; y++)
	{
		for (x = 0; x < msMapSize.x; x++)
		{
			pBlitter = GetBlitter(x, y);
			if (pBlitter.IsNotNull())
			{
				pBlitter->Blit(x * msCelSize.x, y * msCelSize.y);
			}
		}
	}

	mpViewport = pOldViewport;
	mpCache = pOldCache;

	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::CreateCelBlitters(void)
{
	Ptr<CImageCel>		pCel;
	int32				x, y;
	Ptr<CImageBlitter>	pBlitter;
	size				iSize;
	size				i;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			maBlitters.Clear();

			iSize = msMapSize.x * msMapSize.y;
			for (i = 0; i < iSize; i++)
			{
				maBlitters.Add(NULL);
			}

			for (y = 0; y < msMapSize.y; y++)
			{
				for (x = 0; x < msMapSize.x; x++)
				{
					pCel = GetTile(x, y);
					if (pCel)
					{

						pBlitter = mpCache->CreateImageBlitter(pCel);
						if (pBlitter.IsNull())
						{
							return false;
						}
						SetBlitter(x, y, pBlitter);
					}
				}
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::ClearCelBlitters(void)
{
	maBlitters.Clear();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetBlitter(size x, size y, Ptr<CImageBlitter> pBlitter)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maBlitters.Set(x + iYOffset, pBlitter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CTileLayerCel::GetBlitter(size x, size y)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maBlitters.Get(x + iYOffset);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::GetImageDestBounds(CRectangle* pcRect)
{
	Ptr<CImageCel>	pCel;
	CSubImage*		pcSubImage;
	CRectangle		cBounding;
	CRectangle		cCelRect;
	bool			bFirst;
	int32			x, y;

	if (pcRect)
	{
		cBounding.Init();
		bFirst = true;
		for (y = 0; y < msMapSize.y; y++)
		{
			for (x = 0; x < msMapSize.x; x++)
			{
				pCel = GetTile(x, y);
				if (pCel)
				{
					pcSubImage = pCel->GetSubImage();
					if (bFirst)
					{
						pcSubImage->GetImageDestBounds(x * msCelSize.x, y * msCelSize.y, &cBounding);
						bFirst = false;
					}
					else
					{
						pcSubImage->GetImageDestBounds(x * msCelSize.x, y * msCelSize.y, &cCelRect);
						cBounding.GrowToContain(&cCelRect);
					}
				}
			}
		}
		pcRect->Init(&cBounding);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::GetFullDestBounds(CRectangle* pcRect)
{
	if ((msMapSize.x > 0) && (msCelSize.x > 0) && (msMapSize.y > 0) && (msCelSize.y > 0))
	{
		pcRect->Init(msMapSize.x* msCelSize.x, msMapSize.y* msCelSize.y);
		return true;
	}
	else
	{
		pcRect->Init();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTileLayerCel::CreateViewportImage(void)
{
	bool			bExists;
	size			uiWidth;
	size			uiHeight;
	size			uiPadding;
	CRectangle		cBoundingRect;
	Ptr<CImage>		pDestImage;

	bExists = GetFullDestBounds(&cBoundingRect);
	if (!bExists)
	{
		return NULL;
	}

	uiWidth = cBoundingRect.GetWidth();
	uiHeight = cBoundingRect.GetHeight();
	if (uiWidth % 8 != 0)
	{
		uiPadding = (8 - uiWidth % 8);
		uiWidth += uiPadding;
	}

	pDestImage = OMalloc<CImage>(uiWidth, uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pDestImage.IsNull())
	{
		return NULL;
	}

	pDestImage->Clear();
	return pDestImage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCelBlitterCache> CTileLayerCel::CreateBlitterCache(void)
{
	if (mpViewport)
	{
		return OMalloc<CImageCelBlitterCache>(mpViewport);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) { mpCache = pCache; }
void CTileLayerCel::SetViewport(Ptr<CImage> pViewport) { mpViewport = pViewport; }

