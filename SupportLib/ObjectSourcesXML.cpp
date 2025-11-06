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
#include "BaseLib/Logger.h"
#include "BaseLib/MarkupTextParser.h"
#include "MovableBlocks.h"
#include "MovableBlockImageCel.h"
#include "ObjectSourcesXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourcesXML::Import(CMovableBlocks* pcWorld, CMarkupTag* pcTag)
{
	CMarkupTag*		pcObjectClass;
	STagIterator	sIter;
	bool			bResult;
	
	mpcWorld = pcWorld;
	pcObjectClass = pcTag->GetTag("ObjectClass", &sIter);
	while (pcObjectClass)
	{
		bResult = ImportObjectClass(pcWorld, pcObjectClass);
		if (!bResult)
		{
			return false;
		}

		pcObjectClass = pcTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourcesXML::ImportObjectClass(CMovableBlocks* pcWorld, CMarkupTag* pcTag)
{
	CMarkupTag*	pcClass;
	CMarkupTag*	pcFields;
	CMarkupTag*	pcObjects;
	CChars		szClass;
	CMovableBlockType*	pcType;
	bool		bResult;

	pcClass = CMarkupTextParser::GetTag(pcTag, "Class");
	if (!pcClass)
	{
		return false;
	}

	pcFields = CMarkupTextParser::GetTag(pcTag, "Fields");

	pcObjects = CMarkupTextParser::GetTag(pcTag, "Objects");
	if (!pcClass)
	{
		return false;
	}

	szClass.Init();
	pcClass->GetText(&szClass);

	bResult = false;
	if (szClass.EqualsIgnoreCase("Image"))
	{
		pcType = pcWorld->GetType("Image");
		bResult = ImportImages(pcType, pcObjects);
	}
	else if (szClass.EqualsIgnoreCase("Boolean"))
	{
		pcType = pcWorld->GetType("Boolean");
		bResult = ImportBooleans(pcType, pcObjects);
	}
	else
	{
		CMarkupTextParser::LogError(pcClass, "Type not allowed.  Must be Image or Boolean.");
	}

	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourcesXML::ImportImages(CMovableBlockType* pcType, CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcObject;
	bool			bResult;

	pcObject = pcTag->GetTag("Object", &sIter);
	while (pcObject)
	{
		bResult = ImportImage(pcType, pcObject);
		if (!bResult)
		{
			return false;
		}
		pcObject = pcTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourcesXML::ImportImage(CMovableBlockType* pcType, CMarkupTag* pcTag)
{
	CMarkupTag*					pcName;
	CMarkupTag*					pcCelIndex;
	CMarkupTag*					pcBrushID;
	CMarkupTag*					pcSourceName;
	CMarkupTag*					pcProperties;
	CChars						szName;
	int64						iCelIndex;
	int64						iBrushID;
	CChars						szSourceName;
	CMovableBlockImageCel*				pcTile;
	Ptr<CImageCelGroup>	pcGroup;
	CImageCel*					pcCel;

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return false;
	}
	pcCelIndex = CMarkupTextParser::GetTag(pcTag, "CelIndex");
	if (!pcCelIndex)
	{
		return false;
	}
	pcBrushID = CMarkupTextParser::GetTag(pcTag, "BrushID");
	if (!pcBrushID)
	{
		return false;
	}
	pcSourceName = CMarkupTextParser::GetTag(pcTag, "SourceName");
	if (!pcSourceName)
	{
		return false;
	}
	pcProperties = CMarkupTextParser::GetTag(pcTag, "Properties");

	szName.Init();
	pcName->GetText(&szName);
	if (!CMarkupTextParser::ReadInteger(pcCelIndex, &iCelIndex))
	{
		szName.Kill();
		return false;
	}
	if (!CMarkupTextParser::ReadInteger(pcBrushID, &iBrushID))
	{
		szName.Kill();
		return false;
	}
	szSourceName.Init();
	pcSourceName->GetText(&szSourceName);
	if (szSourceName.Empty())
	{
		CMarkupTextParser::LogErrorTagWasEmpty(pcSourceName);
		szName.Kill();
		szSourceName.Kill();
		return false;
	}


	pcGroup = mpcWorld->GetGroup(szSourceName.Text());
	if (!pcGroup)
	{
		szName.Kill();
		szSourceName.Kill();
		gcLogger.Error("Couldn't find Cel Group.");
		return false;
	}

	pcCel = pcGroup->GetCel((size)iCelIndex);
	if (!pcCel)
	{
		szName.Kill();
		szSourceName.Kill();
		gcLogger.Error("Couldn't find Cel.");
		return false;
	}

	pcTile = UMalloc(CMovableBlockImageCel);
	pcTile->Init(pcCel, pcType, szName.Text());
	pcType->AddTile(pcTile);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectSourcesXML::ImportBooleans(CMovableBlockType* pcType, CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcObject;

	pcObject = pcTag->GetTag("Object", &sIter);
	while (pcObject)
	{
		pcObject = pcTag->GetNextTag(&sIter);
	}
	return true;
}

