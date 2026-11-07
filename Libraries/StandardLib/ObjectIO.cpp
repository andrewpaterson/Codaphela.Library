/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Chars.h"
#include "BaseLib/FileUtil.h"
#include "SerialisedObject.h"
#include "ObjectFileGeneral.h"
#include "ObjectIO.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(OIndex oi, CChars* pszDestName)
{
	pszDestName->Init(OBJECT_UNNAMED_FILE"/");
	pszDestName->AppendHexHiLo(&oi, sizeof(OIndex));
	return pszDestName->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Unnamed(CSerialisedObject* pcSerialised, CChars* pszDestName)
{
	return Unnamed(pcSerialised->GetIndex(), pszDestName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CObjectIO::Filename(const char* szSerialisedName, const char* szWorkingDirectory, CChars* pszDestDirectory, CChars* pszDestFullFilename)
{
	CFileUtil	cFileUtil;
	CChars		szShortFilename;
	CChars		szDirectory;
	CChars*		pszDirectoryTemp;

	if (pszDestDirectory)
	{
		pszDirectoryTemp = pszDestDirectory;
	}
	else
	{
		pszDirectoryTemp = &szDirectory;
	}

	szShortFilename.Init();
	pszDirectoryTemp->Init();
	cFileUtil.SplitPath(szSerialisedName, &szShortFilename, pszDirectoryTemp);
	szShortFilename.Append(".");
	szShortFilename.Append(OBJECT_FILE_EXTENSION);

	pszDestFullFilename->Init(szWorkingDirectory);
	cFileUtil.AppendToPath(pszDestFullFilename, pszDirectoryTemp->Text());
	pszDirectoryTemp->Set(pszDestFullFilename);

	cFileUtil.AppendToPath(pszDestFullFilename, szShortFilename.Text());
	szShortFilename.Kill();

	if (!pszDestDirectory)
	{
		szDirectory.Kill();
	}

	return pszDestFullFilename->Text();
}

