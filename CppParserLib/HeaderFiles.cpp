/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "HeaderFiles.h"
#include "HeaderFileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHeaderFiles::Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, bool bIncludeSubDirectories, bool bSystem)
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
		return false;
	}

	mszBaseDirectory.Init(szBaseDirectory);
	mcFileNames.Init(true, false);
	mpcFileMap = pcFileMap;
	mbSystem = bSystem;

	AddFiles(bIncludeSubDirectories);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFiles::Kill(void)
{
	mcFileNames.Kill();
	mszBaseDirectory.Kill();
	mpcFileMap = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFiles::AddFiles(bool bIncludeSubDirectories)
{
	CChars					szTemp;
	CArrayChars				aszHeaderExtensions;
	size					i;
	size					j;
	char*					szFile;
	char*					szExtension;
	size					iBaseDirLen;
	CArrayChars				aszFileNames;
	CFileUtil				cFileUtil;
	bool					bIsExtension;
	size					iNumFileNames;
	size					iNumExtensions;

	szTemp.Init("h;inl;hpp;rh;");

	aszHeaderExtensions.Init();
	szTemp.Split(&aszHeaderExtensions, ';'); 
	szTemp.Kill();
	iBaseDirLen = mszBaseDirectory.Length();


	aszFileNames.Init();
	cFileUtil.FindAllFiles(mszBaseDirectory.Text(), &aszFileNames, bIncludeSubDirectories, false);

	iNumExtensions = aszHeaderExtensions.NumElements();
	iNumFileNames = aszFileNames.NumElements();
	for (j = 0; j < iNumFileNames; j++)
	{
		szFile = aszFileNames.Get(j)->Text();

		for (i = 0; i < iNumExtensions; i++)
		{
			szExtension = aszHeaderExtensions.Get(i)->Text();
			bIsExtension = cFileUtil.IsExtension(szFile, szExtension);
			if (bIsExtension)
			{
				AddFile(&szFile[iBaseDirLen + 1]);
				break;
			}
		}
	}

	aszFileNames.Kill();
	aszHeaderExtensions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFiles::AddFile(char* szRelativeFileName)
{
	CHeaderFile*	pcHeader;
	CChars			szShortName;
	CChars			szFullName;

	szFullName.Init(mszBaseDirectory);
	szFullName.Append(FILE_SEPARATOR);
	szFullName.Append(szRelativeFileName);
	pcHeader = mpcFileMap->AddFile(szFullName.Text(), mbSystem);
	szFullName.Kill();

	szShortName.Init(szRelativeFileName);
	szShortName.Replace('\\', '/');
	if (!mcFileNames.HasKey(szShortName.Text()))
	{
		mcFileNames.Put(szShortName.Text(), &pcHeader);
	}
	szShortName.Kill();

	return pcHeader;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFiles::GetFile(char* szRelativeFileName)
{
	CHeaderFile** ppcHeaderFile;

	ppcHeaderFile = mcFileNames.Get(szRelativeFileName);
	if (ppcHeaderFile)
	{
		return *ppcHeaderFile;
	}

	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHeaderFiles::GetBaseDirectoryLength(void)
{
	return mszBaseDirectory.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFiles::Dump(void)
{
	CHeaderFile*	pcHeader;
	CChars			sz;
	SMapIterator	sIter;
	bool			bResult;
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

