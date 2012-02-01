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
#ifndef __CSV_FILE_H__
#define __CSV_FILE_H__
#include "FileBasic.h"


#define CSV_FILE_LINE_BUFFER_LENGTH 256


class CCSVFile
{
protected:
	CFileBasic	mcFile;
	filePos		miFileSize;
	char		mcSeparator;
	
public:
	void 	Init(char cSeparator);
	void 	Kill(void);

	void 	Open(CAbstractFile* pcFile);
	void 	Close(void);

	BOOL	ReadLine(CChars* szString);
	BOOL	ReadLine(char* szString, int iMaxLength);

protected:
	int		ReadLine(char* snz);
};


#endif //__CSV_FILE_H__

