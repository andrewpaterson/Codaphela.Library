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
#include "ObjectReaderChunkFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunkFile::Init(CChunkFileFileSystem* pcChunkFile)
{
	mpcChunkFile = pcChunkFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunkFile::Kill(void)
{
	CObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderChunkFile::Read(OIndex oi)
{
	CChars				szUnnamed;
	CSerialisedObject*	pcSerialised;

	szUnnamed.Init(OBJECT_UNNAMED_FILE"/");
	szUnnamed.AppendHexHiLo(&oi, sizeof(OIndex));

	pcSerialised = Read(szUnnamed.Text());

	szUnnamed.Kill();
	return pcSerialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderChunkFile::Read(char* szChunkName)
{
	CSerialisedObject*	pcSerialised;
	CChunkFileFile		cChunkFile;
	CFileBasic			cFileBasic;

	if (!mpcChunkFile->ReadChunkBegin(szChunkName))
	{
		return NULL;
	}

	cChunkFile.Init(mpcChunkFile->GetChunkFile());
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

	if (!mpcChunkFile->ReadChunkEnd())
	{
		free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}

