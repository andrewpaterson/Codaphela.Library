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
#include "StandardLib/Objects.h"
#include "MovableBlockBoolean.h"
#include "MovableBlockImageCel.h"
#include "MapsContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::Init(void)
{
	CMovableBlockImageCel*	pcNullImageTile;
	CMovableBlockBoolean*	pcNullBooleanTile;
	CMovableBlockType*		pcBooleanType;
	CMovableBlockType*		pcImageType;

	macBlockTypes.Init();
	macSpriteTypes.Init();

	macImages.Init();
	macGroups.Init();

	pcImageType = AddBlockType("Image");
	pcNullImageTile = UMalloc(CMovableBlockImageCel);
	pcNullImageTile->Init(NULL, pcImageType, "NULL");
	pcImageType->AddTile(pcNullImageTile);

	pcBooleanType = AddBlockType("Boolean");
	pcNullBooleanTile = UMalloc(CMovableBlockBoolean);
	pcNullBooleanTile->Init(false, pcBooleanType, "NULL");
	pcBooleanType->AddTile(pcNullBooleanTile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::Kill(void)
{
	macGroups.Kill();
	macImages.Kill();
	macSpriteTypes.Kill();
	macBlockTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMovableBlockType* CMapsContext::AddBlockType(char* szTypeName)
{
	CMovableBlockType*	pcType;

	pcType = macBlockTypes.Add();
	pcType->Init(szTypeName);
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMovableBlockType* CMapsContext::GetBlockType(char* szTypeName)
{
	SSetIterator	sIter;
	CMovableBlockType*		pcType;

	pcType = (CMovableBlockType*)macBlockTypes.StartIteration(&sIter);
	while (pcType)
	{
		if (pcType->Is(szTypeName))
		{
			return pcType;
		}
		pcType = (CMovableBlockType*)macBlockTypes.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSpriteType* CMapsContext::AddSpriteType(char* szTypeName)
{
	CSpriteType*	pcType;

	pcType = macSpriteTypes.Add();
	pcType->Init(szTypeName);
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSpriteType* CMapsContext::GetSpriteType(char* szTypeName)
{
	SSetIterator	sIter;
	CSpriteType*	pcType;

	pcType = (CSpriteType*)macSpriteTypes.StartIteration(&sIter);
	while (pcType)
	{
		if (pcType->Is(szTypeName))
		{
			return pcType;
		}
		pcType = (CSpriteType*)macSpriteTypes.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::AddImages(Ptr<CArray<CImage>> pacImages)
{
	macImages.AddAll(pacImages);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::AddGroup(Ptr<CImageCelGroup> pcGroup)
{
	macGroups.Add(pcGroup);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCelGroup> CMapsContext::GetGroup(char* szName)
{
	size					i;
	Ptr<CImageCelGroup>		pcImageCelGroup;

	for (i = 0; i < macGroups.NumElements(); i++)
	{
		pcImageCelGroup = macGroups.Get(i);
		if (pcImageCelGroup->IsNamed(szName))
		{
			return pcImageCelGroup;
		}
	}

	return NULL;
}

