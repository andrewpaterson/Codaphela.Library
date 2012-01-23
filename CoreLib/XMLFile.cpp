/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
BOOL CXMLFile::Read(char* szFileName, char* szDirectory)
{
	BOOL			bResult;
	CMarkupDoc*		pcDoc;

	mszDirectory.Init(szDirectory);

	pcDoc = mcMarkup.mpcDoc;
	bResult = Read(szFileName, pcDoc);
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
BOOL CXMLFile::Read(char* szFileName, CMarkupDoc* pcDoc)
{
	CXMLParser	cXMLParser;
	CTextFile	cTextFile;
	BOOL		bResult;
	TRISTATE	tResult;
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init(mszDirectory);
	cFileUtil.FullPath(&szPath);
	cFileUtil.AppendToPath(&szPath, szFileName);

	cTextFile.Init();
	bResult = cTextFile.Read(szPath.Text());
	if (!bResult)
	{
		cTextFile.Kill();
		return FALSE;
	}

	cTextFile.PassifyNewlines();
	cXMLParser.Init(pcDoc, &gcLogger);
	tResult = cXMLParser.Parse(cTextFile.Text(), szFileName);
	if (tResult != TRITRUE)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		return FALSE;
	}

	bResult = Entities(&cXMLParser);
	if (!bResult)
	{
		cXMLParser.Kill();
		cTextFile.Kill();
		return FALSE;
	}

	cXMLParser.Kill();
	cTextFile.Kill();
	return TRUE;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CXMLFile::Entities(CXMLParser* pcXMLParser)
{
	int					i;
	CXMLEntity*			pcEntity;
	CMarkup*			pcMarkup;
	CMarkupSubText*		pcSubText;
	CMarkupSubDoc*		pcSubDoc;
	CMarkupDoc*			pcDoc;
	BOOL				bResult;

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
			bResult = Read(pcEntity->mszValue.Text(), pcDoc);
			if (!bResult)
			{
				return FALSE;
			}
		}
		else if (pcEntity->meType == XET_Internal)
		{
			pcSubText = pcMarkup->AllocateSubText();
			pcSubText->Init(pcEntity->mszName.Text(), pcEntity->mszValue.Text());
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CXMLFile::Write(char* szFileName, char* szDirectory)
{
	return FALSE;
}

