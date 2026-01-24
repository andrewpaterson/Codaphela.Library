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
#ifndef __TILE_LAYER_H__
#define __TILE_LAYER_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayUnknownTemplate.h"
#include "MovableBlock.h"


class CTileMap;
class CMovableBlockType;
class CTileLayer : public CUnknown
{
CONSTRUCTABLE(CTileLayer);
protected:
	CArrayUnknown		macTiles;
	CCharsImmutable		mszname;
	CTileMap*			mpcTileMap;
	bool				mbVisible;
	CMovableBlockType*		mpcTileType;

public:
	void			Init(char* szName, CTileMap* pcTileMap, CMovableBlockType*	pcTileType);
	void			Kill(void);

	void			SetTile(int x, int y, CMovableBlock* pcTile);
	CMovableBlock*	GetTile(size uiIndex);
	CMovableBlock*	GetTile(int x, int y);
};


typedef CArrayUnknownTemplate<CTileLayer>	CArrayTileLayer;


#endif // __TILE_LAYER_H__

