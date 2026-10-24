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
	CArrayChars	aszFiles;
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

	aszFiles.Init();
	cFileUtil.FindAllFiles(mszDirectory.Text(), &aszFiles, false, true);

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
		MatchCase(&aszFiles, &szSource);
		maszSources.Add(szSource.Text());
		szSource.Kill();
	}
	aszFiles.Kill();
	cFile.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The name of the file in the directory that is pszSource ignoring case.
//
//////////////////////////////////////////////////////////////////////////
void CVisualStudioProject::MatchCase(CArrayChars* paszFiles, CChars* pszSource)
{
	CFileUtil	cFileUtil;
	CChars		szName;
	size		i;

	for (i = 0; i < paszFiles->NumElements(); i++)
	{
		szName.Init(paszFiles->GetText(i));
		cFileUtil.RemovePath(&szName);
		if (szName.EqualsIgnoreCase(pszSource->Text()))
		{
			pszSource->Kill();
			pszSource->Init(szName);
			szName.Kill();
			return;
		}
		szName.Kill();
	}
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

