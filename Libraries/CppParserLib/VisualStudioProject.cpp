/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "VisualStudioProject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVisualStudioProject::Init(void)
{
	mszDirectory.Init();
	maszSources.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CVisualStudioProject::Kill(void)
{
	maszSources.Kill();
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the project's sources, which are in the project's directory.  Returns false if the file can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CVisualStudioProject::Read(const char* szProjectFile)
{
	CTextFile	cFile;
	CFileUtil	cFileUtil;
	CChars		szSource;
	const char*	szText;
	const char*	szStart;
	const char*	szEnd;
	const char*	szTag;

	mszDirectory.Kill();
	mszDirectory.Init(szProjectFile);
	cFileUtil.FullPath(&mszDirectory);
	cFileUtil.RemoveLastFromPath(&mszDirectory);

	cFile.Init();
	if (!cFile.Read((char*)szProjectFile))
	{
		cFile.Kill();
		return false;
	}

	szTag = "<ClCompile Include=\"";
	szText = cFile.Text();
	for (szStart = strstr(szText, szTag); szStart; szStart = strstr(szEnd, szTag))
	{
		szStart += strlen(szTag);
		szEnd = strchr(szStart, '"');
		if (!szEnd)
		{
			break;
		}
		szSource.Init();
		szSource.Append((char*)szStart, (int)(szEnd - szStart));
		szSource.Replace('\\', FILE_SEPARATOR[0]);
		MatchCase(&szSource);
		maszSources.Add(szSource.Text());
		szSource.Kill();
	}
	cFile.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pszSource (relative to the project's directory) named as its directories
// and file are, a part at a time: each is the entry of the directory it is
// in that is it ignoring case.  A part nothing matches (a file that isn't
// there) is left as it was written.
//
//////////////////////////////////////////////////////////////////////////
void CVisualStudioProject::MatchCase(CChars* pszSource)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszParts;
	CArrayChars	aszEntries;
	CChars		szDirectory;
	CChars		szMatched;
	CChars		szName;
	char*		szPart;
	size		i;
	size		j;

	aszParts.Init();
	pszSource->Split(&aszParts, FILE_SEPARATOR[0]);
	szDirectory.Init(mszDirectory);
	szMatched.Init();
	for (i = 0; i < aszParts.NumElements(); i++)
	{
		szPart = aszParts.GetText(i);
		szName.Init(szPart);
		if ((strcmp(szPart, ".") != 0) && (strcmp(szPart, "..") != 0))
		{
			aszEntries.Init();
			cFileUtil.FindAllDirectories(szDirectory.Text(), &aszEntries, true);
			cFileUtil.FindAllFiles(szDirectory.Text(), &aszEntries, false, true);
			for (j = 0; j < aszEntries.NumElements(); j++)
			{
				szName.Kill();
				szName.Init(aszEntries.GetText(j));
				cFileUtil.RemovePath(&szName);
				if (szName.EqualsIgnoreCase(szPart))
				{
					break;
				}
			}
			if (j == aszEntries.NumElements())
			{
				szName.Kill();
				szName.Init(szPart);
			}
			aszEntries.Kill();
		}
		if (i > 0)
		{
			szMatched.Append(FILE_SEPARATOR[0]);
		}
		szMatched.Append(szName);
		cFileUtil.AppendToPath(&szDirectory, szName.Text());
		szName.Kill();
	}
	pszSource->Kill();
	pszSource->Init(szMatched);
	szMatched.Kill();
	szDirectory.Kill();
	aszParts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CVisualStudioProject::GetDirectory(void)
{
	return mszDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CVisualStudioProject::NumSources(void)
{
	return maszSources.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CVisualStudioProject::GetSource(size uiIndex)
{
	return maszSources.GetText(uiIndex);
}

