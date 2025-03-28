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
#ifndef __FILES_H__
#define __FILES_H__
#include "DiskFile.h"
#include "PackFile.h"
#include "FileSystem.h"
#include "MapStringInt.h"
#include "FileIterator.h"
#include "PackFileOffset.h"


typedef CArrayTemplate<CPackFileOffset>	CArrayPackFiles;


class CFiles
{
protected:
	CFileSystem			mcFileSystem;
	CArrayPackFiles		mcPackFilesArray;
	CChars				mszPackFilesExtension;

public:

	bool					Init(char* szDirectory, char* szPackFilesExtension);
	void					Kill(void);

	size					GetNumPackFiles(void);
	CAbstractFile*			GetFile(char* szFullName);

	CFileIteratorReturn*	StartIteration(CFileIterator* pcIter);
	CFileIteratorReturn*	Iterate(CFileIterator* pcIter);
	void					StopIteration(CFileIterator* pcIter);

	void					GetFilenames(CMapStringInt* cFilenames);

	CPackFiles*				GetPackFiles(size iIndex);

protected:
	CDiskFile*				GetSystemFile(char* szFullName);
	CPackFile*				GetPackFile(char* szFullName);
	CPackFile*				GetPackFile(CPackFileOffset* pcPackFiles, char* szFullName);

	bool					AddPackFile(CFileNodeSystemFile* pcFileNodeSystemFile);
	bool					AddPackFiles(void);

	CFileIteratorReturn*	IterateOnFileSystem(CFileIterator* pcIter);
	CFileIteratorReturn*	IterateInPackFiles(CFileIterator* pcIter);

	CFileIteratorReturn*	IterateInPackFiles(CFileIterator* pcIter, CFileNodePackFileNode* pcPackFileNode, CPackFileOffset* pcPackFiles);
	CFileIteratorReturn*	IterateFileSystemNode(CFileIterator* pcIter, CSystemFileNode* pcSystemFileNode);
};


int ComparePackFileOffset(const void* arg1, const void* arg2);


#endif // __FILES_H__

