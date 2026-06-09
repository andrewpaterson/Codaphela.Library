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
#ifndef __TILE_LAYER_H__
#define __TILE_LAYER_H__
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "ImageCelBlitterCache.h"


class CTileLayer : public CObject
{
CONSTRUCTABLE(CTileLayer);
DESTRUCTABLE(CTileLayer);
protected:
	CPointer		mpTileMap;
	CChars			mszTileType;
	SInt32Vec2		msMapSize;		//The tile is indexed by x + y * msMapSize.x
	SInt32Vec2		msCelSize;
	SInt32Vec2		msPosition;		//Typically zero.

public:
			void	Init(CPointer pTileMap, const char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition);
			void 	Free(void);
			void	Class(void);

			bool	Save(CObjectWriter* pcFile);
			bool	Load(CObjectReader* pcFile);

			int32	GetLayerSizeX(void);
			int32	GetLayerSizeY(void);

	virtual	void	SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) =0;
	virtual	void	SetViewport(Ptr<CImage> pViewport) =0;

	virtual void	TileLayerAbstract(void) =0;
};


typedef CArray<CTileLayer>	CArrayTileLayer;


#endif // __TILE_LAYER_H__

