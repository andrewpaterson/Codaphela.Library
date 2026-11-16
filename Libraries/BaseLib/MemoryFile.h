/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __MEMORY_FILE_H__
#define __MEMORY_FILE_H__
#include "ArrayChar.h"
#include "AbstractFile.h"


class CMemoryFile : public CAbstractFile
{
protected:
	size		miFlags;
	lsize		miPos;
	bool		mbOpen;
	void*		mpvInitialMem;
	lsize		miInitialLength;
	bool		mbFakeArray;
	CArrayLargeChar	mcArray;

public:
	void		Init(void);
	void		Init(void* pvInitialMem, lsize iInitialLength);
	void		Kill(void) override;

	void*		GetBufferPointer(void) const;
	lsize		GetBufferSize(void) const;
	void		SetBufferPointer(void* pvBuffer);
	void		SetBufferSize(lsize iBufferSize);

	bool		Open(EFileMode eFileMode) override;
	bool		Close(void) override;
	lsize		Read(void* pvBuffer, lsize iSize, lsize iCount) override;
	bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin) override;
	lsize		Write(const void* pvBuffer, lsize iSize, lsize iCount) override;
	filePos		Tell(void) override;
	bool		IsOpen(void) override;
	filePos		Size(void) override;
	bool		Truncate(filePos iSize) override;
	bool		Flush(void) override;
	bool		Delete(void) override;
	const char*		GetFilename(void) override;
};


CMemoryFile* MemoryFile(void);
CMemoryFile* MemoryFile(void* pvInitialMem, lsize iInitialLength);


#endif // __MEMORY_FILE_H__

