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
#include "ObjectFileGeneral.h"
#include "SerialisedObject.h"
#include "MultiFileObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiFileObjectWriter::Init(char* szDirectory, char* szBaseName)
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
BOOL CMultiFileObjectWriter::Begin(void)
{
	return CExternalObjectWriter::Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMultiFileObjectWriter::End(void)
{
	return CExternalObjectWriter::End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMultiFileObjectWriter::Write(CSerialisedObject* pcSerialised)
{
	CFileUtil		cFileUtil;
	CChars			szDirectory;
	CChars			szFileName;
	CFileBasic		cFile;
	char*			szExtension;
	CChars			szUnnamed;

	if (pcSerialised->IsNamed())
	{
		ReturnOnFalse(ObjectStartsWithBaseName(pcSerialised->GetName()));
		FileName(pcSerialised->GetName(), mszDirectory.Text(), &szDirectory, &szFileName);
	}
	else if (pcSerialised->IsIndexed())
	{
		FileName(Unnamed(pcSerialised, &szUnnamed), mszDirectory.Text(), &szDirectory, &szFileName);
		szUnnamed.Kill();
	}
	else
	{
		return FALSE;
	}

	//What the actual fuck is going on in this method?
	ReturnOnFalse(ObjectStartsWithBaseName(pcSerialised->GetName()));

	cFileUtil.MakeDir(szDirectory.Text());
	cFile.Init(DiskFile(szFileName.Text()));
	szFileName.Kill();
	szDirectory.Kill();

	cFile.Open(EFM_Write_Create);

	//Write file type identifier.
	szExtension = OBJECT_FILE_EXTENSION;
	ReturnOnFalse(cFile.WriteData(szExtension, 4));
	ReturnOnFalse(cFile.WriteInt(BASIC_OBJECT_FILE));

	ReturnOnFalse(cFile.WriteData(pcSerialised, pcSerialised->GetLength()));

	cFile.Close();
	cFile.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMultiFileObjectWriter::ObjectStartsWithBaseName(char* szObjectName)
{
	CChars	szRemainingName;

	szRemainingName.Fake(szObjectName);
	return szRemainingName.StartsWith(mszObjectBaseName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMultiFileObjectWriter::RemainingName(CChars* pszRemainingName, char* szObjectName)
{
	pszRemainingName->Init(szObjectName);
	pszRemainingName->RemoveFromStart(mszObjectBaseName.Length());
	if (pszRemainingName->StartsWith("/"))
	{
		pszRemainingName->RemoveCharacter(0);
	}
}

