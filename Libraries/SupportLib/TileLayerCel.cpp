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
#include "BaseLib/PointerFunctions.h"
#include "StandardLib/Objects.h"
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

	mauiTiles.Init();
	maCels.Init();
	maBlitters.Init();
	mcCelIndices.Init(&ComparePtrPtr);
	muiCelVersion = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Free(void)
{
	mcCelIndices.Kill();
	mauiTiles.Kill();
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
	U_Unknown(CArrayTileLayerRow, masRows);
	U_Unknown(CArrayTileLayerTile, mauiTiles);
	M_Embedded(maCels);
	M_Embedded(maBlitters);
	U_Unknown(CMapTileLayerCelIndex, mcCelIndices);
	U_Size(muiCelVersion);
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
// The index in mauiTiles of the tile at x, y; MAX_SIZE when it is
// outside its row's run (and so empty).
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
// The tile value of pCel: its index in maCels plus one, adding it if it
// isn't there yet; 0, an empty tile, for no cel.  It is pCel's until the
// layer is cleared (GetCelVersion).
//
//////////////////////////////////////////////////////////////////////////
uint32 CTileLayerCel::AddCel(Ptr<CImageCel> pCel)
{
	CImageCel*			pcCel;
	size*				puiCel;
	size				uiCel;
	Ptr<CImageBlitter>	pNoBlitter;

	pcCel = (CImageCel*)pCel.Object();
	if (pcCel == NULL)
	{
		return 0;
	}
	puiCel = mcCelIndices.Get(pcCel);
	if (puiCel)
	{
		return (uint32)(*puiCel + 1);
	}

	uiCel = maCels.NumElements();
	maCels.Add(pCel);
	maBlitters.Add(pNoBlitter);
	mcCelIndices.Put(pcCel, &uiCel);
	return (uint32)(uiCel + 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Changes each time the layer forgets its cels (Clear): the tile values
// AddCel gave before are no longer theirs.
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::GetCelVersion(void)
{
	return muiCelVersion;
}


//////////////////////////////////////////////////////////////////////////
//
// Inserts uiCount empty tiles at uiIndex, in row y, and moves the rows
// after it along.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::InsertTiles(size y, size uiIndex, size uiCount)
{
	size			i;
	STileLayerRow*	psRows;

	mauiTiles.InsertNumAt(uiCount, uiIndex);
	memset(mauiTiles.Get(uiIndex), 0, uiCount * sizeof(uint32));

	psRows = masRows.GetData();
	for (i = y + 1; i < msMapSize.y; i++)
	{
		psRows[i].uiFirst += uiCount;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the uiCount tiles at uiIndex, in row y, and moves the rows
// after it back.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::RemoveTiles(size y, size uiIndex, size uiCount)
{
	size			i;
	STileLayerRow*	psRows;

	if (uiCount == 0)
	{
		return;
	}
	mauiTiles.RemoveRange(uiIndex, uiIndex + uiCount);

	psRows = masRows.GetData();
	for (i = y + 1; i < msMapSize.y; i++)
	{
		psRows[i].uiFirst -= uiCount;
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
	while ((uiLeading < uiLength) && (*mauiTiles.Get(psRow->uiFirst + uiLeading) == 0))
	{
		uiLeading++;
	}
	if (uiLeading == uiLength)
	{
		RemoveTiles(y, psRow->uiFirst, uiLength);
		psRow->uiStart = 0;
		psRow->uiEnd = 0;
		return;
	}

	uiTrailing = 0;
	while (*mauiTiles.Get(psRow->uiFirst + uiLength - 1 - uiTrailing) == 0)
	{
		uiTrailing++;
	}

	if (uiTrailing > 0)
	{
		RemoveTiles(y, psRow->uiFirst + uiLength - uiTrailing, uiTrailing);
		psRow->uiEnd -= uiTrailing;
	}
	if (uiLeading > 0)
	{
		RemoveTiles(y, psRow->uiFirst, uiLeading);
		psRow->uiStart += uiLeading;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTile(size x, size y, Ptr<CImageCel> pTile)
{
	STileLayerRow*	psRow;
	size			uiIndex;

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
			*mauiTiles.Get(uiIndex) = 0;
			TrimRow(y);
		}
		return true;
	}

	if (psRow->uiStart == psRow->uiEnd)
	{
		InsertTiles(y, psRow->uiFirst, 1);
		psRow->uiStart = x;
		psRow->uiEnd = x + 1;
	}
	else if (x < psRow->uiStart)
	{
		InsertTiles(y, psRow->uiFirst, psRow->uiStart - x);
		psRow->uiStart = x;
	}
	else if (x >= psRow->uiEnd)
	{
		InsertTiles(y, psRow->uiFirst + (psRow->uiEnd - psRow->uiStart), x + 1 - psRow->uiEnd);
		psRow->uiEnd = x + 1;
	}

	*mauiTiles.Get(psRow->uiFirst + (x - psRow->uiStart)) = AddCel(pTile);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The run of the uiLength tiles at auiTiles from their first cel to their
// last (uiStart == uiEnd == 0 when they have none).
//
//////////////////////////////////////////////////////////////////////////
static void FindTileRun(uint32* auiTiles, size uiLength, size* puiStart, size* puiEnd)
{
	size	uiStart;
	size	uiEnd;

	uiStart = 0;
	while ((uiStart < uiLength) && (auiTiles[uiStart] == 0))
	{
		uiStart++;
	}
	if (uiStart == uiLength)
	{
		*puiStart = 0;
		*puiEnd = 0;
		return;
	}

	uiEnd = uiLength;
	while (auiTiles[uiEnd - 1] == 0)
	{
		uiEnd--;
	}
	*puiStart = uiStart;
	*puiEnd = uiEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the tiles of the uiNumRows rows from y, all at once: row
// y + i takes the auiRowLengths[i] tile values, from column 0, that follow
// the rows before it in auiTiles.  A value is a cel's index plus one, as
// AddCel gives it, or 0 for an empty tile.  Tiles past the right are cut
// off.  False if anything with a cel was cut off; false, changing nothing,
// if the rows go past the bottom or a value isn't one of the layer's.
//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetRowTiles(size y, size uiNumRows, uint32* auiTiles, size* auiRowLengths)
{
	size			uiRow;
	size			uiOffset;
	size			uiLength;
	size			uiStart;
	size			uiEnd;
	size			uiNumTiles;
	size			uiOldFirst;
	size			uiOldEnd;
	size			uiFirst;
	size			uiNumCels;
	size			i;
	STileLayerRow*	psRows;
	STileLayerRow*	psRow;
	uint32*			puiTiles;
	bool			bFitted;

	if ((y > msMapSize.y) || (uiNumRows > msMapSize.y - y))
	{
		return false;
	}
	if (uiNumRows == 0)
	{
		return true;
	}

	//Checked before anything changes, and the tiles the runs will hold counted.
	uiNumCels = maCels.NumElements();
	bFitted = true;
	uiNumTiles = 0;
	uiOffset = 0;
	for (uiRow = 0; uiRow < uiNumRows; uiRow++)
	{
		uiLength = auiRowLengths[uiRow];
		for (i = 0; i < uiLength; i++)
		{
			if (auiTiles[uiOffset + i] > uiNumCels)
			{
				return false;
			}
			if ((i >= msMapSize.x) && (auiTiles[uiOffset + i] != 0))
			{
				bFitted = false;
			}
		}
		FindTileRun(&auiTiles[uiOffset], (uiLength < msMapSize.x) ? uiLength : msMapSize.x, &uiStart, &uiEnd);
		uiNumTiles += uiEnd - uiStart;
		uiOffset += uiLength;
	}

	//The old runs of the rows make way for the new.
	psRows = masRows.GetData();
	uiOldFirst = psRows[y].uiFirst;
	psRow = &psRows[y + uiNumRows - 1];
	uiOldEnd = psRow->uiFirst + (psRow->uiEnd - psRow->uiStart);
	if (uiNumTiles > uiOldEnd - uiOldFirst)
	{
		mauiTiles.InsertNumAt(uiNumTiles - (uiOldEnd - uiOldFirst), uiOldEnd);
	}
	else if (uiNumTiles < uiOldEnd - uiOldFirst)
	{
		mauiTiles.RemoveRange(uiOldFirst + uiNumTiles, uiOldEnd);
	}

	puiTiles = mauiTiles.GetData();
	uiFirst = uiOldFirst;
	uiOffset = 0;
	for (uiRow = 0; uiRow < uiNumRows; uiRow++)
	{
		uiLength = auiRowLengths[uiRow];
		FindTileRun(&auiTiles[uiOffset], (uiLength < msMapSize.x) ? uiLength : msMapSize.x, &uiStart, &uiEnd);
		psRow = &psRows[y + uiRow];
		psRow->uiFirst = uiFirst;
		psRow->uiStart = uiStart;
		psRow->uiEnd = uiEnd;
		if (uiEnd > uiStart)
		{
			memcpy(&puiTiles[uiFirst], &auiTiles[uiOffset + uiStart], (uiEnd - uiStart) * sizeof(uint32));
		}
		uiFirst += uiEnd - uiStart;
		uiOffset += uiLength;
	}

	//The rows after them move along (or back).
	for (uiRow = y + uiNumRows; uiRow < msMapSize.y; uiRow++)
	{
		psRows[uiRow].uiFirst = psRows[uiRow].uiFirst - uiOldEnd + uiFirst;
	}
	return bFitted;
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
	uint32	uiTile;

	uiIndex = TileIndex(x, y);
	if (uiIndex == MAX_SIZE)
	{
		return NULL;
	}
	uiTile = *mauiTiles.Get(uiIndex);
	if (uiTile == 0)
	{
		return NULL;
	}
	return maCels.Get(uiTile - 1);
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
// The tiles the rows' runs hold (including the empty tiles inside runs).
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::NumStoredTiles(void)
{
	return mauiTiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The different cels the tiles have shown since the layer was cleared.
//
//////////////////////////////////////////////////////////////////////////
size CTileLayerCel::NumCels(void)
{
	return maCels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::SetTiles(size x, size y, Ptr<CArrayImageCel> paCels, size uiIndices ...)
{
	va_list			vaMarker;
	size			uiIndex;
	size			uiNumCels;
	size			uiMaxX;
	Ptr<CImageCel>	pCel;
	bool			bResult;

	uiNumCels = paCels->NumElements();
	uiMaxX = msMapSize.x;
	bResult = true;

	va_start(vaMarker, uiIndices);
	uiIndex = uiIndices;
	while ((uiIndex < uiNumCels) && (x < uiMaxX))
	{
		pCel = paCels->Get(uiIndex);
		bResult &= pCel.IsNotNull();
		bResult &= SetTile(x, y, pCel);
		x++;
		if (x < uiMaxX)
		{
			uiIndex = va_arg(vaMarker, int);  //Integer arguments smaller than int are promoted to int when passed through ...
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
	RemoveTiles(y, psRow->uiFirst, psRow->uiEnd - psRow->uiStart);
	psRow->uiStart = 0;
	psRow->uiEnd = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every tile and forgets the cels they showed.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Clear(void)
{
	size			y;
	STileLayerRow*	psRow;

	mauiTiles.ReInit();
	maCels.Clear();
	maBlitters.Clear();
	mcCelIndices.ReInit();
	muiCelVersion++;
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

	//Rows below the new bottom go, and their tiles with them.
	for (y = sMapSize.y; y < msMapSize.y; y++)
	{
		ClearRow(y);
	}
	if (sMapSize.y < msMapSize.y)
	{
		masRows.RemoveRange(sMapSize.y, msMapSize.y);

		//RemoveTiles moves the rows after the one it is in along, so it must only see the rows that are left.
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
					RemoveTiles(y, psRow->uiFirst + (sMapSize.x - psRow->uiStart), psRow->uiEnd - sMapSize.x);
					psRow->uiEnd = sMapSize.x;
					TrimRow(y);
				}
			}
		}
	}

	//New rows at the bottom are empty; their tiles would go after all the others.
	uiFirst = mauiTiles.NumElements();
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
	STileLayerRow*				psRow;
	uint32						uiTile;

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
			uiTile = *mauiTiles.Get(psRow->uiFirst + (x - psRow->uiStart));
			if (uiTile != 0)
			{
				pBlitter = pCache->CreateImageBlitter(maCels.Get(uiTile - 1));
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
// Makes cel uiCel's blitter for the layer's viewport.
//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::CreateCelBlitter(size uiCel)
{
	Ptr<CImageBlitter>	pBlitter;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			pBlitter = mpCache->CreateImageBlitter(maCels.Get(uiCel));
			if (pBlitter.IsNull())
			{
				return false;
			}
			maBlitters.Set(uiCel, pBlitter);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::CreateCelBlitters(void)
{
	size	uiCel;

	if (mpCache && mpViewport)
	{
		for (uiCel = 0; uiCel < maCels.NumElements(); uiCel++)
		{
			if (!CreateCelBlitter(uiCel))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::HasCelBlitters(void)
{
	size	uiCel;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			for (uiCel = 0; uiCel < maBlitters.NumElements(); uiCel++)
			{
				if (maBlitters.UnsafeGetObject(uiCel) == NULL)
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
//
// The blitters are made again the next time they are needed.
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::ClearCelBlitters(void)
{
	size				uiCel;
	Ptr<CImageBlitter>	pNoBlitter;

	for (uiCel = 0; uiCel < maBlitters.NumElements(); uiCel++)
	{
		maBlitters.Set(uiCel, pNoBlitter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The blitter of the tile's cel, NULL for an empty tile or a cel without
// one yet.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageBlitter> CTileLayerCel::GetBlitter(size x, size y)
{
	size	uiIndex;
	uint32	uiTile;

	uiIndex = TileIndex(x, y);
	if (uiIndex == MAX_SIZE)
	{
		return NULL;
	}
	uiTile = *mauiTiles.Get(uiIndex);
	if (uiTile == 0)
	{
		return NULL;
	}
	return maBlitters.Get(uiTile - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::GetImageDestBounds(CInt32Rect* pcRect)
{
	CImageCel*		pcCel;
	CSubImage*		pcSubImage;
	CInt32Rect		cBounding;
	CInt32Rect		cCelRect;
	bool			bFirst;
	size 			x, y;
	STileLayerRow*	psRow;
	uint32			uiTile;

	if (pcRect)
	{
		cBounding.Init();
		bFirst = true;
		for (y = 0; y < msMapSize.y; y++)
		{
			psRow = masRows.Get(y);
			for (x = psRow->uiStart; x < psRow->uiEnd; x++)
			{
				uiTile = *mauiTiles.Get(psRow->uiFirst + (x - psRow->uiStart));
				if (uiTile != 0)
				{
					pcCel = (CImageCel*)maCels.UnsafeGetObject(uiTile - 1);
					pcSubImage = pcCel->GetSubImage();
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
// Makes the blitters of the cels the tiles in xStart..xStop,
// yStart..yStop show, where they haven't one.
//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::EnsureBlitters(int32 xStart, int32 yStart, int32 xStop, int32 yStop)
{
	int32			x, y;
	int32			xRowStart;
	int32			xRowStop;
	STileLayerRow*	psRow;
	uint32			uiTile;

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
			uiTile = *mauiTiles.Get(psRow->uiFirst + ((size)x - psRow->uiStart));
			if ((uiTile != 0) && (maBlitters.UnsafeGetObject(uiTile - 1) == NULL))
			{
				if (!CreateCelBlitter(uiTile - 1))
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
// The tiles in pcViewportRect (map pixels, right and bottom exclusive)	//
// blitted with its top left at the viewport's, cut to pcClip (map		//
// pixels, NULL for none).												//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::BlitClipped(CInt32Rect* pcViewportRect, CInt32Rect* pcClip, bool bEnsureBlitters)
{
	uint32 				x;
	uint32 				y;
	CInt32Rect			cDraw;
	CInt32Rect			cBlitterClip;
	CInt32Rect*			pcBlitterClip;
	int32 				xStart;
	int32 				yStart;
	int32 				xStop;
	int32 				yStop;
	int32				iCelWidth;
	int32				iCelHeight;
	uint32				xRowStart;
	uint32				xRowStop;
	CImageBlitter*		pcBlitter;
	bool				bResult;
	STileLayerRow*		psRow;
	uint32				uiTile;

	if (bEnsureBlitters && mpCache.IsNull() && !maCels.IsEmpty())
	{
		mpCache = CreateBlitterCache();
	}

	cDraw.Init(pcViewportRect);
	pcBlitterClip = NULL;
	if (pcClip)
	{
		//The blitters clip in the viewport's pixels.
		cDraw.ClipTo(pcClip);
		cBlitterClip.Init(cDraw.msTopLeft.x - pcViewportRect->msTopLeft.x, cDraw.msTopLeft.y - pcViewportRect->msTopLeft.y, cDraw.msBottomRight.x - pcViewportRect->msTopLeft.x, cDraw.msBottomRight.y - pcViewportRect->msTopLeft.y);
		pcBlitterClip = &cBlitterClip;
	}
	if ((cDraw.GetWidth() <= 0) || (cDraw.GetHeight() <= 0))
	{
		return true;
	}

	//The cells any of whose pixels are in the drawn part.
	iCelWidth = (int32)msCelSize.x;
	iCelHeight = (int32)msCelSize.y;
	xStart = (cDraw.msTopLeft.x < 0) ? 0 : cDraw.msTopLeft.x / iCelWidth;
	yStart = (cDraw.msTopLeft.y < 0) ? 0 : cDraw.msTopLeft.y / iCelHeight;
	xStop = (cDraw.msBottomRight.x <= 0) ? 0 : (cDraw.msBottomRight.x + iCelWidth - 1) / iCelWidth;
	yStop = (cDraw.msBottomRight.y <= 0) ? 0 : (cDraw.msBottomRight.y + iCelHeight - 1) / iCelHeight;
	if (xStop > (int32)msMapSize.x)
	{
		xStop = (int32)msMapSize.x;
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
			uiTile = *mauiTiles.Get(psRow->uiFirst + (x - psRow->uiStart));
			if (uiTile != 0)
			{
				pcBlitter = (CImageBlitter*)maBlitters.UnsafeGetObject(uiTile - 1);
				if (pcBlitter != NULL)
				{
					bResult = pcBlitter->Blit((int32)x * iCelWidth - pcViewportRect->msTopLeft.x, (int32)y * iCelHeight - pcViewportRect->msTopLeft.y, pcBlitterClip);
					if (!bResult)
					{
						return false;
					}
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

