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
#include "StandardLib/Object.h"
#include "StandardLib/Array.h"
#include "MovableBlock.h"


class CTileMap;
class CMovableBlockType;
class CTileLayer : public CObject
{
CONSTRUCTABLE(CTileLayer);
DESTRUCTABLE(CTileLayer);
protected:
	CArrayMovableBlock		maTiles;
	CCharsImmutable			mszName;
	Ptr<CTileMap>			mpTileMap;
	bool					mbVisible;
	Ptr<CMovableBlockType>	mpTileType;

public:
	void				Init(char* szName, Ptr<CTileMap> pTileMap, Ptr<CMovableBlockType> pTileType);
	void 				Free(void);
	void				Class(void);

	bool				Save(CObjectWriter* pcFile);
	bool				Load(CObjectReader* pcFile);

	void				SetTile(int x, int y, Ptr<CMovableBlock> pTile);
	Ptr<CMovableBlock>	GetTile(size uiIndex);
	Ptr<CMovableBlock>	GetTile(int x, int y);
};


typedef CArray<CTileLayer>	CArrayTileLayer;


#endif // __TILE_LAYER_H__

