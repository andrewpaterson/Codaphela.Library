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
#include "BaseLib/Logger.h"
#include "CoreLib/MarkupTextParser.h"
#include "TileWorld.h"
#include "TileImageCel.h"
#include "ObjectSourcesXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourcesXML::Import(CTileWorld* pcWorld, CMarkupTag* pcTag)
{
	CMarkupTag*		pcObjectClass;
	STagIterator	sIter;
	BOOL			bResult;
	
	mpcWorld = pcWorld;
	pcObjectClass = pcTag->GetTag("ObjectClass", &sIter);
	while (pcObjectClass)
	{
		bResult = ImportObjectClass(pcWorld, pcObjectClass);
		if (!bResult)
		{
			return FALSE;
		}

		pcObjectClass = pcTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourcesXML::ImportObjectClass(CTileWorld* pcWorld, CMarkupTag* pcTag)
{
	CMarkupTag*	pcClass;
	CMarkupTag*	pcFields;
	CMarkupTag*	pcObjects;
	CChars		szClass;
	CTileType*	pcType;
	BOOL		bResult;

	pcClass = CMarkupTextParser::GetTag(pcTag, "Class");
	if (!pcClass)
	{
		return FALSE;
	}

	pcFields = CMarkupTextParser::GetTag(pcTag, "Fields");

	pcObjects = CMarkupTextParser::GetTag(pcTag, "Objects");
	if (!pcClass)
	{
		return FALSE;
	}

	szClass.Init();
	pcClass->GetText(&szClass);

	bResult = FALSE;
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
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourcesXML::ImportImages(CTileType* pcType, CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcObject;
	BOOL			bResult;

	pcObject = pcTag->GetTag("Object", &sIter);
	while (pcObject)
	{
		bResult = ImportImage(pcType, pcObject);
		if (!bResult)
		{
			return FALSE;
		}
		pcObject = pcTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourcesXML::ImportImage(CTileType* pcType, CMarkupTag* pcTag)
{
	CMarkupTag*			pcName;
	CMarkupTag*			pcCelIndex;
	CMarkupTag*			pcBrushID;
	CMarkupTag*			pcSourceName;
	CMarkupTag*			pcProperties;
	CChars				szName;
	int					iCelIndex;
	int					iBrushID;
	CChars				szSourceName;
	CTileImageCel*		pcTile;
	CImageCelGroup*		pcGroup;
	CImageCel*			pcCel;

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return FALSE;
	}
	pcCelIndex = CMarkupTextParser::GetTag(pcTag, "CelIndex");
	if (!pcCelIndex)
	{
		return FALSE;
	}
	pcBrushID = CMarkupTextParser::GetTag(pcTag, "BrushID");
	if (!pcBrushID)
	{
		return FALSE;
	}
	pcSourceName = CMarkupTextParser::GetTag(pcTag, "SourceName");
	if (!pcSourceName)
	{
		return FALSE;
	}
	pcProperties = CMarkupTextParser::GetTag(pcTag, "Properties");

	szName.Init();
	pcName->GetText(&szName);
	if (!CMarkupTextParser::ReadInteger(pcCelIndex, &iCelIndex))
	{
		szName.Kill();
		return FALSE;
	}
	if (!CMarkupTextParser::ReadInteger(pcBrushID, &iBrushID))
	{
		szName.Kill();
		return FALSE;
	}
	szSourceName.Init();
	pcSourceName->GetText(&szSourceName);
	if (szSourceName.Empty())
	{
		CMarkupTextParser::LogErrorTagWasEmpty(pcSourceName);
		szName.Kill();
		szSourceName.Kill();
		return FALSE;
	}


	pcGroup = mpcWorld->GetGroup(szSourceName.Text());
	if (!pcGroup)
	{
		szName.Kill();
		szSourceName.Kill();
		gcLogger.Error("Couldn't find Cel Group.");
		return FALSE;
	}

	pcCel = pcGroup->GetCel(iCelIndex);
	if (!pcCel)
	{
		szName.Kill();
		szSourceName.Kill();
		gcLogger.Error("Couldn't find Cel.");
		return FALSE;
	}

	pcTile = UMalloc(CTileImageCel);
	pcTile->Init(pcCel, pcType, szName.Text());
	pcType->AddTile(pcTile);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourcesXML::ImportBooleans(CTileType* pcType, CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcObject;

	pcObject = pcTag->GetTag("Object", &sIter);
	while (pcObject)
	{
		pcObject = pcTag->GetNextTag(&sIter);
	}
	return TRUE;
}

