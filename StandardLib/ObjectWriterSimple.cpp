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
#include "ObjectWriterSimple.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterSimple::Init(char* szDirectory)
{
	mszDirectory.Init(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriterSimple::Kill(void)
{
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriterSimple::Write(OIndex oi, char* szObjectName, char* szClassName, void* pvObject, int iLength)
{
	CFileUtil		cFileUtil;
	CChars			szFileName;
	CChars			szDirectory;
	CFileBasic		cFile;
	CChars			szFullName;

	szFileName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szObjectName, &szFileName, &szDirectory);
	szFileName.Append(".DRG");

	szFullName.Init(mszDirectory);
	szFullName.Append(FILE_SEPARATOR[0]);
	szFullName.Append(szDirectory);

	cFileUtil.MakeDir(szFullName.Text());

	szFullName.Append(FILE_SEPARATOR[0]);
	szFullName.Append(szFileName);

	cFile.Init(DiskFile(szFullName.Text()));
	cFile.Open(EFM_Write_Create);

	ReturnOnFalse(cFile.WriteInt(0));
	ReturnOnFalse(cFile.WriteString(szClassName));
	ReturnOnFalse(cFile.WriteLong(oi));

	ReturnOnFalse(cFile.Write(pvObject, iLength, 1));

	cFile.Close();
	cFile.Kill();

	szFullName.Kill();
	szDirectory.Kill();
	szFileName.Kill();
	return TRUE;
}

