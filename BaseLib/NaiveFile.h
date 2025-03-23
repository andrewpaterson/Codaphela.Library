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
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __NAIVE_FILE_H__
#define __NAIVE_FILE_H__
#include "FileBasic.h"
#include "Define.h"
#include "Chars.h"


class CNaiveFile
{
protected:
	CChars		mszFilename;
	void*		mpvMem;
	filePos		miSize;

public:
	CNaiveFile();

	CNaiveFile*	Init(void);
	CNaiveFile* Init(void* pvMem, size iSize);
	void		Kill(void);
	void		KillExceptBuffer(void);

	bool		Read(const char* szFilename);
	bool		Read(CAbstractFile* pcAbstractFile);
	bool		Write(const char* szFilename);

	void*		Get(void);
	void*		Get(int iOffset);
	void		Set(void* pvMem, size iSize);
	filePos		Size();

	int			Compare(void* pvOther, size iSize);
};


int CompareFileToMemory(const char* szFilename, void* pvMem, size iLength);


#endif // __NAIVE_FILE_H__

