/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "TextFile.h"
#include "FileUtil.h"
#include "XMLFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLFile::Init(void)
{
	mcMarkup.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLFile::Kill(void)
{
	mcMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLFile::Read(char* szFilename, char* szDirectory)
{
	bool			bResult;
	CMarkupDoc*		pcDoc;
	CChars			mszDirectory;

	mszDirectory.Init(szDirectory);

	pcDoc = mcMarkup.mpcDoc;
	bResult = Read(szFilename, pcDoc, &mszDirectory);
	if (!bResult)
	{
		mszDirectory.Kill();
		return false;
	}

	bResult = pcDoc->FixReferences();
	if (!bResult)
	{
		mszDirectory.Kill();
		return false;
	}
	mszDirectory.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLFile::Read(char* szFilename, CMarkupDoc* pcDoc, CChars* pszDirectory)
{
	CXMLParser	cXMLParser;
	CTextFile	cTextFile;
	bool		bResult;
	TRISTATE	tResult;
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init(pszDirectory);
	cFileUtil.FullPath(&szPath);
	cFileUtil.AppendToPath(&szPath, szFilename);

	cTextFile.Init();
	bResult = cTextFile.Read(szPath.Text());
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read XML file [", szPath.Text(), "].", NULL);
		cTextFile.Kill();
		szPath.Kill();
		return false;
	}

	cTextFile.PassifyNewlines();
	cXMLParser.Init(pcDoc, &gcLogger);
	tResult = cXMLParser.Parse(cTextFile.Text(), szFilename);
	if (tResult != TRITRUE)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		szPath.Kill();
		return false;
	}

	bResult = Entities(&cXMLParser, pszDirectory);
	if (!bResult)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		szPath.Kill();
		return false;
	}

	cXMLParser.Kill();
	cTextFile.Kill();
	szPath.Kill();
	return true;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLFile::Entities(CXMLParser* pcXMLParser, CChars* pszDirectory)
{
	int					i;
	CXMLEntity*			pcEntity;
	CMarkup*			pcMarkup;
	CMarkupSubText*		pcSubText;
	CMarkupSubDoc*		pcSubDoc;
	CMarkupDoc*			pcDoc;
	bool				bResult;
	CMarkupDoc*			pcParentDoc;

	pcParentDoc = pcXMLParser->mpcDoc;
	pcMarkup = pcXMLParser->mpcDoc->mpcMarkup;
	for (i = 0; i <	pcXMLParser->macEntities.NumElements(); i++)
	{
		pcEntity = pcXMLParser->macEntities.Get(i);
		if ((pcEntity->meType == XET_External) || (pcEntity->meType == XET_ExternalFile))
		{
			pcDoc = pcMarkup->AllocateDoc();
			pcDoc->Init(pcMarkup);
			pcSubDoc = pcMarkup->AllocateSubDoc();
			pcSubDoc->Init(pcEntity->mszName.Text(), pcDoc);
			pcParentDoc->AddSubstitute(pcSubDoc);
		
			bResult = Read(pcEntity->mszValue.Text(), pcDoc, pszDirectory);
			if (!bResult)
			{
				return false;
			}
		}
		else if (pcEntity->meType == XET_Internal)
		{
			pcSubText = pcMarkup->AllocateSubText();
			pcSubText->Init(pcEntity->mszName.Text(), pcEntity->mszValue.Text());
			pcParentDoc->AddSubstitute(pcSubText);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLFile::Write(char* szFilename, char* szDirectory)
{
	return false;
}

