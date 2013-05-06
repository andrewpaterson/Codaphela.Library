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
#include "TileType.h"
#include "TileMap.h"
#include "TileLayer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayer::Init(char* szName, CTileMap* pcTileMap, CTileType*	pcTileType)
{
	int			i;
	CTile*		pcNull;
	int			iSize;

	macTiles.Init(32);
	macTiles.KillElements(FALSE);
	mszname.Init(szName);
	mpcTileMap = pcTileMap;
	mbVisible = TRUE;
	mpcTileType = pcTileType;

	pcNull = mpcTileType->GetNull();
	iSize = pcTileMap->msMapSize.x * pcTileMap->msMapSize.y;
	for (i = 0; i < iSize; i++)
	{
		macTiles.Add(pcNull);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayer::Kill(void)
{
	mszname.Kill();
	macTiles.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayer::Set(int x, int y, CTile* pcTile)
{
	macTiles.Set(x + y * mpcTileMap->msMapSize.x, pcTile);
}

