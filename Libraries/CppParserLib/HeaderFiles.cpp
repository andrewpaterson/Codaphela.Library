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
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "HeaderFiles.h"
#include "HeaderFileMap.h"
#include "BaseLib/ErrorHandler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHeaderFiles::Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, bool bIncludeSubDirectories, bool bSystem)
{
	CFileUtil	cFileUtil;
	CChars		sz;

	//Initialised even for a directory that doesn't exist: it is searched and killed like any other, and has no files.
	mszBaseDirectory.Init(szBaseDirectory);
	//Include names are not case sensitive on Windows (CPP_HEADER_NAMES_CASE_SENSITIVE).
	mcFileNames.Init(CPP_HEADER_NAMES_CASE_SENSITIVE, false);
	mpcFileMap = pcFileMap;
	mbSystem = bSystem;

	if (!cFileUtil.Exists(szBaseDirectory))
	{
		sz.Init("Directory [");
		sz.Append(szBaseDirectory);
		sz.Append("] does not exist.");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return false;
	}

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
// The name after the last directory separator.
//
//////////////////////////////////////////////////////////////////////////
static const char* FileNamePart(const char* szPath)
{
	const char*	szName;

	for (szName = szPath; *szPath != '\0'; szPath++)
	{
		if ((*szPath == '/') || (*szPath == '\\'))
		{
			szName = szPath + 1;
		}
	}
	return szName;
}


//////////////////////////////////////////////////////////////////////////
//
// The directory's headers: files with no extension (as C++'s own, <new>)
// and those with a header's (CPP_HEADER_EXTENSIONS).
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFiles::AddFiles(bool bIncludeSubDirectories)
{
	static const char*	aszExtensions[] = { CPP_HEADER_EXTENSIONS, NULL };
	CArrayChars			aszFileNames;
	CFileUtil			cFileUtil;
	char*				szFile;
	const char*			szName;
	size				iBaseDirLen;
	size				i;
	size				j;
	bool				bHeader;

	iBaseDirLen = mszBaseDirectory.Length();
	aszFileNames.Init();
	cFileUtil.FindAllFiles(mszBaseDirectory.Text(), &aszFileNames, bIncludeSubDirectories, false);
	for (j = 0; j < aszFileNames.NumElements(); j++)
	{
		szFile = aszFileNames.Get(j)->Text();
		szName = &szFile[iBaseDirLen + 1];
		bHeader = strchr(FileNamePart(szName), '.') == NULL;
		for (i = 0; !bHeader && aszExtensions[i]; i++)
		{
			bHeader = cFileUtil.IsExtension(szFile, aszExtensions[i]);
		}
		if (bHeader)
		{
			AddFile((char*)szName);
		}
	}
	aszFileNames.Kill();
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
char* CHeaderFiles::GetBaseDirectory(void)
{
	return mszBaseDirectory.Text();
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

