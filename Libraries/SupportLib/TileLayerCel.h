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
#ifndef __TILE_LAYER_CEL_H__
#define __TILE_LAYER_CEL_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapPrimitiveTemplate.h"
#include "ImageCel.h"
#include "ImageBlitter.h"
#include "TileMap.h"
#include "TileLayer.h"


//A row keeps only the run of tiles from its first cel to its last; the tiles outside it are empty and not stored.  The
//runs are stored one after the other, top row first, in mauiTiles.
struct STileLayerRow
{
	size	uiFirst;	//The index in mauiTiles of the tile at uiStart (where the row's tiles go when it has none).
	size	uiStart;	//The first x with a cel.
	size	uiEnd;		//One past the last x with a cel.  uiStart == uiEnd when the row has no cels.
};


typedef CArrayTemplate<STileLayerRow>				CArrayTileLayerRow;
typedef CArrayTemplate<uint32>						CArrayTileLayerTile;
typedef CMapPrimitiveTemplate<CImageCel*, size>		CMapTileLayerCelIndex;


//A grid of cels.  A tile is plain data, the index (plus one, 0 for none) of its cel in maCels, which holds each cel
//once however many tiles show it, so setting a tile doesn't change what points to the cel.  Each cel has one blitter.
class CImageCelBlitterCache;
class CTileLayerCel : public CTileLayer
{
CONSTRUCTABLE(CTileLayerCel);
DESTRUCTABLE(CTileLayerCel);
protected:
	CArrayTileLayerRow			masRows;
	CArrayTileLayerTile			mauiTiles;		//Each row's run of tiles: a cel's index plus one, or 0 for an empty tile.
	CArrayImageCel				maCels;			//Each cel the tiles show, once.
	CArrayImageBlitter			maBlitters;		//The blitter of the cel at the same index in maCels, NULL until made.
	CMapTileLayerCelIndex		mcCelIndices;	//A cel's index in maCels.
	Ptr<CImageCelBlitterCache>	mpCache;
	Ptr<CImage>					mpViewport;

public:
	void						Init(Ptr<CTileMap> pTileMap, const char* szTileType, SSizeVec2 sMapSize, SSizeVec2 sCelSize, int iIdentifier, SInt32Vec2 sPosition);
	void 						Free(void) override;
	void						Class(void) override;

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) override;
	void						SetViewport(Ptr<CImage> pViewport) override;

	bool						SetTile(size x, size y, Ptr<CImageCel> pTile);
	Ptr<CImageCel>				GetTile(size uiIndex);
	Ptr<CImageCel>				GetTile(size x, size y);
	size						GetRowStart(size y);
	size						GetRowEnd(size y);
	size						NumStoredTiles(void);
	size						NumCels(void);

	bool						SetTiles(size x, size y, Ptr<CArrayImageCel> paCels, size uiIndices ...);
	void						ClearRow(size y);
	void						Clear(void);
	void						SetMapSize(SSizeVec2 sMapSize);

	bool						BlitClipped(CInt32Rect* pcViewportRect, CInt32Rect* pcClip, bool bEnsureBlitters) override;

	bool						GetImageDestBounds(CInt32Rect* pcRect);
	bool						GetFullDestBounds(CInt32Rect* pcRect);

	Ptr<CImage>					WriteToImage(void);
	Ptr<CImage>					CreateViewportImage(void);
	Ptr<CImageCelBlitterCache>	CreateBlitterCache(void);

	bool						EnsureBlitters(int32 x, int32 y, int32 xStop, int32 yStop);
	bool						CreateCelBlitters(void) override;
	bool						HasCelBlitters(void) override;
	bool						UsesCelBlitters(void) override;

	Ptr<CImageBlitter>			GetBlitter(size x, size y);
	void						ClearCelBlitters(void) override;

	void						TileLayerAbstract(void) override {}

protected:
	size						TileIndex(size x, size y);
	uint32						AddCel(Ptr<CImageCel> pCel);
	bool						CreateCelBlitter(size uiCel);
	void						InsertTiles(size y, size uiIndex, size uiCount);
	void						RemoveTiles(size y, size uiIndex, size uiCount);
	void						TrimRow(size y);
};


#endif // __TILE_LAYER_CEL_H__

