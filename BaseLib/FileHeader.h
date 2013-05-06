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
#ifndef __FILE_HEADER_H__
#define __FILE_HEADER_H__
#include "Version.h"
#include "Bool.h"


class CFileWriter;
class CFileReader;
class CFileHeader
{
protected:
	char	mszCodaphela[10];
	char	mszEngineVersion[100];
	char	mszFileType[146];

public:
	BOOL	Save(CFileWriter* pcWriter, char* szFileType);
	BOOL	Load(CFileReader* pcReader);

	char*	GetEngineVersion(void);
	char*	GetFileType(void);

	BOOL	IsType(char* szFileType);
	BOOL	IsVersion(char* szEngineVersion);
};


#endif // __FILE_HEADER_H__
