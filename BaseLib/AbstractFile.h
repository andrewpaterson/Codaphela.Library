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
afilePos with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ABSTRACT_FILE_H__
#define __ABSTRACT_FILE_H__
#include "Define.h"
#include "Chars.h"
#include "FileMode.h"


typedef long long int		filePos;


class CAbstractFile
{
public:
	BOOL	mbBasicFileMustFree;  //Set by helpers

			void		Init(void);
	virtual void		Kill(void) =0;
	virtual BOOL		Open(EFileMode eFileMode) =0;
	virtual BOOL		Close(void) =0;
	virtual filePos		Read(void* pvBuffer, filePos iSize, filePos iCount) =0;
	virtual BOOL		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin) =0;
	virtual filePos		Write(const void* pvBuffer, filePos iSize, filePos iCount) =0;
	virtual filePos		Tell(void) =0;
	virtual BOOL		Eof(void) =0;
	virtual BOOL		IsOpen(void) =0;
	virtual filePos		Size(void) =0;
	virtual BOOL		Flush(void) =0;
	virtual BOOL		Delete(void) =0;
};


#endif // __ABSTRACT_FILE_H__

