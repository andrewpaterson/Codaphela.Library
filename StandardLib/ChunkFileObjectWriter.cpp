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
#include "BaseLib/Logger.h"
#include "ChunkFileNames.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "ChunkFileObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileObjectWriter::Init(char* szDirectory, char* szBaseName, char* szChunkFileName)
{
	CObjectWriter::Init(szDirectory);
	mszDirectory.Init(szDirectory);
	mszFileName.Init(szChunkFileName);
	mszObjectBaseName.Init(szBaseName);

	mszObjectBaseName.Replace("\\", "/");
	if (mszObjectBaseName.EndsWith("/"))
	{
		mszObjectBaseName.RemoveLastCharacter();
	}

	mcChunkFile._Init();
	mcChunkFileNames._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChunkFileObjectWriter::Kill(void)
{
	mszObjectBaseName.Kill();
	mszFileName.Kill();
	mszDirectory.Kill();
	CObjectWriter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileObjectWriter::Begin(void)
{
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	CChars		szFullDirectory;
	CChars		szFileName;

	CObjectWriter::Begin();

	szFullDirectory.Init(mszDirectory);
	cFileUtil.AppendToPath(&szFullDirectory, mszObjectBaseName.Text());

	if (!cFileUtil.Exists(szFullDirectory.Text()))
	{
		if (!cFileUtil.TouchDir(szFullDirectory.Text()))
		{
			gcLogger.Error2(__METHOD__, " Could not create directory [", szFullDirectory.Text(), "].", NULL);
		}
	}
	szFileName.Init(szFullDirectory);
	szFullDirectory.Kill();

	cFileUtil.AppendToPath(&szFileName, mszFileName.Text());
	szFileName.Append(".");
	szFileName.Append(OBJECT_FILE_EXTENSION);

	pcDiskFile = DiskFile(szFileName.Text());
	szFileName.Kill();

	mcChunkFile.Init(pcDiskFile);
	mcChunkFileNames.Init(&mcChunkFile);
	return mcChunkFileNames.WriteOpen(CHUNKED_OBJECT_FILE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileObjectWriter::End(void)
{
	mcChunkFileNames.WriteClose();
	mcChunkFileNames.Kill();
	mcChunkFile.Kill();

	return CObjectWriter::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileObjectWriter::Write(CSerialisedObject* pcSerialised)
{
	CChars	szChunkName;
	BOOL	bResult;

	if (pcSerialised->IsNamed())
	{
		bResult = ObjectStartsWithBaseName(pcSerialised->GetName());
		if (!bResult)
		{
			return FALSE;
		}
		RemainingName(&szChunkName, pcSerialised->GetName());
	}
	else if (pcSerialised->IsIndexed())
	{
		Unnamed(pcSerialised, &szChunkName);
	}
	else
	{
		return FALSE;
	}

	ReturnOnFalse(mcChunkFileNames.WriteChunkBegin(szChunkName.Text()));
	ReturnOnFalse(mcChunkFileNames.WriteData(pcSerialised, pcSerialised->GetLength()));
	ReturnOnFalse(mcChunkFileNames.WriteChunkEnd());

	szChunkName.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChunkFileObjectWriter::ObjectStartsWithBaseName(char* szObjectName)
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

