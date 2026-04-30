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
#include "StandardLib/Objects.h"
#include "MovableBlockBoolean.h"
#include "MovableBlockImageCel.h"
#include "Maps.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Init(Ptr<CMapsContext> pContext)
{
	mpContext = pContext;
	maMaps.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Free(void)
{
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaps::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaps::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTileMap> CMaps::AddTileMap(char* szName, int iCelWidth, int iCelHeight)
{
	Ptr<CTileMap>	pMap;

	if (StrEmpty(szName))
	{
		pMap = OMalloc<CTileMap>(iCelWidth, iCelHeight);
	}
	else
	{
		pMap = ONMalloc<CTileMap>(szName, iCelWidth, iCelHeight);
	}
	maMaps.Add(pMap);

	return pMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSpriteMap> CMaps::AddSpriteMap(char* szName)
{
	Ptr<CSpriteMap>		pMap;

	if (StrEmpty(szName))
	{
		pMap = OMalloc<CSpriteMap>();
	}
	else
	{
		pMap = ONMalloc<CSpriteMap>(szName);
	}
	maMaps.Add(pMap);

	return pMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMapsContext> CMaps::GetContext(void) {	return mpContext; }

