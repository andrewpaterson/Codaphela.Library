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
#include "ImageCel.h"
#include "ImageBlitter.h"
#include "TileMap.h"
#include "TileLayer.h"


class CImageCelBlitterCache;
class CTileLayerCel : public CTileLayer
{
CONSTRUCTABLE(CTileLayerCel);
DESTRUCTABLE(CTileLayerCel);
protected:
	CArrayImageCel				maTiles;
	CArrayImageBlitter			maBlitters;
	Ptr<CImageCelBlitterCache>	mpCache;
	Ptr<CImage>					mpViewport;

public:
	void						Init(Ptr<CTileMap> pTileMap, const char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition);
	void 						Free(void);
	void						Class(void);

	bool						Save(CObjectWriter* pcFile);
	bool						Load(CObjectReader* pcFile);

	void						SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) override;
	void						SetViewport(Ptr<CImage> pViewport) override;

	bool						SetTile(size x, size y, Ptr<CImageCel> pTile);
	Ptr<CImageCel>				GetTile(size uiIndex);
	Ptr<CImageCel>				GetTile(size x, size y);

	bool						SetTiles(size x, size y, Ptr<CArrayImageCel> paCels, size uiIndices ...);

	bool						Blit(CRectangle* pcViewportRect) override;

	bool						GetImageDestBounds(CRectangle* pcRect);
	bool						GetFullDestBounds(CRectangle* pcRect);

	Ptr<CImage>					WriteToImage(void);
	Ptr<CImage>					CreateViewportImage(void);
	Ptr<CImageCelBlitterCache>	CreateBlitterCache(void);

	bool						CreateCelBlitters(void);
	bool						SetBlitter(size x, size y, Ptr<CImageBlitter> pBlitter);
	Ptr<CImageBlitter>			GetBlitter(size x, size y);
	void						ClearCelBlitters(void);

	void						TileLayerAbstract(void) override {}
};


typedef CArray<CTileLayerCel>	CArrayTileImageCel;


#endif // __TILE_LAYER_CEL_H__

