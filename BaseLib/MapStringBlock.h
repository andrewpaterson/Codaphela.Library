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
	void		Init(int iChunkSize, BOOL bCaseSensitive = TRUE, BOOL bOverwrite = TRUE);
	void		Init(CMallocator* pcMallocator, int iChunkSize, BOOL bCaseSensitive = TRUE, BOOL bOverwrite = TRUE);
	void		Kill(void);

	void*		Get(char* szKey);

	void*		Put(char* szKey, int iDataSize);
	BOOL		Put(char* szKey, void* psData, int iDataSize);

	BOOL		Remove(char* szKey);

	BOOL		WriteCaseSensitivity(CFileWriter* pcFileWriter);
	BOOL		Write(CFileWriter* pcFileWriter);
	CompareFunc	ReadCaseSensitivity(CFileReader* pcFileReader);
	BOOL		Read(CFileReader* pcFileReader);

	BOOL		IsCaseSensitive(void);
	void		SetCaseSensitive(BOOL bCaseSensitive);

protected:
	CompareFunc	CalculateCompareFunc(BOOL bCaseSensitive);
};


#endif // __MAP_STRING_BLOCK_H__

