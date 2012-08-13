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
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFullDirectory;

	mszDirectory.Init(szDirectory);
	mszBaseName.Init(szBaseName);
	if (mszBaseName.EndsWith("/"))
	{
		mszBaseName.RemoveLastCharacter();
	}

	szFullDirectory.Init(szDirectory);
	cFileUtil.AppendToPath(&szFullDirectory, szBaseName);

	pcDiskFile = DiskFile(szFullDirectory.Text());
	szFullDirectory.Kill();

	mcChunkFile.Init(pcDiskFile);
	mcChunkFile.WriteOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterChunked::Kill(void)
{
	mcChunkFile.WriteClose();
	mcChunkFile.Kill();

	mszDirectory.Kill();
	mszBaseName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterChunked::Write(OIndex oi, char* szObjectName, void* pvObject, int iLength)
{
	CChars	szRemainingName;
	BOOL	bResult;

	szRemainingName.Init(szObjectName);
	bResult = szRemainingName.StartsWith(mszBaseName.Text());
	if (!bResult)
	{
		szRemainingName.Kill();
		return FALSE;
	}

	szRemainingName.RemoveFromStart(mszBaseName.Length());
	if (szRemainingName.StartsWith("/"))
	{
		szRemainingName.RemoveCharacter(0);
	}

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

