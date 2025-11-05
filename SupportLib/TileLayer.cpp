/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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

	macTiles.Init();
	macTiles.KillElements(false);
	mszname.Init(szName);
	mpcTileMap = pcTileMap;
	mbVisible = true;
	mpcTileType = pcTileType;

	pcNull = mpcTileType->GetNull();
	iSize = pcTileMap->GetMapSizeX() * pcTileMap->GetMapSizeY();
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
void CTileLayer::SetTile(int x, int y, CTile* pcTile)
{
	int		iYOffset;

	iYOffset = y * mpcTileMap->GetMapSizeX();
	macTiles.Set(x + iYOffset, pcTile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTile* CTileLayer::GetTile(size uiIndex)
{
	return mpcTileType->Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTile* CTileLayer::GetTile(int x, int y)
{
	int		iYOffset;

	iYOffset = y * mpcTileMap->GetMapSizeX();
	return mpcTileType->Get(x + iYOffset);
}


