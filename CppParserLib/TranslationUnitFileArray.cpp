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
#include "TranslationUnitFileArray.h"
#include "BaseLib/FileUtil.h"
#include "Library.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnitFileArray::Init(CLibrary* pcLibrary)
{
	mcFiles.Init();
	mpcLibrary = pcLibrary;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnitFileArray::Kill(void)
{
	CTranslationUnit*	pcFile;

	pcFile = mcFiles.GetHead();
	while (pcFile)
	{
		pcFile->Kill();
		pcFile = mcFiles.GetNext(pcFile);
	}

	mcFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTranslationUnitFileArray::AddAllFiles(BOOL bLogInlucdes, BOOL bLogBlocks)
{
	CChars					szTemp;
	CArrayChars			aszTemp;
	int						i;
	CChars*					pszFile;
	int						iIndex;
	int						j;
	CChars*					szFile;
	char*					szExtension;
	int						iBaseDirLen;
	CFileUtil				cFileUtil;
	CArrayChars			aszFileNames;

	szTemp.Init("*.c;*.cpp");

	aszTemp.Init();
	szTemp.Split(&aszTemp, ';');
	iBaseDirLen = mpcLibrary->mszBaseDir.Length();

	for (i = 0; i < aszTemp.NumElements(); i++)
	{
		pszFile = aszTemp.Get(i);
		pszFile->StripWhiteSpace();

		iIndex = pszFile->Find(0, ".");
		szExtension = pszFile->Text(iIndex+1);

		aszFileNames.Init();
		cFileUtil.FindFilesWithExtension(mpcLibrary->mszBaseDir.Text(), szExtension, &aszFileNames);

		for (j = 0; j < aszFileNames.NumElements(); j++)
		{
			szFile = aszFileNames.Get(j);
			AddFile(szFile->Text(iBaseDirLen+1), bLogInlucdes, bLogBlocks);
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
CTranslationUnit* CTranslationUnitFileArray::AddFile(char* szRelativeFileName, BOOL bLogIncludes, BOOL bLogBlocks)
{
	CChars				szTemp;
	CTranslationUnit*	pcTranslationUnit;
	CTranslationUnit	cTranslationUnit;

	szTemp.Init(mpcLibrary->mszBaseDir);
	szTemp.Append(FILE_SEPARATOR);
	szTemp.Append(szRelativeFileName);
	cTranslationUnit.Init(szTemp.Text(), mpcLibrary, bLogIncludes, bLogBlocks);
	szTemp.Kill();

	pcTranslationUnit = mcFiles.InsertAfterTail();
	memcpy(pcTranslationUnit, &cTranslationUnit, sizeof(CTranslationUnit));

	return pcTranslationUnit;
}
