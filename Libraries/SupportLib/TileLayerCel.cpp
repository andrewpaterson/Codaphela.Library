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
void CTileLayerCel::Init(Ptr<CTileMap> pTileMap, const char* szTileType, SSizeVec2 sMapSize, SSizeVec2 sCelSize, int iIdentifier, SInt32Vec2 sPosition)
{
	PreInit();

	size			y;
	STileLayerRow*	psRow;

	CTileLayer::Init(pTileMap, szTileType, sMapSize, sCelSize, iIdentifier, sPosition);

	mpCache = NULL;
	mpViewport = NULL;

	masRows.Init();
	for (y = 0; y < sMapSize.y; y++)
	{
		psRow = masRows.Add();
		psRow->uiFirst = 0;
		psRow->uiStart = 0;
		psRow->uiEnd = 0;
	}

	maTiles.Init();
	maBlitters.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Free(void)
{
	masRows.Kill();
	CTileLayer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Class(void)
{
	CTileLayer::Class();
	U_Data(CArrayTileLayerRow, masRows);
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
//
// The index in maTiles of the tile at x, y; MAX_SIZE when it is outside its
// row's run (and so has no cel).
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::TileIndex(size x, size y)
{
	STileLayerRow*	psRow;

	if (y >= msMapSize.y)
	{
		return MAX_SIZE;
	}

	psRow = masRows.Get(y);
	if ((x < psRow->uiStart) || (x >= psRow->uiEnd))
	{
		return MAX_SIZE;
	}
	return psRow->uiFirst + (x - psRow->uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
// Inserts uiCount NULL cels (and blitters) at uiIndex, in row y, and moves
// the rows after it along.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::InsertCels(size y, size uiIndex, size uiCount)
{
	size				i;
	Ptr<CImageCel>		pNoCel;
	Ptr<CImageBlitter>	pNoBlitter;
	bool				bBlitters;

	bBlitters = !maBlitters.IsEmpty();
	for (i = 0; i < uiCount; i++)
	{
		maTiles.Insert(uiIndex, pNoCel);
		if (bBlitters)
		{
			maBlitters.Insert(uiIndex, pNoBlitter);
		}
	}

	for (i = y + 1; i < msMapSize.y; i++)
	{
		masRows.Get(i)->uiFirst += uiCount;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the uiCount cels (and blitters) at uiIndex, in row y, and moves
// the rows after it back.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::RemoveCels(size y, size uiIndex, size uiCount)
{
	size	i;
	bool	bBlitters;

	bBlitters = !maBlitters.IsEmpty();
	for (i = 0; i < uiCount; i++)
	{
		maTiles.RemoveAt(uiIndex);
		if (bBlitters)
		{
			maBlitters.RemoveAt(uiIndex);
		}
	}

	for (i = y + 1; i < msMapSize.y; i++)
	{
		masRows.Get(i)->uiFirst -= uiCount;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Drops the empty tiles at the start and end of row y's run so that a run
// always starts and ends with a cel.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::TrimRow(size y)
{
	STileLayerRow*	psRow;
	size			uiLength;
	size			uiLeading;
	size			uiTrailing;

	psRow = masRows.Get(y);
	uiLength = psRow->uiEnd - psRow->uiStart;

	uiLeading = 0;
	while ((uiLeading < uiLength) && maTiles.Get(psRow->uiFirst + uiLeading).IsNull())
	{
		uiLeading++;
	}
	if (uiLeading == uiLength)
	{
		RemoveCels(y, psRow->uiFirst, uiLength);
		psRow->uiStart = 0;
		psRow->uiEnd = 0;
		return;
	}

	uiTrailing = 0;
	while (maTiles.Get(psRow->uiFirst + uiLength - 1 - uiTrailing).IsNull())
	{
		uiTrailing++;
	}

	if (uiTrailing > 0)
	{
		RemoveCels(y, psRow->uiFirst + uiLength - uiTrailing, uiTrailing);
		psRow->uiEnd -= uiTrailing;
	}
	if (uiLeading > 0)
	{
		RemoveCels(y, psRow->uiFirst, uiLeading);
		psRow->uiStart += uiLeading;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTile(size x, size y, Ptr<CImageCel> pTile)
{
	STileLayerRow*		psRow;
	size				uiIndex;
	Ptr<CImageBlitter>	pNoBlitter;

	if ((x >= msMapSize.x) || (y >= msMapSize.y))
	{
		return false;
	}

	psRow = masRows.Get(y);
	if (pTile.IsNull())
	{
		uiIndex = TileIndex(x, y);
		if (uiIndex != MAX_SIZE)
		{
			maTiles.Set(uiIndex, pTile);
			if (!maBlitters.IsEmpty())
			{
				maBlitters.Set(uiIndex, pNoBlitter);
			}
			TrimRow(y);
		}
		return true;
	}

	if (psRow->uiStart == psRow->uiEnd)
	{
		InsertCels(y, psRow->uiFirst, 1);
		psRow->uiStart = x;
		psRow->uiEnd = x + 1;
	}
	else if (x < psRow->uiStart)
	{
		InsertCels(y, psRow->uiFirst, psRow->uiStart - x);
		psRow->uiStart = x;
	}
	else if (x >= psRow->uiEnd)
	{
		InsertCels(y, psRow->uiFirst + (psRow->uiEnd - psRow->uiStart), x + 1 - psRow->uiEnd);
		psRow->uiEnd = x + 1;
	}

	return maTiles.Set(psRow->uiFirst + (x - psRow->uiStart), pTile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(size uiIndex)
{
	if (msMapSize.x == 0)
	{
		return NULL;
	}
	return GetTile(uiIndex % msMapSize.x, uiIndex / msMapSize.x);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(size x, size y)
{
	size	uiIndex;

	uiIndex = TileIndex(x, y);
	if (uiIndex == MAX_SIZE)
	{
		return NULL;
	}
	return maTiles.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The first x in row y with a cel (equal to GetRowEnd when it has none).
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::GetRowStart(size y)
{
	return masRows.Get(y)->uiStart;
}


//////////////////////////////////////////////////////////////////////////
//
// One past the last x in row y with a cel.
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::GetRowEnd(size y)
{
	return masRows.Get(y)->uiEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// The cels the rows' runs hold (including the empty tiles inside runs).
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::NumStoredTiles(void)
{
	return maTiles.NumElements();
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
		if (x < iMaxX)
		{
			iIndex = va_arg(vaMarker, int);  //Integer arguments smaller than int are promoted to int when passed through ...
		}
	}
	va_end(vaMarker);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every tile in row y.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::ClearRow(size y)
{
	STileLayerRow*	psRow;

	if (y >= msMapSize.y)
	{
		return;
	}

	psRow = masRows.Get(y);
	RemoveCels(y, psRow->uiFirst, psRow->uiEnd - psRow->uiStart);
	psRow->uiStart = 0;
	psRow->uiEnd = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every tile.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Clear(void)
{
	size			y;
	STileLayerRow*	psRow;

	maTiles.Clear();
	maBlitters.Clear();
	for (y = 0; y < msMapSize.y; y++)
	{
		psRow = masRows.Get(y);
		psRow->uiFirst = 0;
		psRow->uiStart = 0;
		psRow->uiEnd = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Grows or shrinks the grid.  Tiles in it keep their places; tiles that
// are no longer in it are removed.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::SetMapSize(SSizeVec2 sMapSize)
{
	size			y;
	size			uiFirst;
	STileLayerRow*	psRow;

	//Rows below the new bottom go, and their cels with them.
	for (y = sMapSize.y; y < msMapSize.y; y++)
	{
		ClearRow(y);
	}
	if (sMapSize.y < msMapSize.y)
	{
		masRows.RemoveRange(sMapSize.y, msMapSize.y);

		//RemoveCels moves the rows after the one it is in along, so it must only see the rows that are left.
		msMapSize.y = sMapSize.y;
	}

	//Columns right of the new right go.
	if (sMapSize.x < msMapSize.x)
	{
		for (y = 0; y < msMapSize.y; y++)
		{
			psRow = masRows.Get(y);
			if (psRow->uiEnd > sMapSize.x)
			{
				if (psRow->uiStart >= sMapSize.x)
				{
					ClearRow(y);
				}
				else
				{
					RemoveCels(y, psRow->uiFirst + (sMapSize.x - psRow->uiStart), psRow->uiEnd - sMapSize.x);
					psRow->uiEnd = sMapSize.x;
					TrimRow(y);
				}
			}
		}
	}

	//New rows at the bottom are empty; their cels would go after all the others.
	uiFirst = maTiles.NumElements();
	for (y = msMapSize.y; y < sMapSize.y; y++)
	{
		psRow = masRows.Add();
		psRow->uiFirst = uiFirst;
		psRow->uiStart = 0;
		psRow->uiEnd = 0;
	}
	msMapSize = sMapSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Draws every tile into a new image the size of the layer.  Its blitters
// are its own: the layer's blitters, cache and viewport are left as they
// are.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CTileLayerCel::WriteToImage(void)
{
	size 						x, y;
	Ptr<CImageCelBlitterCache>	pCache;
	Ptr<CImage>					pImage;
	Ptr<CImageBlitter>			pBlitter;
	Ptr<CImageCel>				pCel;
	STileLayerRow*				psRow;

	pImage = CreateViewportImage();
	if (pImage.IsNull())
	{
		return NULL;
	}
	pCache = OMalloc<CImageCelBlitterCache>(pImage);

	for (y = 0; y < msMapSize.y; y++)
	{
		psRow = masRows.Get(y);
		for (x = psRow->uiStart; x < psRow->uiEnd; x++)
		{
			pCel = maTiles.Get(psRow->uiFirst + (x - psRow->uiStart));
			if (pCel.IsNotNull())
			{
				pBlitter = pCache->CreateImageBlitter(pCel);
				if (pBlitter.IsNull() || !pBlitter->Blit((int32)x * (int32)msCelSize.x, (int32)y * (int32)msCelSize.y))
				{
					return NULL;
				}
			}
		}
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// A NULL blitter for each of maTiles.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::FillBlitters(void)
{
	size				i;
	size				uiNumTiles;
	Ptr<CImageBlitter>	pNoBlitter;

	maBlitters.Clear();
	uiNumTiles = maTiles.NumElements();
	for (i = 0; i < uiNumTiles; i++)
	{
		maBlitters.Add(pNoBlitter);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::CreateCelBlitters(void)
{
	Ptr<CImageCel>		pCel;
	Ptr<CImageBlitter>	pBlitter;
	size				uiNumTiles;
	size				i;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			FillBlitters();
			uiNumTiles = maTiles.NumElements();
			for (i = 0; i < uiNumTiles; i++)
			{
				pCel = maTiles.Get(i);
				if (pCel)
				{
					pBlitter = mpCache->CreateImageBlitter(pCel);
					if (pBlitter.IsNull())
					{
						return false;
					}
					maBlitters.Set(i, pBlitter);
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
bool CTileLayerCel::HasCelBlitters(void)
{
	size	uiNumTiles;
	size	i;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			uiNumTiles = maTiles.NumElements();
			if (maBlitters.NumElements() != uiNumTiles)
			{
				return false;
			}

			for (i = 0; i < uiNumTiles; i++)
			{
				if (maBlitters.Get(i).IsNull() && maTiles.Get(i).IsNotNull())
				{
					return false;
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
bool CTileLayerCel::UsesCelBlitters(void)
{
	return true;
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
//
// Only a tile inside its row's run can have a blitter; a NULL blitter is
// always set.
//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetBlitter(size x, size y, Ptr<CImageBlitter> pBlitter)
{
	size	uiIndex;

	uiIndex = TileIndex(x, y);
	if (uiIndex == MAX_SIZE)
	{
		return pBlitter.IsNull();
	}

	if (maBlitters.IsEmpty())
	{
		if (pBlitter.IsNull())
		{
			return true;
		}
		FillBlitters();
	}
	return maBlitters.Set(uiIndex, pBlitter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CTileLayerCel::GetBlitter(size x, size y)
{
	size	uiIndex;

	uiIndex = TileIndex(x, y);
	if ((uiIndex == MAX_SIZE) || maBlitters.IsEmpty())
	{
		return NULL;
	}
	return maBlitters.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::GetImageDestBounds(CInt32Rect* pcRect)
{
	Ptr<CImageCel>	pCel;
	CSubImage*		pcSubImage;
	CInt32Rect		cBounding;
	CInt32Rect		cCelRect;
	bool			bFirst;
	size 			x, y;
	STileLayerRow*	psRow;

	if (pcRect)
	{
		cBounding.Init();
		bFirst = true;
		for (y = 0; y < msMapSize.y; y++)
		{
			psRow = masRows.Get(y);
			for (x = psRow->uiStart; x < psRow->uiEnd; x++)
			{
				pCel = maTiles.Get(psRow->uiFirst + (x - psRow->uiStart));
				if (pCel)
				{
					pcSubImage = pCel->GetSubImage();
					if (bFirst)
					{
						pcSubImage->GetImageDestBounds((int32)x * (int32)msCelSize.x, (int32)y * (int32)msCelSize.y, &cBounding);
						bFirst = false;
					}
					else
					{
						pcSubImage->GetImageDestBounds((int32)x * (int32)msCelSize.x, (int32)y * (int32)msCelSize.y, &cCelRect);
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
bool CTileLayerCel::GetFullDestBounds(CInt32Rect* pcRect)
{
	if ((msMapSize.x > 0) && (msCelSize.x > 0) && (msMapSize.y > 0) && (msCelSize.y > 0))
	{
		pcRect->Init((int32)msMapSize.x * (int32)msCelSize.x, (int32)msMapSize.y * (int32)msCelSize.y);
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
	CInt32Rect		cBoundingRect;
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

	pDestImage = OMalloc<CImage>((uint32)uiWidth, (uint32)uiHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pDestImage.IsNull())
	{
		return NULL;
	}

	pDestImage->Black();
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
//
// Makes the blitters of the tiles in xStart..xStop, yStart..yStop match
// their cels: a new blitter where a tile has none or its cel changed, none
// where a tile has no cel.
//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::EnsureBlitters(int32 xStart, int32 yStart, int32 xStop, int32 yStop)
{
	int32				x, y;
	int32				xRowStart;
	int32				xRowStop;
	size				uiIndex;
	Ptr<CImageBlitter>	pBlitter;
	Ptr<CImageCel>		pCel;
	STileLayerRow*		psRow;

	if (maTiles.IsEmpty())
	{
		return true;
	}
	if (maBlitters.NumElements() != maTiles.NumElements())
	{
		FillBlitters();
	}

	if (yStart < 0)
	{
		yStart = 0;
	}
	if (yStop > (int32)msMapSize.y)
	{
		yStop = (int32)msMapSize.y;
	}
	for (y = yStart; y < yStop; y++)
	{
		psRow = masRows.Get(y);
		xRowStart = (xStart > (int32)psRow->uiStart) ? xStart : (int32)psRow->uiStart;
		xRowStop = (xStop < (int32)psRow->uiEnd) ? xStop : (int32)psRow->uiEnd;
		for (x = xRowStart; x < xRowStop; x++)
		{
			uiIndex = psRow->uiFirst + ((size)x - psRow->uiStart);
			pCel = maTiles.Get(uiIndex);
			pBlitter = maBlitters.Get(uiIndex);
			if (pCel.IsNotNull())
			{
				if (pBlitter.IsNull() || (pBlitter->GetCel() != pCel))
				{
					pBlitter = mpCache->CreateImageBlitter(pCel);
					if (pBlitter.IsNull())
					{
						return false;
					}
					maBlitters.Set(uiIndex, pBlitter);
				}
			}
			else if (pBlitter.IsNotNull())
			{
				pBlitter = NULL;
				maBlitters.Set(uiIndex, pBlitter);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters)
{
	uint32 				x, y;
	int32 				xStop, yStop;
	int32 				xStart, yStart;
	int32 				xSize, ySize;
	uint32				xRowStart;
	uint32				xRowStop;
	Ptr<CImageBlitter>	pBlitter;
	bool				bResult;
	int32				xOffset;
	int32				yOffset;
	STileLayerRow*		psRow;

	if (bEnsureBlitters)
	{
		if (maBlitters.IsEmpty() && !maTiles.IsEmpty())
		{
			mpCache = CreateBlitterCache();
			bResult = CreateCelBlitters();
			if (!bResult)
			{
				return false;
			}
		}
	}

	xSize = pcViewportRect->GetWidth() + 1;
	xStart = pcViewportRect->msTopLeft.x / (int32)msCelSize.x;
	xStop = xStart + xSize / (int32)msCelSize.x;
	if (xSize % (int32)msCelSize.x != 0)
	{
		xStop++;
	}
	if (xStart < 0)
	{
		xStart = 0;
	}
	if (xStop > (int32)msMapSize.x)
	{
		xStop = (int32)msMapSize.x;
	}

	ySize = pcViewportRect->GetHeight();
	yStart = pcViewportRect->msTopLeft.y / (int32)msCelSize.y;
	yStop = yStart + ySize / (int32)msCelSize.y;
	yStop++;
	if (ySize % (int32)msCelSize.y != 0)
	{
		yStop++;
	}
	if (yStart < 0)
	{
		yStart = 0;
	}
	if (yStop > (int32)msMapSize.y)
	{
		yStop = (int32)msMapSize.y;
	}

	if ((xStart >= xStop) || (yStart >= yStop))
	{
		return true;
	}

	if (bEnsureBlitters)
	{
		bResult = EnsureBlitters(xStart, yStart, xStop, yStop);
		if (!bResult)
		{
			return false;
		}
	}

	for (y = (uint32)yStart; y < (uint32)yStop; y++)
	{
		psRow = masRows.Get(y);
		xRowStart = ((uint32)xStart > psRow->uiStart) ? (uint32)xStart : (uint32)psRow->uiStart;
		xRowStop = ((uint32)xStop < psRow->uiEnd) ? (uint32)xStop : (uint32)psRow->uiEnd;
		for (x = xRowStart; x < xRowStop; x++)
		{
			pBlitter = GetBlitter(x, y);
			if (pBlitter.IsNotNull())
			{
				xOffset = (int32)x * (int32)msCelSize.x;
				yOffset = (int32)y * (int32)msCelSize.y;
				bResult = pBlitter->Blit(xOffset - pcViewportRect->msTopLeft.x, yOffset - pcViewportRect->msTopLeft.y);
				if (!bResult)
				{
					return false;
				}
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) { mpCache = pCache; }
void CTileLayerCel::SetViewport(Ptr<CImage> pViewport) { mpViewport = pViewport; }

