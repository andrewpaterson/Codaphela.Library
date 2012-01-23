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
#include "TileLayer.h"
#include "Tile.h"


class CTileWorld;
class CTileMap : public CUnknown
{
BASE_FUNCTIONS(CTileMap);
public:
	CArrayTileLayer		macTileLayers;
	CChars				mszName;
	SInt2				msMapSize;
	SInt2				msCelSize;
	CTileWorld*			mpcWorld;

	void			Init(void);
	void 			Init(char* szName, int iCelSizeX, int iCelSizeY, CTileWorld* pcWorld);
	void 			Kill(void);

	void			SetMapSize(int iMapSizeX, int iMapSizeY);
	CTileLayer*		AddLayer(char* szName, CTileType* pcTileType);
};


typedef CArrayType<CTileMap>	CArrayTileMap;


#endif // __TILE_MAP_H__

