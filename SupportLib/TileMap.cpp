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
#include "TileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Init(void)
{
	PreInit();

	CMovableBlockMap::Init();
	msMapSize.Init(0, 0);
	msCelSize.Init(0, 0);
	maTileLayers.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Init(char* szName, int iCelSizeX, int iCelSizeY)
{
	PreInit();

	CMovableBlockMap::Init(szName);
	msMapSize.Init(0, 0);
	msCelSize.Init(iCelSizeX, iCelSizeY);
	maTileLayers.Init();

	PostInit();
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
void CTileMap::Free(void)
{
	CMovableBlockMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Class(void)
{
	CMovableBlockMap::Class();
	M_Embedded(maTileLayers);
	U_2Int32(msMapSize);
	U_2Int32(msCelSize);
}	


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileLayer> CTileMap::AddLayer(char* szName, Ptr<CMovableBlockType> pTileType)
{
	Ptr<CTileLayer> pLayer;

	pLayer = OMalloc<CTileLayer>(szName, this, pTileType);
	maTileLayers.Add(pLayer);
	return pLayer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::MovableBlockMapAbstract(void) {}
int CTileMap::GetMapSizeX(void) { return msMapSize.x; }
int CTileMap::GetMapSizeY(void) { return msMapSize.y; }


