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
#include "IndexedDataDescriptor.h"
#include "IndexedFile.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"


class CIndexedFiles
{
protected:
	CDurableFileController*	mpcDurableFileControl;

	//Needs a more performant way of looking up files by index.
	CListIndexedFile		mcFiles;  //These are the data on disk.  Generally one file per data size.

	CChars					mszDataExtension;

	CChars					mszDescricptorName;
	CChars					mszDescricptorRewrite;
	CDurableFile			mcFileDescriptors;

public:
	void			Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite);
	void			Kill(void);

	BOOL			RemoveFiles(void);

	void			InitIndexedFileDescriptors(char* szDescricptorName, char* szDescricptorRewrite);
	BOOL			ReadIndexedFileDescriptors(void);
	BOOL			WriteIndexedFileDescriptors(void);
	BOOL			DataFileName(char* szFile1, char* szFile2, int iDataSize, int iFileNum);

	CIndexedFile* 	GetOrCreateFile(int iDataSize);
	CIndexedFile* 	GetFile(int iDataSize, int iFileNum);
	CIndexedFile* 	GetFile(int iFileIndex);
	int				GetUniqueFileNumber(int iDataSize);

	int64			NumInFile(int iDataSize);
	int				NumFiles(void);

	BOOL			Write(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			Read(CIndexedDataDescriptor* pcDescriptor, void* pvData);

	BOOL			IsDurable(void);
	
	void			Dump(void);

protected:
	BOOL			WriteNew(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			WriteExisting(CIndexedDataDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_FILES_H__

