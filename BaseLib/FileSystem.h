/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__
#include "AbstractFile.h"
#include "FileSystemGeneral.h"
#include "FileSystemIterator.h"


class CFileSystem
{
protected:
	CSystemFileNames	mcNames;
	CChars				mszDirectory;

public:
	void					Init(char* szDirectory);
	void					Kill(void);

	char*					GetFileName(char* szFileName);
	CFileNodeSystemFile*	GetFileNode(char* szName, CChars* pszRemainingName);
	CChars*					GetFullDirectoryName(void);
	void					GetFiles(CArraySystemFilePtrs* pcSystemFiles);
	void					GetFiles(CArraySystemFilePtrs* pcSystemFiles, char* szExtension);

	CSystemFileNode*		StartIteration(CFileSystemIterator* psIter);
	CSystemFileNode*		Iterate(CFileSystemIterator* psIter);
	void					StopIteration(CFileSystemIterator* psIter);
	
protected:
	void					ReadDirectory(void);
};


#endif // !__FILE_SYSTEM_H__

