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
#include <string.h>
#include "StringHelper.h"
#include "FileIO.h"
#include "Logger.h"
#include "FileHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::Save(CFileWriter* pcWriter, const char* szWriterType, const char* szWriterVersion)
{
	if (!CheckHeaderSize(__METHOD__))
	{
		return FALSE;
	}

	memset(this, 0, sizeof(CFileHeader));

	StrCpySafe(mszCodaphela, ENGINE_NAME, FILE_HEADER_CODAPHELA_SIZE);
	StrCpySafe(mszEngineVersion, ENGINE_VERSION, FILE_HEADER_ENGINE_VERSION_SIZE);
	StrCpySafe(mszWriterType, szWriterType, FILE_HEADER_WRITER_TYPE_SIZE);
	StrCpySafe(mszWriterVersion, szWriterVersion, FILE_HEADER_WRITER_VERSION_SIZE);

	return pcWriter->WriteData(this, sizeof(CFileHeader));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::Load(CFileReader* pcReader, const char* szExpectedWriterType, const char* szExpectedWriterVersion)
{
	if (!CheckHeaderSize(__METHOD__))
	{
		return FALSE;
	}

	if (!pcReader->ReadData(this, sizeof(CFileHeader)))
	{
		gcLogger.Error2(__METHOD__, " Could not read file header of [", szExpectedWriterType, "] for file version [", szExpectedWriterVersion, "].", NULL);
		return FALSE;
	}

	if (!IsWriterVersion(szExpectedWriterVersion))
	{
		gcLogger.Error2(__METHOD__, " File version mismatch.  Expected [", szExpectedWriterVersion, "] but read [", mszWriterVersion, "].", NULL);
		return FALSE;
	}

	if (!IsWriterType(szExpectedWriterType))
	{
		gcLogger.Error2(__METHOD__, " File type mismatch.  Expected [", szExpectedWriterType, "] but read [", mszWriterType, "].", NULL);
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
 char* CFileHeader::GetWriterVersion(void)
{
	return mszWriterVersion;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
 char* CFileHeader::GetWriterType(void)
{
	return mszWriterType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::IsWriterType(const char* szWriterType)
{
	return strcmp(mszWriterType, szWriterType) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::IsEngineVersion(const char* szEngineVersion)
{
	return strcmp(mszEngineVersion, szEngineVersion) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::IsWriterVersion(const char* szWriterVersion)
{
	return strcmp(mszWriterVersion, szWriterVersion) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileHeader::CheckHeaderSize(const char* szMethod)
{
	size_t		tFileHeaderSize;

	tFileHeaderSize = sizeof(CFileHeader);
	if (tFileHeaderSize != FILE_HEADER_SIZE)
	{
		gcLogger.Error2(szMethod, " File header memory size [", IntToString(tFileHeaderSize), "] different to expected size [", IntToString(FILE_HEADER_SIZE), "].", NULL);
		return FALSE;
	}
	return TRUE;
}

