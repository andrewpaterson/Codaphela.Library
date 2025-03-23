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
#ifndef __MAP_STRING_BLOCK_H__
#define __MAP_STRING_BLOCK_H__
#include "Mallocator.h"
#include "SystemAllocator.h"
#include "MapBlock.h"
#include "Chars.h"
#include "FileReader.h"


class CMapStringBlock : public CMapBlock
{
public:
	void		Init(bool bCaseSensitive = true, bool bOverwrite = true);
	void		Init(CMallocator* pcMalloc, bool bCaseSensitive = true, bool bOverwrite = true);
	void		Kill(void);

	void*		Get(const char* szKey);
	void*		Get(const char* szKey, size* piDataSize);

	void*		Put(char* szKey, size iDataSize);
	bool		Put(char* szKey, void* psData, size iDataSize);
	void*		Put(const char* szKey, size iDataSize);
	bool		Put(const char* szKey, void* psData, size iDataSize);

	bool		Remove(char* szKey);
	bool		Remove(const char* szKey);

	bool		HasKey(char* szKey);
	bool		HasKey(const char* szKey);

	bool		WriteCaseSensitivity(CFileWriter* pcFileWriter);
	bool		Write(CFileWriter* pcFileWriter);
	DataCompare	ReadCaseSensitivity(CFileReader* pcFileReader);
	bool		Read(CFileReader* pcFileReader);

	char*		GetKeyForData(void* psData);

	bool		IsCaseSensitive(void);
	void		SetCaseSensitive(bool bCaseSensitive);

	void		GetKeysAsString(CChars* pszDest, char* szSeparator);

protected:
	DataCompare	CalculateCompareFunc(bool bCaseSensitive);
};


#endif // __MAP_STRING_BLOCK_H__

