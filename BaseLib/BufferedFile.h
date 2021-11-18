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
#ifndef __BUFFERED_FILE_H__
#define __BUFFERED_FILE_H__
#include "ArrayChar.h"
#include "AbstractFile.h"


#define BUFFERED_FILE_CACHE_SIZE	(64 KB)


enum EBufferType
{
	BT_Empty,
	BT_Read,
	BT_Write,
};

enum EBufferLastOperation
{
	BLO_None,
	BLO_Seek,
	BLO_Read,
	BLO_Write,
};


class CBufferedFile : public CAbstractFile
{
public:
	CAbstractFile*			mpcFile;

	void*					mpvMem;
	size_t					miAllocatedSize;

	size_t					miBufferSizeRead;
	size_t					miBufferPos;
	EBufferType				meType;
	filePos					mulliFilePos;
	filePos					mulliCurrentPos;
	filePos					mulliLength;
	EBufferLastOperation	meLastOp;

	void		Init(CAbstractFile* pcFile);
	void		Init(CAbstractFile* pcFile, int iBufferSize);
	void		Kill(void);

	BOOL		Open(EFileMode eFileMode);
	BOOL		Close(void);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);
	BOOL		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos		Tell(void);
	BOOL		IsOpen(void);
	filePos		Size(void);
	BOOL		Truncate(filePos iSize);
	BOOL		Flush(void);
	BOOL		Delete(void);
	char*		GetFileName(void);

	void		MatchFilePosToCurrentPos(void);
	void		BufferSourceFileRead(void);
	void		CopyFromBuffer(void* pvDest, size_t iByteSize, size_t iDestOffset);
	BOOL		WriteUnwritten(void);
};


//Helper function to make creating basic files easier.
CBufferedFile* BufferedFile(CAbstractFile* pcFile);
CBufferedFile* BufferedFile(CAbstractFile* pcFile, int iBufferSize);


#endif // !__BUFFERED_FILE_H__

