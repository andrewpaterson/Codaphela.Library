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
#ifndef __DEBUG_OUTPUT_FILE_H__
#define __DEBUG_OUTPUT_FILE_H__
#include "ArrayChar.h"
#include "AbstractFile.h"


class CDebugOutputFile : public CAbstractFile
{
protected:
	filePos		miPos;
	bool		mbOpen;

public:
	void		Init(void);
	void		Kill(void) override;

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


CDebugOutputFile* DebugOutputFile(void);


#endif // __DEBUG_OUTPUT_FILE_H__

