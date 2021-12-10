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
#include "BaseLib/TextFile.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/FileUtil.h"
#include "Library.h"
#include "SourceFile.h"
#include "PPTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Init(char* szName)
{
	mszContents.Init();
	mszFullName.Init(szName);
	mbLoaded = FALSE;
	mcTokenMemory.Init();
	macBlockSets.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Kill(void)
{
	macBlockSets.Kill();
	mcTokenMemory.Kill();
	mszFullName.Kill();
	mszContents.Kill();
	mbLoaded = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Load(void)
{
	CTextFile	cFile;

	if (!mbLoaded)
	{
		mbLoaded = TRUE;

		cFile.Init();
		cFile.Read(mszFullName.Text());

		mszContents.Kill();
		mszContents.Init(cFile.Text());

		cFile.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CSourceFile::IsLoaded(void)
{
	return mbLoaded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CSourceFile::IsNamed(char* szName)
{
	return (strcmp(ShortName(), szName) == 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::SetContents(const char* sz)
{
	mszContents.Set(sz);
	mbLoaded = TRUE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::ShortName(void)
{
	int		iIndex;

	iIndex = mszFullName.FindFromEnd(FILE_SEPARATOR);
	if (iIndex == -1)
	{
		return mszFullName.Text();
	}
	else
	{
		return mszFullName.Text(iIndex+1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::FullName(void)
{
	return mszFullName.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::DumpBlockSets(void)
{
	macBlockSets.DumpBlockSets();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Path(CChars* pszPath)
{
	CFileUtil	cFileUtil;

	pszPath->Init(mszFullName);
	cFileUtil.RemoveLastFromPath(pszPath);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::GetContents(void)
{
	return mszContents.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CSourceFile::GetContentsLength(void)
{
	return mszContents.Length();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPTokenMemory* CSourceFile::GetTokenMemory(void)
{
	return &mcTokenMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPBlockSetArray* CSourceFile::GetBlockSets(void)
{
	return &macBlockSets;
}

