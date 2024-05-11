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
#ifndef __CHUNK_FILE_FILE_H__
#define __CHUNK_FILE_FILE_H__
#include <stdio.h>
#include "AbstractFile.h"


class CChunkFile;
class CChunkFileFile : public CAbstractFile
{
protected:
	CChunkFile*		mpcChunkFile;
	filePos			miChunkStart;
	filePos			miChunkSize;

	bool			mbEndOfFile;
	bool			mbWriteMode;
	bool			mbReadMode;

public:
	void		Init(CChunkFile* pcChunkFile);
	void		Kill(void);

	bool		Open(EFileMode eMode);
	bool		Close(void);
	size		Read(void* pvBuffer, size iSize, size iCount);
	bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	size		Write(const void* pvBuffer, size iSize, size iCount);
	filePos		Tell(void);
	bool		IsOpen(void);
	bool		Truncate(filePos iSize);
	filePos		Size(void);
	bool		Flush(void);
	bool		Delete(void);
	char*		GetFilename(void);
};


#endif // __CHUNK_FILE_FILE_H__

