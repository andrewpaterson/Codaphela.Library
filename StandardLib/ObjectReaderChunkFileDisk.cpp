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
#include "ChunkFileNames.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "ObjectReaderChunkFileDisk.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunkFileDisk::Init(char* szDirectory, char* szChunkFileName)
{
	CObjectReaderChunkFile::Init(&mcChunkFileNames);
	mszFileName.Init(szChunkFileName);
	mszFullDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunkFileDisk::Kill(void)
{
	mszFullDirectory.Kill();
	mszFileName.Kill();
	CObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderChunkFileDisk::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFileName;

	CObjectReader::Begin();

	szFileName.Init(mszFullDirectory);

	cFileUtil.AppendToPath(&szFileName, mszFileName.Text());
	szFileName.Append(".");
	szFileName.Append(OBJECT_FILE_EXTENSION);

	pcDiskFile = DiskFile(szFileName.Text());
	szFileName.Kill();

	mcChunkFile.Init(pcDiskFile);
	mcChunkFileNames.Init(&mcChunkFile);
	return mcChunkFileNames.ReadOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderChunkFileDisk::End(void)
{
	mcChunkFileNames.ReadClose();
	mcChunkFileNames.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderChunkFileDisk::Read(char* szChunkName)
{
	CSerialisedObject*	pcSerialised;
	CChunkFileFile		cChunkFile;
	CFileBasic			cFileBasic;

	if (!mcChunkFileNames.ReadChunkBegin(szChunkName))
	{
		return NULL;
	}

	cChunkFile.Init(mcChunkFileNames.GetChunkFile());
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

	if (!mcChunkFileNames.ReadChunkEnd())
	{
		free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}

