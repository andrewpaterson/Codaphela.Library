/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
BOOL CHeaderNameMap::Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, BOOL bSystem)
{
	CFileUtil cFileUtil;

	if (!cFileUtil.Exists(szBaseDirectory))
	{
		CChars	sz;

		sz.Init();
		sz.Append("Directory [");
		sz.Append(szBaseDirectory);
		sz.Append("] does not exist.");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}

	mszBaseDirectory.Init(szBaseDirectory);
	mcFileNames.Init(16, FALSE);
	mpcFileMap = pcFileMap;
	mbSystem = bSystem;

	AddFiles();
	return TRUE;
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
	CArrayChars				aszTemp;
	int						i;
	int						j;
	char*					szFile;
	char*					szExtension;
	int						iBaseDirLen;
	CArrayChars				aszFileNames;
	CFileUtil				cFileUtil;
	BOOL					bIsExtension;

	szTemp.Init("h;inl;hpp;rh;");

	aszTemp.Init();
	szTemp.Split(&aszTemp, ';');
	iBaseDirLen = mszBaseDirectory.Length();


	aszFileNames.Init();
	cFileUtil.FindAllFiles(mszBaseDirectory.Text(), &aszFileNames, FALSE, FALSE);

	for (j = 0; j < aszFileNames.NumElements(); j++)
	{
		szFile = aszFileNames.Get(j)->Text();

		for (i = 0; i < aszTemp.NumElements(); i++)
		{
			szExtension = aszTemp.Get(i)->Text();
			bIsExtension = cFileUtil.IsExtension(szFile, szExtension);
			if (bIsExtension)
			{
				AddFile(&szFile[iBaseDirLen + 1]);
				break;
			}
		}
	}

	aszFileNames.Kill();


	szTemp.Kill();
	aszTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderNameMap::AddFile(char* szFile)
{
	CHeaderFile* pcHeader;
	CChars			szShortName;
	CChars			szFullName;

	szFullName.Init(mszBaseDirectory);
	szFullName.Append(FILE_SEPARATOR);
	szFullName.Append(szFile);
	pcHeader = mpcFileMap->AddFile(szFullName.Text(), mbSystem);
	szFullName.Kill();

	szShortName.Init(szFile);
	szShortName.Replace('\\', '/');
	if (!mcFileNames.HasKey(szShortName.Text()))
	{
		mcFileNames.Put(szShortName.Text(), &pcHeader);
	}
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
	bResult = mcFileNames.StartIteration(&sIter, (void**)&szName, NULL, (void**)&pcHeader, NULL);
	while (bResult)
	{	
		sz.Append(szName);
		sz.AppendNewLine();
		bResult = mcFileNames.Iterate(&sIter, (void**)&szName, NULL, (void**)&pcHeader, NULL);
	}
	sz.Dump();
	sz.Kill();
}

