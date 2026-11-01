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
#ifndef __INDEXED_FILES_H__
#define __INDEXED_FILES_H__
#include "ChunkFile.h"
#include "Chars.h"
#include "IndexedFile.h"
#include "IndexedDataDescriptor.h"


class CIndexedFiles
{
protected:
	CDurableFileController*		mpcDurableFileControl;

	CListIndexedFile			mcFiles;  //These are the data on disk.  Generally one file per data size.

	CChars						mszDataExtension;
	CChars						mszSubDirectory;

	CDurableFile				mcFileDescriptors;

public:
	void			Init(CDurableFileController* pcDurableFileControl, const char* szSubDirectory, const char* szDataExtension, const char* szDescriptorName, const char* szDescriptorRewrite);
	bool			Kill(void);

	bool			ReadIndexedFileDescriptors(void);
	bool			WriteIndexedFileDescriptors(void);
	bool			DataFilename(char* szFile1, char* szFile2, lsize iDataSize, uint32 iFileNum);

	CIndexedFile* 	GetOrCreateFile(lsize uiDataSize);
	CIndexedFile* 	GetFile(lsize uiDataSize, uint32 iFileNum);
	CIndexedFile* 	GetFile(uint32 uiFileIndex);
	uint32			GetUniqueFileNumber(lsize iDataSize);
	void			GetFiles(CArrayIndexedFilePtr* pac);
	lsize			GetFileDataSize(uint32 uiFileIndex);

	int64			NumDatas(void);
	int64			NumDatas(lsize iDataSize);
	size			NumFiles(lsize iDataSize);
	size			NumFiles(void);

	bool			Read(CFileDataIndex* pcDataIndex, void* pvData);
	bool			Delete(CFileDataIndex* pcDataIndex);
	CFileDataIndex	WriteNew(const void* pvData, lsize iDataSize);
	bool			WriteExisting(CFileDataIndex* pcDataIndex, const void* pvData, lsize iDataSize);

	bool			IsDurable(void);
	
	void			Dump(void);
};


#endif // __INDEXED_FILES_H__

