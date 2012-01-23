/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "FileIO.h"
#include "FileHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::Save(CFileWriter* pcWriter, char* szFileType)
{
	int	iLen;

	memset(this, 0, sizeof(CFileHeader));
	strcpy(mszCodaphela, ENGINE_NAME);
	strcpy(mszEngineVersion, ENGINE_VERSION);

	iLen = (int)strlen(szFileType);
	if (iLen >= 146)
	{
		memcpy(mszFileType, szFileType, 145);
	}
	else
	{
		memcpy(mszFileType, szFileType, iLen);
	}

	return pcWriter->Write(this, sizeof(CFileHeader), 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::Load(CFileReader* pcReader)
{
	BOOL	bResult;

	bResult = pcReader->Read(this, sizeof(CFileHeader), 1);
	if (!bResult)
	{
		return FALSE;
	}

	if (strcmp(ENGINE_NAME, mszCodaphela) != 0)
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFileHeader::GetEngineVersion(void)
{
	return mszEngineVersion;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFileHeader::GetFileType(void)
{
	return mszFileType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::IsType(char* szFileType)
{
	return strcmp(mszFileType, szFileType) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::IsVersion(char* szEngineVersion)
{
	return strcmp(mszEngineVersion, szEngineVersion) == 0;
}

