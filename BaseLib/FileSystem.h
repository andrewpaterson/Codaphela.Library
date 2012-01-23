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
#include "FileNames.h"
#include "FileNodeFileSystem.h"


typedef CFileNode<CFileNodeSystemFile>			CSystemFileNode;
typedef CFileNodeDirectory<CFileNodeSystemFile>	CSystemDirectoryNode;
typedef CFileNames<CFileNodeSystemFile>			CSystemFileNames;
typedef CArrayTemplate<CFileNodeSystemFile*>	CArraySystemFileNodePtrs;
typedef CArrayTemplate<CSystemFileNode*>		CArraySystemFilePtrs;



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
	void					GetFileExtension(CArraySystemFileNodePtrs* paFileNodePtrs, char* szExtension);
	CChars*					GetFullDirectoryName(void);
	void					GetFiles(CArraySystemFilePtrs* pcFiles);
	
protected:
	void					RecurseGetFiles(CSystemFileNode* pcNode, CArraySystemFilePtrs* pcPackFiles);
	void					RecurseGetFileExtension(CArraySystemFileNodePtrs* paFileNodePtrs, char* szExtension, CSystemFileNode* pcDirectoryNode);
	void					ReadDirectory(void);
};


#endif // __FILE_SYSTEM_H__

