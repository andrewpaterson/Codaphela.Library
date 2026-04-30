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
#include "MapsContext.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::Init(void)
{
	PreInit();

	Ptr<CMovableBlockImageCel>	pcNullImageTile;
	Ptr<CMovableBlockBoolean>	pcNullBooleanTile;
	Ptr<CMovableBlockType>		pcBooleanType;
	Ptr<CMovableBlockType>		pcImageType;

	maBlockTypes.Init();
	maSpriteTypes.Init();

	maImages.Init();
	maGroups.Init();

	pcImageType = AddBlockType("MovableBlockType.Image");
	pcNullImageTile = OMalloc<CMovableBlockImageCel>(CPointer());
	pcImageType->AddBlock(pcNullImageTile);

	pcBooleanType = AddBlockType("MovableBlockType.Boolean");
	pcNullBooleanTile = OMalloc<CMovableBlockBoolean>(false);
	pcBooleanType->AddBlock(pcNullBooleanTile);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::Class(void)
{
	M_Embedded(maBlockTypes);
	M_Embedded(maSpriteTypes);
	M_Embedded(maImages);
	M_Embedded(maGroups);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapsContext::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapsContext::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMovableBlockType> CMapsContext::AddBlockType(char* szTypeName)
{
	Ptr<CMovableBlockType>	pcType;

	if (StrEmpty(szTypeName))
	{
		pcType = OMalloc<CMovableBlockType>();
	}
	else
	{
		pcType = ONMalloc<CMovableBlockType>(szTypeName);
	}
	maBlockTypes.Add(pcType);
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSpriteType> CMapsContext::AddSpriteType(char* szTypeName)
{
	Ptr<CSpriteType>	pcType;

	if (StrEmpty(szTypeName))
	{
		pcType = OMalloc<CSpriteType>();
	}
	else
	{
		pcType = ONMalloc<CSpriteType>(szTypeName);

	}
	maSpriteTypes.Add(pcType);
	return pcType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::AddImages(Ptr<CArrayImage> paImages)
{
	maImages.AddAll(paImages);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsContext::AddGroup(Ptr<CImageCelGroup> pGroup)
{
	maGroups.Add(pGroup);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCelGroup> CMapsContext::GetGroup(char* szName)
{
	size					i;
	Ptr<CImageCelGroup>		pcImageCelGroup;

	for (i = 0; i < maGroups.NumElements(); i++)
	{
		pcImageCelGroup = maGroups.Get(i);
		if (pcImageCelGroup->IsNamed(szName))
		{
			return pcImageCelGroup;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayMovableBlockType> CMapsContext::GetBlockTypes(void)
{
	return &maBlockTypes;
}

