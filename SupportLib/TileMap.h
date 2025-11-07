/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __TILE_MAP_H__
#define __TILE_MAP_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayType.h"
#include "BaseLib/Int2.h"
#include "TileLayer.h"
#include "MovableBlockType.h"
#include "MovableBlock.h"


class CTileMap : public CUnknown
{
CONSTRUCTABLE(CTileMap);
protected:
	CArrayTileLayer		macTileLayers;	//Layers are not multiple graphics layers.  All the layers together make up the complete information about a single tile.  E.g. an ImageCelTile and a BooleanTile.
	CCharsImmutable		mszName;
	SInt2				msMapSize;		//The tile is indexed by x + y * msMapSize.x
	SInt2				msCelSize;

public:
	void			Init(void);
	void 			Init(char* szName, int iCelSizeX, int iCelSizeY);
	void 			Kill(void);

	void			SetMapSize(int iMapSizeX, int iMapSizeY);
	CTileLayer*		AddLayer(char* szName, CMovableBlockType* pcTileType);
	int				GetMapSizeX(void);
	int				GetMapSizeY(void);
};


typedef CArrayType<CTileMap>	CArrayTileMap;


#endif // __TILE_MAP_H__

