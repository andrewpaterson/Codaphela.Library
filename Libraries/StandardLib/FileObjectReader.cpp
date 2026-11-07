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
#include "BaseLib/DiskFile.h"
#include "FileObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileObjectReader::Init(CFileBasic* pcFile)
{
	mpcFile = pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileObjectReader::Kill(void)
{
	mpcFile = NULL;
	CExternalObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::Read(OIndex oi)
{
	CChars				szUnnamed;
	CSerialisedObject*	pcSerialised;

	pcSerialised = Read(Unnamed(oi, &szUnnamed));
	szUnnamed.Kill();
	return pcSerialised;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::Read(const char* szObjectName)
{
	CSerialisedObject*	pcSerialised;

	//The file holds one object: szObjectName isn't needed to find it.
	if (!mpcFile->Open(EFM_Read))
	{
		return NULL;
	}
	pcSerialised = ReadOpenedFile(mpcFile);
	mpcFile->Close();
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
// The serialised object in an opened object file (the file type, then
// the object); NULL if pcFile isn't one.
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CFileObjectReader::ReadOpenedFile(CFileBasic* pcFile)
{
	char	szExtension[4];
	int32	iFileType;

	if (!pcFile->ReadData(szExtension, 4) || (memcmp(szExtension, OBJECT_FILE_EXTENSION, 4) != 0))
	{
		return NULL;
	}
	if (!pcFile->ReadInt32(&iFileType) || (iFileType != BASIC_OBJECT_FILE))
	{
		return NULL;
	}
	return ReadSerialised(pcFile);
}


