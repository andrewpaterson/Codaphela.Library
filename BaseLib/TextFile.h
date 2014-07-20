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
#ifndef __TEXT_FILE_H__
#define __TEXT_FILE_H__
#include "FileBasic.h"
#include "ArrayIntMinimal.h"
#include "Define.h"
#include "Chars.h"


class CTextFile
{
public:
	CChars		mcText;
	CChars		mszFileName;

	void	Init(void);
	void	Init(char* szText);
	void	Kill(void);
	void	KillExceptBuffer(void);
    BOOL	Read(char* szFileName);
	BOOL	Read(CAbstractFile* pcAbstractFile);
	BOOL	Write(char* szFileName);
	char*	Text(void);
	int		Length(void);
	void	PassifyNewlines(void);
	void	Dump(void);
};


#endif // __TEXT_FILE_H__

