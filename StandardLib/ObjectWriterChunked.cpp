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
#include "ObjectWriterChunked.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterChunked::Init(char* szDirectory, char* szBaseName)
{
	CObjectWriter::Init(szDirectory, szBaseName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterChunked::Kill(void)
{
	CObjectWriter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterChunked::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFullDirectory;

	CObjectWriter::Begin();

	szFullDirectory.Init(mszDirectory);
	cFileUtil.AppendToPath(&szFullDirectory, mszBaseName.Text());

	pcDiskFile = DiskFile(szFullDirectory.Text());
	szFullDirectory.Kill();

	mcChunkFile.Init(pcDiskFile);
	return mcChunkFile.WriteOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterChunked::End(void)
{
	mcChunkFile.WriteClose();
	mcChunkFile.Kill();

	return CObjectWriter::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterChunked::Write(OIndex oi, char* szObjectName, void* pvObject, int iLength)
{
	CChars	szRemainingName;

	ReturnOnFalse(ObjectStartsWithBase(szObjectName));
	RemainingName(&szRemainingName, szObjectName);

	ReturnOnFalse(mcChunkFile.WriteChunkBegin(szRemainingName.Text()));

	//Write object identifier.
	ReturnOnFalse(mcChunkFile.WriteLong(oi));
	ReturnOnFalse(mcChunkFile.WriteString(szObjectName));

	//Write object stream.
	ReturnOnFalse(mcChunkFile.WriteData(pvObject, iLength));

	ReturnOnFalse(mcChunkFile.WriteChunkEnd());

	szRemainingName.Kill();
	return TRUE;
}

