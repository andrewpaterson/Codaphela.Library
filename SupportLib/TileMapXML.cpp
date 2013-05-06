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
#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/CSVFileImutable.h"
#include "CoreLib/XMLFile.h"
#include "CoreLib/MarkupTextParser.h"
#include "ObjectSourcesXML.h"
#include "ImageCelsSourceXML.h"
#include "TileMapXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapXML::Init(char* szMapName, char* szTexturePath)
{
	CFileUtil		cFileUtil;

	mszMapName.Init(szMapName);
	mszTexturePath.Init(szTexturePath);

	cFileUtil.FullPath(&mszMapName);
	cFileUtil.FullPath(&mszTexturePath);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMapXML::Kill(void)
{
	mpcWorld = NULL;
	mszTexturePath.Kill();
	mszMapName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::Import(CTileWorld* pcTileWorld)
{
	CXMLFile		cXMLFile;
	CMarkup*		pcMarkup;
	BOOL			bResult;
	CMarkupTag*		pcTagBrushSources;
	CMarkupTag*		pcTagObjectSources;
	CMarkupTag*		pcTagMaps;
	CChars			szFileName;
	CChars			szDirectory;
	CFileUtil		cFileUtil;

	mpcWorld = pcTileWorld;
	mpcWorld->Init();

	szFileName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(mszMapName.Text(), &szFileName, &szDirectory);

	cXMLFile.Init();
	bResult = cXMLFile.Read(szFileName.Text(), szDirectory.Text());
	szFileName.Kill();
	szDirectory.Kill();
	if (!bResult)
	{
		cXMLFile.Kill();
		gcLogger.Error2(szFileName.Text(), " not found or could not be parsed.", NULL);
		return FALSE;
	}
	pcMarkup = &cXMLFile.mcMarkup;

	if (pcMarkup->GetRootTag() == NULL)
	{
		pcMarkup->Kill();
		gcLogger.Error("XML root tag not found");
		return FALSE;
	}

	pcTagBrushSources = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "BrushSources");
	if (!pcTagBrushSources)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	pcTagObjectSources = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "ObjectClasses");
	if (!pcTagObjectSources)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	pcTagMaps = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "Maps");
	if (!pcTagMaps)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	bResult = ImportBrushSources(pcTagBrushSources);
	if (!bResult)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	bResult = ImportObjectSources(pcTagObjectSources);
	if (!bResult)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	bResult = ImportMaps(pcTagMaps);
	if (!bResult)
	{
		pcMarkup->Kill();
		return FALSE;
	}

	pcMarkup->Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportObjectSources(CMarkupTag* pcTag)
{
	CObjectSourcesXML	cObjectSourcesXML;
	BOOL				bResult;

	bResult = cObjectSourcesXML.Import(mpcWorld, pcTag);

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
BOOL CTileMapXML::ImportBrushSources(CMarkupTag* pcTag)
{
	CImageCelsSourceXML cImageCelsSourceXML;
	BOOL				bResult;

	bResult = cImageCelsSourceXML.Import(mpcWorld, pcTag, mszTexturePath.Text());
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
BOOL CTileMapXML::ImportMaps(CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcTileMapWrapper;
	BOOL			bResult;

	pcTileMapWrapper = pcTag->GetTag("TileMapWrapper", &sIter);
	while (pcTileMapWrapper)
	{
		bResult = ImportMap(pcTileMapWrapper);
		if (!bResult)
		{
			return FALSE;
		}
		pcTileMapWrapper = pcTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportMap(CMarkupTag* pcTag)
{
	CMarkupTag*		pcTileMap;
	CMarkupTag*		pcName;
	CMarkupTag*		pcCelWidth;
	CMarkupTag*		pcCelHeight;
	BOOL			bResult;
	CChars			szName;
	int				iCelWidth;
	int				iCelHeight;
	CTileMap*		pcMap;

	pcTileMap = CMarkupTextParser::GetTag(pcTag, "TileMap");
	if (!pcTileMap)
	{
		return FALSE;
	}

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return FALSE;
	}

	pcCelWidth = CMarkupTextParser::GetTag(pcTag, "CelWidth");
	if (!pcCelWidth)
	{
		return FALSE;
	}

	pcCelHeight = CMarkupTextParser::GetTag(pcTag, "CelHeight");
	if (!pcCelHeight)
	{
		return FALSE;
	}

	if (!CMarkupTextParser::ReadInteger(pcCelWidth, &iCelWidth))
	{
		return FALSE;
	}
	if (!CMarkupTextParser::ReadInteger(pcCelHeight, &iCelHeight))
	{
		return FALSE;
	}

	szName.Init();
	pcName->GetText(&szName);
	if (szName.Empty())
	{
		szName.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcName);
		return FALSE;
	}

	pcMap = mpcWorld->AddMap(szName.Text(), iCelWidth, iCelHeight);

	bResult = ImportMap2(pcTileMap, pcMap);
	if (!bResult)
	{
		return FALSE;
	}

	szName.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportMap2(CMarkupTag* pcTag, CTileMap* pcMap)
{
	CMarkupTag*		pcWidth;
	CMarkupTag*		pcHeight;
	CMarkupTag*		pcLayers;
	int				iWidth;
	int				iHeight;
	
	pcWidth = CMarkupTextParser::GetTag(pcTag, "Width");
	if (!pcWidth)
	{
		return FALSE;
	}

	pcHeight = CMarkupTextParser::GetTag(pcTag, "Height");
	if (!pcHeight)
	{
		return FALSE;
	}

	pcLayers = CMarkupTextParser::GetTag(pcTag, "Layers");
	if (!pcLayers)
	{
		return FALSE;
	}

	if (!CMarkupTextParser::ReadInteger(pcWidth, &iWidth))
	{
		return FALSE;
	}
	if (!CMarkupTextParser::ReadInteger(pcHeight, &iHeight))
	{
		return FALSE;
	}

	pcMap->SetMapSize(iWidth, iHeight);

	return ImportLayers(pcLayers, pcMap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportLayers(CMarkupTag* pcTag, CTileMap* pcMap)
{
	STagIterator	sIter;
	CMarkupTag*		pcLayer;
	BOOL			bResult;

	pcLayer = pcTag->GetTag("Layer", &sIter);
	while (pcLayer)
	{
		bResult = ImportLayer(pcLayer, pcMap);
		if (!bResult)
		{
			return bResult;
		}
		pcLayer = pcTag->GetNextTag(&sIter);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportLayer(CMarkupTag* pcTag, CTileMap* pcMap)
{
	CMarkupTag*		pcName;
	CMarkupTag*		pcObjectClass;
	CMarkupTag*		pcTiles;
	CChars			szName;
	CChars			szObjectClass;
	BOOL			bResult;
	CTileType*		pcType;
	CTileLayer*		pcLayer;

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return FALSE;
	}
	pcObjectClass = CMarkupTextParser::GetTag(pcTag, "ObjectClass");
	if (!pcObjectClass)
	{
		return FALSE;
	}
	pcTiles = CMarkupTextParser::GetTag(pcTag, "Tiles");
	if (!pcTiles)
	{
		return FALSE;
	}

	szName.Init();
	pcName->GetText(&szName);
	if (szName.Empty())
	{
		szName.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcName);
		return FALSE;
	}

	szObjectClass.Init();
	pcObjectClass->GetText(&szObjectClass);
	if (szObjectClass.Empty())
	{
		szName.Kill();
		szObjectClass.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcObjectClass);
		return FALSE;
	}

	pcType = mpcWorld->GetType(szObjectClass.Text());
	if (!pcType)
	{
		CMarkupTextParser::LogError(pcObjectClass, "Could not find a TileType for Tag.");
		return FALSE;
	}

	pcLayer = pcMap->AddLayer(szName.Text(), pcType);

	bResult = ImportTiles(pcTiles, pcLayer);

	szObjectClass.Kill();
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTileMapXML::ImportTiles(CMarkupTag* pcTag, CTileLayer* pcLayer)
{
	CChars				szCSV;
	CCSVFileImmutable	cCSVFile;
	CMemoryFile			cMemoryFile;
	int					iRow;
	SCSVRowImmutable*	psRow;
	int					i;
	char*				szCelIndex;
	CTextParser			cTextParser;
	TRISTATE			tResult;
	int					iCelIndex;
	CChars				szError;
	CTile*				pcTile;

	szCSV.Init(128);
	pcTag->GetText(&szCSV);
	if (szCSV.Empty())
	{
		szCSV.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcTag);
		return FALSE;
	}

	szCSV.StripWhiteSpace(TRUE);
	//szCSV.Replace(" ", "");  //Write a test for this, why does it not work?

	cMemoryFile.Init(szCSV.Text(), szCSV.Length());

	cCSVFile.Init(',');
	cCSVFile.Open(&cMemoryFile);
	cCSVFile.ReadAllLines();

	for (iRow = 0; iRow < cCSVFile.NumRows(); iRow++)
	{
		psRow = cCSVFile.Get(iRow);

		for (i = 0; i < psRow->iNumFields; i++)
		{
			szCelIndex = psRow->Get(i);
			cTextParser.Init(szCelIndex);
			tResult = cTextParser.GetInteger(&iCelIndex);
			if (tResult != TRITRUE)
			{
				szError.Init("CSV cell [");
				szError.Append(i);
				szError.Append(", ");
				szError.Append(iRow);
				szError.Append("] could not be parsed as an integer.");
				CMarkupTextParser::LogError(pcTag, szError.Text());
				szError.Kill();
				cCSVFile.Close();
				cMemoryFile.Kill();
				szCSV.Kill();
				return FALSE;
			}
			
			pcTile = pcLayer->mpcTileType->Get(iCelIndex);
			if (pcTile)
			{
				pcLayer->Set(i, iRow, pcTile);
			}
		}
	}

	cCSVFile.Close();
	cMemoryFile.Kill();
	szCSV.Kill();
	return TRUE;
}

