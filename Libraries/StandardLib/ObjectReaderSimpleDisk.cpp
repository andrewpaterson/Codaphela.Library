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
#include "BaseLib/Logger.h"
#include "BaseLib/DiskFile.h"
#include "ObjectReaderSimpleDisk.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Init(const char* szDirectory)
{
	CFileObjectReader::Init(&mcFile);
	mszFullDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderSimpleDisk::Kill(void)
{
	mszFullDirectory.Kill();
	CFileObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderSimpleDisk::Read(const char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	CChars				szDirectory;
	CChars				szFilename;

	Filename(szObjectName, mszFullDirectory.Text(), &szDirectory, &szFilename);
	mcFile.Init(DiskFile(szFilename.Text()));
	szFilename.Kill();
	szDirectory.Kill();

	if (!mcFile.Open(EFM_Read))
	{
		gcLogger.Error2(__METHOD__, " Could not open file [", mcFile.GetFilename(), "] for reading.", NULL);
		mcFile.Kill();
		return NULL;
	}

	pcSerialised = ReadOpenedFile(&mcFile);
	mcFile.Close();
	mcFile.Kill();
	return pcSerialised;
}

