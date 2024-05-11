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
#ifndef __DISK_FILE_H__
#define __DISK_FILE_H__
#include <stdio.h>
#include "Chars.h"
#include "ArrayChar.h"
#include "AbstractFile.h"


class CDiskFile : public CAbstractFile
{
public:
	uint8		maFile[8];
	CChars		mszFilename;

	void		Init(const char* szFilename);
	void		Init(CChars szFilename);
	void		Kill(void);

	bool		Open(EFileMode eMode);
	bool		Close(void);
	size		Read(void* pvBuffer, size iSize, size iCount);
	bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	size		Write(const void* pvBuffer, size iSize, size iCount);
	filePos		Tell(void);
	bool		IsOpen(void);
	filePos		Size(void);
	bool		Truncate(filePos iSize);
	bool		Flush(void);
	bool		Delete(void);
	char*		GetFilename(void);

	void		SetFile(void* pvFile, size uiSize);
	bool		IsFile(void* pvFile, size uiSize);
	void*		GetFile(void);
};


//Helper function to make creating basic files easier.
CDiskFile* DiskFile(const char* szName);


#endif // __DISK_FILE_H__

