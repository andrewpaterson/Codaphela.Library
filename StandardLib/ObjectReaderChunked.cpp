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
#include "ChunkFileNames.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "ObjectReaderChunked.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunked::Init(char* szDirectory, char* szChunkFileName)
{
	CObjectReader::Init(szDirectory);
	mszFileName.Init(szChunkFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReaderChunked::Kill(void)
{
	mszFileName.Kill();
	CObjectReader::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderChunked::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFileName;

	CObjectReader::Begin();

	cFileUtil.MakeDir(mszFullDirectory.Text());
	szFileName.Init(mszFullDirectory);

	cFileUtil.AppendToPath(&szFileName, mszFileName.Text());
	szFileName.Append(".");
	szFileName.Append(OBJECT_FILE_EXTENSION);

	pcDiskFile = DiskFile(szFileName.Text());
	szFileName.Kill();

	mcChunkFile.Init(pcDiskFile);
	return mcChunkFile.WriteOpen(CHUNKED_OBJECT_FILE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReaderChunked::End(void)
{
	mcChunkFile.ReadClose();
	mcChunkFile.Kill();

	return CObjectReader::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CObjectReaderChunked::Read(char* szChunkName)
{
	int					iLength;
	CSerialisedObject*	pcSerialised;
	void*				pvOffset;
	BOOL				bResult;

	if (!mcChunkFile.ReadChunkBegin(szChunkName))
	{
		return NULL;
	}
	if (!mcChunkFile.ReadInt(&iLength))
	{
		return NULL;
	}
	if (iLength <= 0)
	{
		return NULL;
	}

	pcSerialised = (CSerialisedObject*)malloc(iLength);
	if (!pcSerialised)
	{
		return NULL;
	}

	pcSerialised->SetLength(iLength);
	iLength -= sizeof(int);

	pvOffset = RemapSinglePointer(pcSerialised, sizeof(int));
	bResult = mcChunkFile.ReadData(pvOffset, iLength);
	if (!bResult)
	{
		free(pcSerialised);
		return NULL;
	}
	if (!mcChunkFile.ReadChunkEnd())
	{
		free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}

