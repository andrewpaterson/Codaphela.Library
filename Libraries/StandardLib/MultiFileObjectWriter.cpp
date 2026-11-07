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
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "MultiFileObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiFileObjectWriter::Init(const char* szDirectory, const char* szBaseName)
{
	CExternalObjectWriter::Init(szBaseName);
	mszDirectory.Init(szDirectory);
	mszObjectBaseName.Init(szBaseName);

	mszObjectBaseName.Replace("\\", "/");
	if (mszObjectBaseName.EndsWith("/"))
	{
		mszObjectBaseName.RemoveLastCharacter();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiFileObjectWriter::Kill(void)
{
	mszObjectBaseName.Kill();
	mszDirectory.Kill();
	CExternalObjectWriter::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMultiFileObjectWriter::Begin(void)
{
	return CExternalObjectWriter::Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMultiFileObjectWriter::End(void)
{
	return CExternalObjectWriter::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMultiFileObjectWriter::Write(CSerialisedObject* pcSerialised)
{
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	CChars			szFilename;
	CFileBasic		cFile;
	CChars			szUnnamed;
	char*			szName;
	bool			bResult;

	if (pcSerialised->IsNamed())
	{
		szName = pcSerialised->GetName();
		ReturnOnFalse(ObjectStartsWithBaseName(szName));
		Filename(szName, mszDirectory.Text(), &szDirectory, &szFilename);
	}
	else if (pcSerialised->IsIndexed())
	{
		Filename(Unnamed(pcSerialised, &szUnnamed), mszDirectory.Text(), &szDirectory, &szFilename);
		szUnnamed.Kill();
	}
	else
	{
		return false;
	}

	cFileUtil.MakeDir(szDirectory.Text());
	cFile.Init(DiskFile(szFilename.Text()));
	szFilename.Kill();
	szDirectory.Kill();

	if (!cFile.Open(EFM_Write_Replace))
	{
		gcLogger.Error2(__METHOD__, " Could not open file [", cFile.GetFilename(), "] for writing.", NULL);
		cFile.Kill();
		return false;
	}

	//The file type identifier, then the object.
	bResult = cFile.WriteData(OBJECT_FILE_EXTENSION, 4);
	bResult = bResult && cFile.WriteInt32(BASIC_OBJECT_FILE);
	bResult = bResult && cFile.WriteData(pcSerialised, pcSerialised->GetLength());

	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMultiFileObjectWriter::ObjectStartsWithBaseName(char* szObjectName)
{
	CChars	szRemainingName;

	szRemainingName.Fake(szObjectName);
	return szRemainingName.StartsWith(mszObjectBaseName.Text());
}

