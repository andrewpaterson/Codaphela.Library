/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2005 Andrew Paterson

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
#ifndef __FILE_BASIC_H__
#define __FILE_BASIC_H__
#include "Define.h"
#include "FileMode.h"
#include "AbstractFile.h"
#include "FileIO.h"


class CFileBasic : public CFileIO
{
protected:
	CAbstractFile*	mpcFile;

public:
	void		Init(CAbstractFile*	pcFile);
	void		Kill(void);

	BOOL		Open(EFileMode eMode);
	BOOL 		Close();

	BOOL		Seek(filePos iOffset, EFileSeekOrigin eOrigin);
	BOOL		Seek(filePos iOffset);

	BOOL		Flush(void);

	BOOL		Delete(void);

	BOOL		IsOpen(void);
	filePos		GetFileLength(void);
	filePos		GetFilePos(void);
	filePos		GetFileSize(void);  //Same as GetFileLength.

	BOOL		Truncate(filePos iSize);

	char*		GetFileName(void);

	//Generally you shouldn't use these.  Use WriteData instead.
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);
};


#endif // !__FILE_BASIC_H__

