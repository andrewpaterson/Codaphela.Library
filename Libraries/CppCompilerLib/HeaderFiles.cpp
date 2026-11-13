/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

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
	//The directory is a full path, as the files found in it are (AddFiles) and the files including them are.
	mszBaseDirectory.Init(szBaseDirectory);
	cFileUtil.FullPath(&mszBaseDirectory);
	//Include names are not case sensitive on Windows (CPP_HEADER_NAMES_CASE_SENSITIVE).
	mcFileNames.Init(CPP_HEADER_NAMES_CASE_SENSITIVE, false);
	mpcFileMap = pcFileMap;
	mbSystem = bSystem;
	mbSubDirectories = bIncludeSubDirectories;

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
// A header's name: a file with no extension (as C++'s own, <new>) or with
// a header's (CPP_HEADER_EXTENSIONS).
//
//////////////////////////////////////////////////////////////////////////
static bool IsHeaderName(const char* szName)
{
	static const char*	aszExtensions[] = { CPP_HEADER_EXTENSIONS, NULL };
	CFileUtil			cFileUtil;
	size				i;

	if (strchr(FileNamePart(szName), '.') == NULL)
	{
		return true;
	}
	for (i = 0; aszExtensions[i]; i++)
	{
		if (cFileUtil.IsExtension((char*)szName, (char*)aszExtensions[i]))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The directory's headers (IsHeaderName).
//
//////////////////////////////////////////////////////////////////////////
void CHeaderFiles::AddFiles(bool bIncludeSubDirectories)
{
	CArrayChars			aszFileNames;
	CFileUtil			cFileUtil;
	char*				szFile;
	const char*			szName;
	size				iBaseDirLen;
	size				j;

	iBaseDirLen = mszBaseDirectory.Length();
	aszFileNames.Init();
	cFileUtil.FindAllFiles(mszBaseDirectory.Text(), &aszFileNames, bIncludeSubDirectories, false);
	for (j = 0; j < aszFileNames.NumElements(); j++)
	{
		szFile = aszFileNames.Get(j)->Text();
		szName = &szFile[iBaseDirLen + 1];
		if (IsHeaderName(szName))
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
	CHeaderFile**	ppcHeaderFile;

	if (mbSubDirectories)
	{
		//Every header it can have was listed when it was made, and isn't changed after.
		ppcHeaderFile = mcFileNames.Get(szRelativeFileName);
		return ppcHeaderFile ? *ppcHeaderFile : NULL;
	}
	return FindInSubDirectory(szRelativeFileName);
}


//////////////////////////////////////////////////////////////////////////
//
// szRelativeFileName in a directory added without its sub directories.  A
// header in it was listed when it was made; one in a sub directory
// ("BaseLib/Chars.h") is the header if the file is there, remembered
// either way so the disk is looked at once for each name.  Units compiled
// at the same time ask, so the project's header map's lock is held.  A
// name that leaves the directory ("../x.h", "/x.h") isn't looked for.
//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CHeaderFiles::FindInSubDirectory(char* szRelativeFileName)
{
	CHeaderFile**	ppcHeaderFile;
	CHeaderFile*	pcHeaderFile;
	CFileUtil		cFileUtil;
	CChars			szFullName;

	if ((szRelativeFileName[0] == '/') || (strstr(szRelativeFileName, "..") != NULL) || (strchr(szRelativeFileName, ':') != NULL))
	{
		return NULL;
	}

	mpcFileMap->Lock();
	ppcHeaderFile = mcFileNames.Get(szRelativeFileName);
	if (ppcHeaderFile)
	{
		pcHeaderFile = *ppcHeaderFile;
	}
	else
	{
		pcHeaderFile = NULL;
		if ((strchr(szRelativeFileName, '/') != NULL) && IsHeaderName(szRelativeFileName))
		{
			szFullName.Init(mszBaseDirectory);
			cFileUtil.AppendToPath(&szFullName, szRelativeFileName);
			if (cFileUtil.Exists(szFullName.Text()) && !cFileUtil.IsDirectory(szFullName.Text()))
			{
				pcHeaderFile = AddFile(szRelativeFileName);
			}
			szFullName.Kill();
		}
		if (!pcHeaderFile)
		{
			mcFileNames.Put(szRelativeFileName, &pcHeaderFile);
		}
	}
	mpcFileMap->Unlock();
	return pcHeaderFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHeaderFiles::HasSubDirectories(void)
{
	return mbSubDirectories;
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

