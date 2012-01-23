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
#include "TileWorld.h"
#include "TileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Init(void)
{
	mszName.Init();
	msMapSize.Init(0, 0);
	msCelSize.Init(0, 0);
	macTileLayers.Init();
	mpcWorld = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Init(char* szName, int iCelSizeX, int iCelSizeY, CTileWorld* pcWorld)
{
	mpcWorld = pcWorld;
	mszName.Init(szName);
	msMapSize.Init(0, 0);
	msCelSize.Init(iCelSizeX, iCelSizeY);
	macTileLayers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::SetMapSize(int iMapSizeX, int iMapSizeY)
{
	msMapSize.Init(iMapSizeX, iMapSizeY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Kill(void)
{
	macTileLayers.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileLayer* CTileMap::AddLayer(char* szName, CTileType* pcTileType)
{
	CTileLayer* pcLayer;

	pcLayer = macTileLayers.Add();
	pcLayer->Init(szName, this, pcTileType);
	return pcLayer;
}

