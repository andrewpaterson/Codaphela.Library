/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
afilePos with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ABSTRACT_FILE_H__
#define __ABSTRACT_FILE_H__
#include "Define.h"
#include "FileMode.h"
#include "FilePosition.h"


class CAbstractFile
{
public:
	bool	mbBasicFileMustFree;  //Set by helpers

			void		Init(void);
	virtual void		Kill(void) =0;
	virtual bool		Open(EFileMode eFileMode) =0;
	virtual bool		Close(void) =0;
	virtual size		Read(void* pvBuffer, size iSize, size iCount) =0;
	virtual bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin) =0;
	virtual size		Write(const void* pvBuffer, size iSize, size iCount) =0;
	virtual filePos		Tell(void) =0;
	virtual bool		IsOpen(void) =0;
	virtual filePos		Size(void) =0;
	virtual bool		Truncate(filePos iSize) =0;
	virtual bool		Flush(void) =0;
	virtual bool		Delete(void) =0;
	virtual char*		GetFilename(void) =0;
};


#endif // __ABSTRACT_FILE_H__

