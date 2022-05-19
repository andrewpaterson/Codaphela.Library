/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
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
BOOL CXMLFile::Read(char* szFilename, char* szDirectory)
{
	BOOL			bResult;
	CMarkupDoc*		pcDoc;
	CChars			mszDirectory;

	mszDirectory.Init(szDirectory);

	pcDoc = mcMarkup.mpcDoc;
	bResult = Read(szFilename, pcDoc, &mszDirectory);
	if (!bResult)
	{
		mszDirectory.Kill();
		return FALSE;
	}

	bResult = pcDoc->FixReferences();
	if (!bResult)
	{
		mszDirectory.Kill();
		return FALSE;
	}
	mszDirectory.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CXMLFile::Read(char* szFilename, CMarkupDoc* pcDoc, CChars* pszDirectory)
{
	CXMLParser	cXMLParser;
	CTextFile	cTextFile;
	BOOL		bResult;
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
		cTextFile.Kill();
		szPath.Kill();
		return FALSE;
	}

	cTextFile.PassifyNewlines();
	cXMLParser.Init(pcDoc, &gcLogger);
	tResult = cXMLParser.Parse(cTextFile.Text(), szFilename);
	if (tResult != TRITRUE)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		szPath.Kill();
		return FALSE;
	}

	bResult = Entities(&cXMLParser, pszDirectory);
	if (!bResult)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		szPath.Kill();
		return FALSE;
	}

	cXMLParser.Kill();
	cTextFile.Kill();
	szPath.Kill();
	return TRUE;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CXMLFile::Entities(CXMLParser* pcXMLParser, CChars* pszDirectory)
{
	int					i;
	CXMLEntity*			pcEntity;
	CMarkup*			pcMarkup;
	CMarkupSubText*		pcSubText;
	CMarkupSubDoc*		pcSubDoc;
	CMarkupDoc*			pcDoc;
	BOOL				bResult;
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
				return FALSE;
			}
		}
		else if (pcEntity->meType == XET_Internal)
		{
			pcSubText = pcMarkup->AllocateSubText();
			pcSubText->Init(pcEntity->mszName.Text(), pcEntity->mszValue.Text());
			pcParentDoc->AddSubstitute(pcSubText);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CXMLFile::Write(char* szFilename, char* szDirectory)
{
	return FALSE;
}

