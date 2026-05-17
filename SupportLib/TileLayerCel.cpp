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
#include "TileLayerCel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Init(Ptr<CTileMap> pTileMap, char* szTileType, SInt32Vec2 sMapSize, SInt32Vec2 sCelSize, SInt32Vec2 sPosition)
{
	PreInit();

	int			i;
	int			iSize;

	CTileLayer::Init(pTileMap, szTileType, sMapSize, sCelSize, sPosition);

	maTiles.Init();

	iSize = sMapSize.x * sMapSize.y;
	for (i = 0; i < iSize; i++)
	{
		maTiles.Add(NULL);
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Free(void)
{
	CTileLayer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::Class(void)
{
	CTileLayer::Class();
	M_Embedded(maTiles);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::Load(CObjectReader* pcFile)
{
	return CTileLayer::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileLayerCel::Save(CObjectWriter* pcFile)
{
	return CTileLayer::Save(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTileLayerCel::SetTile(int x, int y, Ptr<CImageCel> pTile)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	maTiles.Set(x + iYOffset, pTile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(size uiIndex)
{
	return maTiles.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTileLayerCel::GetTile(int x, int y)
{
	size	iYOffset;

	iYOffset = y * msMapSize.x;
	return maTiles.Get(x + iYOffset);
}

