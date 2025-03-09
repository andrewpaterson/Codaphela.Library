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
#include "BaseLib/XMLFile.h"
#include "BaseLib/MarkupTextParser.h"
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
bool CTileMapXML::Import(CTileWorld* pcTileWorld)
{
	CXMLFile		cXMLFile;
	CMarkup*		pcMarkup;
	bool			bResult;
	CMarkupTag*		pcTagBrushSources;
	CMarkupTag*		pcTagObjectSources;
	CMarkupTag*		pcTagMaps;
	CChars			szFilename;
	CChars			szDirectory;
	CFileUtil		cFileUtil;

	mpcWorld = pcTileWorld;
	mpcWorld->Init();

	szFilename.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(mszMapName.Text(), &szFilename, &szDirectory);

	cXMLFile.Init();
	bResult = cXMLFile.Read(szFilename.Text(), szDirectory.Text());
	szFilename.Kill();
	szDirectory.Kill();
	if (!bResult)
	{
		cXMLFile.Kill();
		gcLogger.Error2(szFilename.Text(), " not found or could not be parsed.", NULL);
		return false;
	}
	pcMarkup = &cXMLFile.mcMarkup;

	if (pcMarkup->GetRootTag() == NULL)
	{
		pcMarkup->Kill();
		gcLogger.Error("XML root tag not found");
		return false;
	}

	pcTagBrushSources = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "BrushSources");
	if (!pcTagBrushSources)
	{
		pcMarkup->Kill();
		return false;
	}

	pcTagObjectSources = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "ObjectClasses");
	if (!pcTagObjectSources)
	{
		pcMarkup->Kill();
		return false;
	}

	pcTagMaps = CMarkupTextParser::GetTag(pcMarkup->GetRootTag(), "Maps");
	if (!pcTagMaps)
	{
		pcMarkup->Kill();
		return false;
	}

	bResult = ImportBrushSources(pcTagBrushSources);
	if (!bResult)
	{
		pcMarkup->Kill();
		return false;
	}

	bResult = ImportObjectSources(pcTagObjectSources);
	if (!bResult)
	{
		pcMarkup->Kill();
		return false;
	}

	bResult = ImportMaps(pcTagMaps);
	if (!bResult)
	{
		pcMarkup->Kill();
		return false;
	}

	pcMarkup->Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapXML::ImportObjectSources(CMarkupTag* pcTag)
{
	CObjectSourcesXML	cObjectSourcesXML;
	bool				bResult;

	bResult = cObjectSourcesXML.Import(mpcWorld, pcTag);

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
bool CTileMapXML::ImportBrushSources(CMarkupTag* pcTag)
{
	CImageCelsSourceXML cImageCelsSourceXML;
	bool				bResult;

	bResult = cImageCelsSourceXML.Import(mpcWorld, pcTag, mszTexturePath.Text());
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
bool CTileMapXML::ImportMaps(CMarkupTag* pcTag)
{
	STagIterator	sIter;
	CMarkupTag*		pcTileMapWrapper;
	bool			bResult;

	pcTileMapWrapper = pcTag->GetTag("TileMapWrapper", &sIter);
	while (pcTileMapWrapper)
	{
		bResult = ImportMap(pcTileMapWrapper);
		if (!bResult)
		{
			return false;
		}
		pcTileMapWrapper = pcTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapXML::ImportMap(CMarkupTag* pcTag)
{
	CMarkupTag*		pcTileMap;
	CMarkupTag*		pcName;
	CMarkupTag*		pcCelWidth;
	CMarkupTag*		pcCelHeight;
	bool			bResult;
	CChars			szName;
	int64			iCelWidth;
	int64			iCelHeight;
	CTileMap*		pcMap;

	pcTileMap = CMarkupTextParser::GetTag(pcTag, "TileMap");
	if (!pcTileMap)
	{
		return false;
	}

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return false;
	}

	pcCelWidth = CMarkupTextParser::GetTag(pcTag, "CelWidth");
	if (!pcCelWidth)
	{
		return false;
	}

	pcCelHeight = CMarkupTextParser::GetTag(pcTag, "CelHeight");
	if (!pcCelHeight)
	{
		return false;
	}

	if (!CMarkupTextParser::ReadInteger(pcCelWidth, &iCelWidth))
	{
		return false;
	}
	if (!CMarkupTextParser::ReadInteger(pcCelHeight, &iCelHeight))
	{
		return false;
	}

	szName.Init();
	pcName->GetText(&szName);
	if (szName.Empty())
	{
		szName.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcName);
		return false;
	}

	pcMap = mpcWorld->AddMap(szName.Text(), (int)iCelWidth, (int)iCelHeight);

	bResult = ImportMap2(pcTileMap, pcMap);
	if (!bResult)
	{
		return false;
	}

	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapXML::ImportMap2(CMarkupTag* pcTag, CTileMap* pcMap)
{
	CMarkupTag*		pcWidth;
	CMarkupTag*		pcHeight;
	CMarkupTag*		pcLayers;
	int64			iWidth;
	int64			iHeight;
	
	pcWidth = CMarkupTextParser::GetTag(pcTag, "Width");
	if (!pcWidth)
	{
		return false;
	}

	pcHeight = CMarkupTextParser::GetTag(pcTag, "Height");
	if (!pcHeight)
	{
		return false;
	}

	pcLayers = CMarkupTextParser::GetTag(pcTag, "Layers");
	if (!pcLayers)
	{
		return false;
	}

	if (!CMarkupTextParser::ReadInteger(pcWidth, &iWidth))
	{
		return false;
	}
	if (!CMarkupTextParser::ReadInteger(pcHeight, &iHeight))
	{
		return false;
	}

	pcMap->SetMapSize((int)iWidth, (int)iHeight);

	return ImportLayers(pcLayers, pcMap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapXML::ImportLayers(CMarkupTag* pcTag, CTileMap* pcMap)
{
	STagIterator	sIter;
	CMarkupTag*		pcLayer;
	bool			bResult;

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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMapXML::ImportLayer(CMarkupTag* pcTag, CTileMap* pcMap)
{
	CMarkupTag*		pcName;
	CMarkupTag*		pcObjectClass;
	CMarkupTag*		pcTiles;
	CChars			szName;
	CChars			szObjectClass;
	bool			bResult;
	CTileType*		pcType;
	CTileLayer*		pcLayer;

	pcName = CMarkupTextParser::GetTag(pcTag, "Name");
	if (!pcName)
	{
		return false;
	}
	pcObjectClass = CMarkupTextParser::GetTag(pcTag, "ObjectClass");
	if (!pcObjectClass)
	{
		return false;
	}
	pcTiles = CMarkupTextParser::GetTag(pcTag, "Tiles");
	if (!pcTiles)
	{
		return false;
	}

	szName.Init();
	pcName->GetText(&szName);
	if (szName.Empty())
	{
		szName.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcName);
		return false;
	}

	szObjectClass.Init();
	pcObjectClass->GetText(&szObjectClass);
	if (szObjectClass.Empty())
	{
		szName.Kill();
		szObjectClass.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcObjectClass);
		return false;
	}

	pcType = mpcWorld->GetType(szObjectClass.Text());
	if (!pcType)
	{
		CMarkupTextParser::LogError(pcObjectClass, "Could not find a TileType for Tag.");
		return false;
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
bool CTileMapXML::ImportTiles(CMarkupTag* pcTag, CTileLayer* pcLayer)
{
	CChars				szCSV;
	CCSVFileImmutable	cCSVFile;
	CMemoryFile			cMemoryFile;
	size				iRow;
	SCSVRowImmutable*	psRow;
	int					i;
	char*				szCelIndex;
	CTextParser			cTextParser;
	TRISTATE			tResult;
	int64				iCelIndex;
	CChars				szError;
	CTile*				pcTile;

	szCSV.Init();
	pcTag->GetText(&szCSV);
	if (szCSV.Empty())
	{
		szCSV.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcTag);
		return false;
	}

	szCSV.StripWhitespace(true);
	//szCSV.Replace(" ", "");  //Write a test for this, why does it not work?

	cMemoryFile.Init(szCSV.Text(), szCSV.Length());

	cCSVFile.Init(',');
	cCSVFile.Open(&cMemoryFile);
	cCSVFile.ReadAllLines();

	for (iRow = 0; iRow < cCSVFile.NumRows(); iRow++)
	{
		psRow = cCSVFile.Get(iRow);

		for (i = 0; i < psRow->NumElements(); i++)
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
				return false;
			}
			
			pcTile = pcLayer->mpcTileType->Get((size)iCelIndex);
			if (pcTile)
			{
				pcLayer->Set(i, iRow, pcTile);
			}
		}
	}

	cCSVFile.Close();
	cMemoryFile.Kill();
	szCSV.Kill();
	return true;
}

