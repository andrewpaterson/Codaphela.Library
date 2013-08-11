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
#include "StandardLib/Objects.h"
#include "TileBoolean.h"
#include "TileImageCel.h"
#include "TileWorld.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileWorld::Init(void)
{
	CTileImageCel*	pcNullImageTile;
	CTileBoolean*	pcNullBooleanTile;
	CTileType*		pcBooleanType;

	macMaps.Init(1);
	macTileTypes.Init(2);

	macImages.Init();
	macGroups.Init();

	mpcImageType = AddType("Image");
	pcNullImageTile = UMalloc(CTileImageCel);
	pcNullImageTile->Init(NULL, mpcImageType, "NULL");
	mpcImageType->AddTile(pcNullImageTile);

	pcBooleanType = AddType("Boolean");
	pcNullBooleanTile = UMalloc(CTileBoolean);
	pcNullBooleanTile->Init(FALSE, pcBooleanType, "NULL");
	pcBooleanType->AddTile(pcNullBooleanTile);
	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileWorld::Kill(void)
{
	macGroups.Kill();
	macImages.Kill();

	macTileTypes.Kill();
	macMaps.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileType* CTileWorld::AddType(char* szTypeName)
{
	CTileType*	pcType;

	pcType = macTileTypes.Add();
	pcType->Init(szTypeName);
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileMap* CTileWorld::AddMap(char* szName, int iCelWidth, int iCelHeight)
{
	CTileMap*	pcMap;

	pcMap = macMaps.Add();
	pcMap->Init(szName, iCelWidth, iCelHeight, this);
	return pcMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTileType* CTileWorld::GetType(char* szTypeName)
{
	SSetIterator	sIter;
	CTileType*		pcType;

	pcType = (CTileType*)macTileTypes.StartIteration(&sIter);
	while (pcType)
	{
		if (pcType->Is(szTypeName))
		{
			return pcType;
		}
		pcType = (CTileType*)macTileTypes.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileWorld::AddImages(Ptr<CArray<CImage>> pacImages)
{
	macImages.AddAll(pacImages);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//CImageCelGroup* CTileWorld::AddGroup(char* szName)
//{
//	CImageCelGroup*		pcGroup;
//
//	pcGroup = macGroups.Add();
//	pcGroup->Init(szName);
//	return pcGroup;
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileWorld::AddGroup(Ptr<CImageCelGroup> pcGroup)
{
	macGroups.Add(pcGroup);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCelGroup> CTileWorld::GetGroup(char* szName)
{
	int						i;
	Ptr<CImageCelGroup>		pcImageCelGroup;

	for (i = 0; i < macGroups.NumElements(); i++)
	{
		pcImageCelGroup = macGroups.Get(i);
		if (pcImageCelGroup->IsNamed(szName))
		{
			return pcImageCelGroup;
		}
	}
	return ONull;
}

