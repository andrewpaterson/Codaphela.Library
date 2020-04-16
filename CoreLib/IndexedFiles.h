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
#ifndef __INDEXED_FILES_H__
#define __INDEXED_FILES_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
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
	void			Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite);
	void			Kill(void);

	BOOL			ReadIndexedFileDescriptors(void);
	BOOL			WriteIndexedFileDescriptors(void);
	BOOL			DataFileName(char* szFile1, char* szFile2, int iDataSize, int iFileNum);

	CIndexedFile* 	GetOrCreateFile(unsigned int uiDataSize);
	CIndexedFile* 	GetFile(unsigned int uiDataSize, int iFileNum);
	CIndexedFile* 	GetFile(int iFileIndex);
	int				GetUniqueFileNumber(int iDataSize);
	void			GetFiles(CArrayIndexedFilePtr* pac);
	unsigned int	GetFileDataSize(int iFileIndex);

	int64			NumDatas(void);
	int64			NumDatas(int iDataSize);
	int				NumFiles(int iDataSize);
	int				NumFiles(void);

	BOOL			Read(CFileDataIndex* pcDataIndex, void* pvData);
	BOOL			Delete(CFileDataIndex* pcDataIndex);
	CFileDataIndex	WriteNew(void* pvData, unsigned int uiDataSize);
	BOOL			WriteExisting(CFileDataIndex* pcDataIndex, void* pvData, unsigned int uiDataSize);

	BOOL			IsDurable(void);
	
	void			Dump(void);
};


#endif // __INDEXED_FILES_H__

