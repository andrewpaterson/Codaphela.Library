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
#include "IndexDescriptor.h"
#include "IndexedFile.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
#include "BaseLib/FreeList.h"
#include "IndexedDescriptors.h"


class CIndexedFiles
{
public:
	CDurableFileController*		mpcDurableFileControl;
	CArrayIndexedFile			mcFiles;  //These are the data on disk.  Generally one file per data size.
	CChars						mszIndexName;
	CChars						mszIndexRewrite;
	CChars						mszExtension;
	CDurableFile				mcFileDescriptorsFile;

	void						Init(CDurableFileController* pcDurableFileControl, char* szExtension);
	void						Kill(void);

	void						InitIndexedFileDescriptors(void);
	void						ReadIndexedFileDescriptors(void);
	void						WriteIndexedFileDescriptors(void);
	BOOL						DataFileName(char* szFile1, char* szFile2, int iDataSize, int iFileNum);

	BOOL						WriteNew(CIndexDescriptor* pcDescriptor, void* pvData);
	BOOL						WriteExisting(CIndexDescriptor* pcDescriptor, void* pvData);
	BOOL						Read(CIndexDescriptor* pcDescriptor, void* pvData);

	CIndexedFile* 				GetFileForNewAllocation(int iDataSize);
	CIndexedFile* 				GetFile(int iDataSize, int iFileNum);
	CIndexedFile* 				GetFile(int iFileIndex);

	OIndex						NumInFile(int iDataSize);
	
	void						Dump(void);
};


#endif // __INDEXED_FILES_H__

