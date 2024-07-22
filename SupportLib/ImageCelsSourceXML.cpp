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
#include "BaseLib/FileUtil.h"
#include "BaseLib/MarkupTextParser.h"
#include "StandardLib/Pointer.h"
#include "StandardLib/Objects.h"
#include "ImageCelSourceSubImages.h"
#include "ImageCelsSource.h"
#include "SubImageXML.h"
#include "ImageCelsSourceXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSourceXML::Import(CTileWorld* pcWorld, CMarkupTag* pcTag, char* szTexturePath)
{
	STagIterator	sIter;
	CMarkupTag*		pcBrushSourceTag;
	bool			bResult;

	mpcWorld = pcWorld;

	pcBrushSourceTag = pcTag->GetTag("BrushSource", &sIter);
	while (pcBrushSourceTag)
	{
		bResult = ImportCelSource(pcBrushSourceTag, szTexturePath);
		if (!bResult)
		{
			return false;
		}
		pcBrushSourceTag = pcTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSourceXML::ImportCelSource(CMarkupTag* pcBrushSourceTag, char* szTexturePath)
{
	CMarkupTag*			pcFileName;
	CChars				szFilename;
	CChars				szShortFileName;
	bool				bResult;
	CMarkupTag*			pcCels;
	CFileUtil			cFileUtil;

	pcFileName = CMarkupTextParser::GetTag(pcBrushSourceTag, "FileName");
	if (!pcFileName)
	{
		return false;
	}

	szShortFileName.Init();
	pcFileName->GetText(&szShortFileName);
	if (szFilename.Empty())
	{
		szShortFileName.Kill();
		CMarkupTextParser::LogErrorTagWasEmpty(pcBrushSourceTag);
		return false;
	}
	
	pcCels = CMarkupTextParser::GetTag(pcBrushSourceTag, "Cels");
	if (!pcCels)
	{
		szShortFileName.Kill();
		return false;
	}

	szFilename.Init(szTexturePath);
	cFileUtil.AppendToPath(&szFilename, szShortFileName.Text());

	bResult = ImportCels(pcCels, szFilename.Text());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCelsSourceXML::ImportCels(CMarkupTag* pcCelsTag, char* szFilename)
{
	STagIterator				sIter;
	CSubImageXML				cSubImageXML;
	CMarkupTag*					pcCelTag;
	CArraySubImage				acSubImages;
	CSubImage*					pcSubImage;
	bool						bResult;
	CImageCelSourceSubImages	cSubImagesSource;
	CImageCelsSource			cCelsSource;
	CChars						szGroupName;
	CFileUtil					cFileUtil;
	Ptr<CImageCelGroup>			pcGroup;

	acSubImages.Init();
	pcCelTag = pcCelsTag->GetTag("Cel", &sIter);
	while (pcCelTag)
	{
		pcSubImage = acSubImages.Add();

		bResult = cSubImageXML.Import(pcCelTag, pcSubImage);
		if (!bResult)
		{
			acSubImages.Kill();
			return false;
		}
		pcCelTag = pcCelsTag->GetNextTag(&sIter);
	}

	cSubImagesSource.Init(&acSubImages);

	szGroupName.Init(szFilename);
	cFileUtil.RemovePath(&szGroupName);

	cCelsSource.Init();
	cCelsSource.AddDiskFileSource(szFilename, szGroupName.Text(), &cSubImagesSource);
	cCelsSource.Load();

	mpcWorld->AddImages(cCelsSource.GetImages());

	pcGroup = ONMalloc<CImageCelGroup>(szGroupName.Text());
	mpcWorld->AddGroup(pcGroup);
	pcGroup->AddCels(cCelsSource.GetImageCels());

	szGroupName.Kill();
	cSubImagesSource.Kill();
	cCelsSource.Kill();
	acSubImages.Kill();
	return true;
}

