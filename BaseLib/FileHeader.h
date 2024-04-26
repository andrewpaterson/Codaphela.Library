/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __FILE_HEADER_H__
#define __FILE_HEADER_H__
#include "Version.h"
#include "Define.h"


#define FILE_HEADER_SIZE 256
#define FILE_HEADER_CODAPHELA_SIZE 10
#define FILE_HEADER_ENGINE_VERSION_SIZE 30
#define FILE_HEADER_WRITER_TYPE_SIZE 108
#define FILE_HEADER_WRITER_VERSION_SIZE 108


class CFileWriter;
class CFileReader;
class CFileHeader
{
protected:
	char	mszCodaphela[FILE_HEADER_CODAPHELA_SIZE];
	char	mszEngineVersion[FILE_HEADER_ENGINE_VERSION_SIZE];
	char	mszWriterType[FILE_HEADER_WRITER_TYPE_SIZE];
	char	mszWriterVersion[FILE_HEADER_WRITER_VERSION_SIZE];

public:
	bool	Load(CFileReader* pcReader, const char* szExpectedWriterType, const char* szExpectedWriterVersion);
	bool	Save(CFileWriter* pcWriter, const char* szWriterType, const char* szWriterVersion);

	char*	GetEngineVersion(void);
	char*	GetWriterVersion(void);
	char*	GetWriterType(void);

	bool	IsWriterType(const char* szWriterType);
	bool	IsEngineVersion(const char* szEngineVersion);
	bool	IsWriterVersion(const char* szEngineVersion);

protected:
	bool	CheckHeaderSize(const char* szMethod);
};


#endif // __FILE_HEADER_H__

