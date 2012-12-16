/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
	FILE*			mpsFileHandle;
	CChunkFile*		mpcChunkFile;
	filePos			miChunkStart;
	filePos			miChunkSize;

	BOOL			mbEndOfFile;

public:
	void	Init(CChunkFile* pcChunkFile);
	void	Kill(void);

	BOOL		Open(EFileMode eMode);
	BOOL		Close(void);
	filePos		Read(void* pvBuffer, filePos iSize, filePos iCount);
	BOOL		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	filePos		Write(const void* pvBuffer, filePos iSize, filePos iCount);
	filePos		Tell(void);
	BOOL		Eof(void);
	BOOL		IsOpen(void);
	filePos		Size(void);
	BOOL		Flush(void);
	BOOL		Delete(void);
};


#endif // __CHUNK_FILE_FILE_H__

