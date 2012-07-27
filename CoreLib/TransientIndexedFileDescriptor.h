/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __TRANSIENT_INDEXED_FILE_DESCRIPTOR_H__
#define __TRANSIENT_INDEXED_FILE_DESCRIPTOR_H__
#include "BaseLib/FileBasic.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "DurableFileController.h"


class CTransientIndexedFileDescriptor
{
public:
	int				miFileIndex;  //I'm not sure what this is.
	CFileBasic		mcFile;
	int				miDataSize;
	int				miNumDatas;
	int				miFileNumber;  //There may be more than one file of the same size.
	CChars			mszFileName;

	void		Init(int iFileIndex, char* szFileName, int iDataSize, int iFileNum);
	void		Kill(void);
	void		GetFileName(char* szName);
	BOOL		IsFull(void);
	filePos		Write(void* pvData);
	BOOL		Write(filePos iIndex, void* pvData);
	BOOL		Read(filePos iIndex, void* pvData);
};


typedef CArrayTemplate<CTransientIndexedFileDescriptor> CArrayTransientIndexedFileDescriptor;


#endif // __TRANSIENT_INDEXED_FILE_DESCRIPTOR_H__

