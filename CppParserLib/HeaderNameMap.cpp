/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/FileUtil.h"
#include "HeaderNameMap.h"
#include "HeaderFileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, BOOL bSystem)
{
	mszBaseDirectory.Init(szBaseDirectory);
	mcFileNames.Init(16, FALSE);
	mpcFileMap = pcFileMap;
	mbSystem = bSystem;

	AddFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::Kill(void)
{
	mcFileNames.Kill();
	mszBaseDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::AddFiles(void)
{
	CChars					szTemp;
	CArrayString			aszTemp;
	int						i;
	CChars*					pszFile;
	int						iIndex;
	int						j;
	char*					szFile;
	char*					szExtension;
	int						iBaseDirLen;
	CArrayString			aszFileNames;
	CFileUtil				cFileUtil;

	szTemp.Init("*.h;*.inl;*.hpp;*.rh");

	aszTemp.Init(16);
	szTemp.Split(&aszTemp, ';');
	iBaseDirLen = mszBaseDirectory.Length();

	for (i = 0; i < aszTemp.NumElements(); i++)
	{
		pszFile = aszTemp.Get(i);
		pszFile->StripWhiteSpace();

		iIndex = pszFile->Find(0, ".");
		szExtension = pszFile->Text(iIndex+1);

		aszFileNames.Init(32);
		cFileUtil.FindFilesWithExtension(mszBaseDirectory.Text(), szExtension, &aszFileNames);

		for (j = 0; j < aszFileNames.NumElements(); j++)
		{
			szFile = aszFileNames.Get(j)->Text();
			AddFile(&szFile[iBaseDirLen+1]);
		}

		aszFileNames.Kill();
	}

	szTemp.Kill();
	aszTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::AddFile(char* szFile)
{
	CHeaderFile*	pcHeader;
	CChars			szShortName;
	CChars			szFullName;

	szFullName.Init(mszBaseDirectory);
	szFullName.Append(FILE_SEPARATOR);
	szFullName.Append(szFile);
	pcHeader = mpcFileMap->AddFile(szFullName.Text(), mbSystem);
	szFullName.Kill();

	szShortName.Init(szFile);
	szShortName.Replace('\\', '/');
	mcFileNames.Put(szShortName.Text(), &pcHeader);
	szShortName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::Dump(void)
{
	CHeaderFile*	pcHeader;
	CChars			sz;
	SMapIterator	sIter;
	BOOL			bResult;
	char*			szName;

	sz.Init();
	bResult = mcFileNames.StartIteration(&sIter, (void**)&szName, (void**)&pcHeader);
	while (bResult)
	{	
		sz.Append(szName);
		sz.AppendNewLine();
		bResult = mcFileNames.Iterate(&sIter, (void**)&szName, (void**)&pcHeader);
	}
	sz.Dump();
	sz.Kill();
}

