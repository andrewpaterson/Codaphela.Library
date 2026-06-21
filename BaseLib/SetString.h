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
#ifndef __SET_STRING_H__
#define __SET_STRING_H__
#include "Mallocator.h"
#include "SystemAllocator.h"
#include "SetBlock.h"
#include "Chars.h"
#include "FileReader.h"


class CSetString : public CSetBlock
{
public:
	void			Init(bool bCaseSensitive = true, bool bOverwrite = true);
	void			Init(CMallocator* pcMalloc, bool bCaseSensitive = true, bool bOverwrite = true);
	void			Kill(void);

	char*			Get(const char* szKey);

	char*			Put(char* szKey);
	char*			Put(const char* szKey);

	bool			Remove(char* szKey);
	bool			Remove(const char* szKey);

	bool			Has(char* szKey);
	bool			Has(const char* szKey);

	bool			WriteCaseSensitivity(CFileWriter* pcFileWriter);
	bool			Write(CFileWriter* pcFileWriter);
	DataCompare		ReadCaseSensitivity(CFileReader* pcFileReader);
	bool			Read(CFileReader* pcFileReader);
	bool			Read(CFileReader* pcFileReader, CDataIO* pcDataIO, CDataFree* pcDataFree);

	bool			IsCaseSensitive(void);
	void			SetCaseSensitive(bool bCaseSensitive);

	void			GetKeysAsString(CChars* pszDest, char* szSeparator);

protected:
	DataCompare		CalculateCompareFunc(bool bCaseSensitive);
};


#endif // __SET_STRING_H__

