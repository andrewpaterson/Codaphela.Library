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
#include "TileLayerBoolean.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerBoolean::Init(Ptr<CTileMap> pTileMap, const char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition)
{
	PreInit();

	CTileLayer::Init(pTileMap, szTileType, sMapSize, sCelSize, sPosition);
	macBools.Init();
	macBools.GrowTo(sMapSize.x * sMapSize.y);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerBoolean::Free(void)
{
	macBools.Kill();
	CTileLayer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerBoolean::Class(void)
{
	CTileLayer::Class();
	U_Data(CArrayBit, macBools);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerBoolean::Load(CObjectReader* pcFile)
{
	return CTileLayer::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerBoolean::Save(CObjectWriter* pcFile)
{
	return CTileLayer::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerBoolean::SetTile(int x, int y, bool bValue)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	if (iYOffset < macBools.NumElements())
	{
		macBools.Set(x + iYOffset, bValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerBoolean::GetTile(size uiIndex)
{
	return macBools.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerBoolean::GetTile(int x, int y)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	if (iYOffset < macBools.NumElements())
	{
		return macBools.Get(x + iYOffset);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerBoolean::SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) {}
void CTileLayerBoolean::SetViewport(Ptr<CImage> pViewport) {}

