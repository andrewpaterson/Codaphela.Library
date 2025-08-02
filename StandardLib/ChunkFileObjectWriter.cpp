/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/ChunkFileFileSystem.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "ChunkFileObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileObjectWriter::Init(char* szDirectory, char* szBaseName, char* szChunkFilename)
{
	CExternalObjectWriter::Init(szDirectory);
	mszDirectory.Init(szDirectory);
	mszFilename.Init(szChunkFilename);
	mszObjectBaseName.Init(szBaseName);

	mszObjectBaseName.Replace("\\", "/");
	if (mszObjectBaseName.EndsWith("/"))
	{
		mszObjectBaseName.RemoveLastCharacter();
	}

	mcChunkFile._Init();
	mcChunkFileFileSystem._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileObjectWriter::Kill(void)
{
	mszObjectBaseName.Kill();
	mszFilename.Kill();
	mszDirectory.Kill();
	CExternalObjectWriter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileObjectWriter::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFullDirectory;
	CChars		szFilename;

	CExternalObjectWriter::Begin();

	szFullDirectory.Init(mszDirectory);
	cFileUtil.AppendToPath(&szFullDirectory, mszObjectBaseName.Text());

	if (!cFileUtil.Exists(szFullDirectory.Text()))
	{
		if (!cFileUtil.TouchDir(szFullDirectory.Text()))
		{
			gcLogger.Error2(__METHOD__, " Could not create directory [", szFullDirectory.Text(), "].", NULL);
		}
	}
	szFilename.Init(szFullDirectory);
	szFullDirectory.Kill();

	cFileUtil.AppendToPath(&szFilename, mszFilename.Text());
	szFilename.Append(".");
	szFilename.Append(OBJECT_FILE_EXTENSION);

	pcDiskFile = DiskFile(szFilename.Text());
	szFilename.Kill();

	mcChunkFile.Init(pcDiskFile);
	mcChunkFileFileSystem.Init(&mcChunkFile);
	return mcChunkFileFileSystem.WriteOpen(CHUNKED_OBJECT_FILE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileObjectWriter::End(void)
{
	mcChunkFileFileSystem.WriteClose();
	mcChunkFileFileSystem.Kill();
	mcChunkFile.Kill();

	return CExternalObjectWriter::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileObjectWriter::Write(CSerialisedObject* pcSerialised)
{
	CChars	szChunkName;
	bool	bResult;
	char*	szName;

	if (pcSerialised->IsNamed())
	{
		szName = pcSerialised->GetName();
		bResult = ObjectStartsWithBaseName(szName);
		if (!bResult)
		{
			return false;
		}
		RemainingName(&szChunkName, szName);
	}
	else if (pcSerialised->IsIndexed())
	{
		Unnamed(pcSerialised, &szChunkName);
	}
	else
	{
		return false;
	}

	ReturnOnFalse(mcChunkFileFileSystem.WriteChunkBegin(szChunkName.Text()));
	ReturnOnFalse(mcChunkFileFileSystem.WriteData(pcSerialised, pcSerialised->GetLength()));
	ReturnOnFalse(mcChunkFileFileSystem.WriteChunkEnd());

	szChunkName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChunkFileObjectWriter::ObjectStartsWithBaseName(char* szObjectName)
{
	CChars	szRemainingName;

	szRemainingName.Fake(szObjectName);
	return szRemainingName.StartsWith(mszObjectBaseName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileObjectWriter::RemainingName(CChars* pszRemainingName, char* szObjectName)
{
	pszRemainingName->Init(szObjectName);
	pszRemainingName->RemoveFromStart(mszObjectBaseName.Length());
	if (pszRemainingName->StartsWith("/"))
	{
		pszRemainingName->RemoveCharacter(0);
	}
}

