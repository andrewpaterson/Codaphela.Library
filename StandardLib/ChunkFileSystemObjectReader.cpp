/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/ChunkFileFile.h"
#include "BaseLib/ChunkFileFileSystem.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "ChunkFileSystemObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileSystemObjectReader::Init(char* szDirectory, char* szChunkFilename)
{
	CChunkFileObjectReader::Init(&mcChunkFileFileSystem);
	mszFilename.Init(szChunkFilename);
	mszFullDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileSystemObjectReader::Kill(void)
{
	mszFullDirectory.Kill();
	mszFilename.Kill();
	CExternalObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileSystemObjectReader::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFilename;

	CExternalObjectReader::Begin();

	szFilename.Init(mszFullDirectory);

	cFileUtil.AppendToPath(&szFilename, mszFilename.Text());
	szFilename.Append(".");
	szFilename.Append(OBJECT_FILE_EXTENSION);

	pcDiskFile = DiskFile(szFilename.Text());
	szFilename.Kill();

	mcChunkFile.Init(pcDiskFile);
	mcChunkFileFileSystem.Init(&mcChunkFile);
	return mcChunkFileFileSystem.ReadOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileSystemObjectReader::End(void)
{
	mcChunkFileFileSystem.ReadClose();
	mcChunkFileFileSystem.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CChunkFileSystemObjectReader::Read(char* szChunkName)
{
	CSerialisedObject*	pcSerialised;
	CChunkFileFile		cChunkFile;
	CFileBasic			cFileBasic;

	if (!mcChunkFileFileSystem.ReadChunkBegin(szChunkName))
	{
		return NULL;
	}

	cChunkFile.Init(mcChunkFileFileSystem.GetChunkFile());
	cFileBasic.Init(&cChunkFile);
	cFileBasic.Open(EFM_Read);

	pcSerialised = ReadSerialised(&cFileBasic);

	cFileBasic.Close();
	cFileBasic.Kill();
	cChunkFile.Kill();

	if (!pcSerialised)
	{
		return NULL;
	}

	if (!mcChunkFileFileSystem.ReadChunkEnd())
	{
		free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}

