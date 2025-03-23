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
#ifndef __INDEXED_FILE_H__
#define __INDEXED_FILE_H__
#include "FileBasic.h"
#include "ChunkFile.h"
#include "Chars.h"
#include "ArrayTemplate.h"
#include "ArrayBit.h"
#include "ListTemplate.h"
#include "DurableFile.h"
#include "DurableFileController.h"


#define INDEXED_FILE_WRITE_ERROR	((filePos)-1)
#define INDEXED_FILE_NOT_FOUND		((uint32)-1)
#define INDEX_FILE_EMPTY_CHAR		0x55


class CIndexedFile
{
private:
	size			muiFileIndex;  //This is the index of the file in the CIndexedFiles.mcFiles array.

	CDurableFile	mcFile;

	size			muiDataSize;
	filePos			miNumDatas;
	uint32			muiFileNumber;  //There may be more than one file of the same size.

public:
	bool			Init(CDurableFileController* pcDurableFileControl, size uiFileIndex, char* szFilename, char* szRewriteName, size uiDataSize, uint32 uiFileNum);
	void			Kill(void);
	filePos			CalculateNumDatas(void);
	bool			IsFull(void);

	filePos			Write(void* pvData);
	bool			Write(filePos iIndex, void* pvData);
	bool			Write(filePos iIndex, void* pvData, size iCount);

	bool			Read(filePos iIndex, void* pvData);
	bool			Read(filePos iIndex, void* pvData, size iCount);

	bool			Delete(filePos iIndex);
	bool			Delete(filePos iIndex, size iCount);

	size			GetFileIndex(void);
	bool			IsFileIndex(size uiFileIndex);

	CFileBasic*		DumpGetPrimaryFile(void);
	char*			GetFilename(void);
	uint32			GetFileNumber(void);
	char*			GetRewriteName(void);
	size			GetDataSize(void);
	filePos			NumDatas(void);
	filePos			GetUsedDataIndices(CArrayBit* pab);

	filePos			GetFileSize(void);
	void			Dump(void);

protected:
	filePos			Write(void* pvData, size iCount);
};


typedef CListTemplate<CIndexedFile> CListIndexedFile;
typedef CArrayTemplate<CIndexedFile*> CArrayIndexedFilePtr;


#endif // __INDEXED_FILE_H__

