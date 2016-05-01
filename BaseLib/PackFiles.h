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
#ifndef __PACK_FILES_H__
#define __PACK_FILES_H__
#include "AbstractFile.h"
#include "FileBasic.h"
#include "PackFilesGeneral.h"
#include "PackFileIterator.h"


#define PACK_FILE_WRITER "PACKED FILE"
#define PACK_FILE_VERSION "V1.0"


class CPackFile;
class CPackFiles
{
friend class CPackFile;
protected:
	CFileBasic					mcFile;
	CFileNamesPackFileNode		mcNames;
	EPackFileMode				meMode;
	filePos						miNodes;
	filePos						miNextNodesPtr;
	
	CPackFileNode*				mpsLastAccessed;
	filePos						miPosition;

	CPackFileReadArray			maReads;

public:
	BOOL					Init(CAbstractFile* pcFile, EPackFileMode eMode);
	void					Kill(void);

	BOOL					IsWriteMode(void);
	BOOL					IsReadMode(void);

	CPackFile*				WriteOpen(char* szFileName);
	CPackFile*				ReadOpen(char* szFileName);

	BOOL					AddFile(CAbstractFile* pcFile, char* szFileName);
	BOOL					AddDirectory(char* szDirectory, char* szPackDirectory = NULL);

	CFileNodePackFileNode*	StartIteration(CPackFileIterator* psIter);
	CFileNodePackFileNode*	Iterate(CPackFileIterator* psIter);
	void					StopIteration(CPackFileIterator* psIter);

	void					GetFiles(CArrayPackFileNodePtrs* pcPackFiles);
	CFileNodePackFileNode*	GetNode(char* szFullName);

	BOOL					Unpack(char* szDirectory);
	void					FixParents(void);

protected:
	BOOL					BeginRead(void);
	BOOL					BeginWrite(void);
	BOOL					EndRead(void);
	BOOL					EndWrite(void);
	BOOL					Close(CPackFile* pcPackFile);
	filePos					Read(CPackFileNode* psPackFile, void* pvBuffer, filePos iSize, filePos iCount);
	BOOL					Seek(CPackFileNode* psPackFile, filePos iOffset, EFileSeekOrigin iSeekOrigin);
	filePos					Write(CPackFileNode* psPackFile, const void* pvBuffer, filePos iSize, filePos iCount);
	filePos					Tell(CPackFileNode* psPackFile);
	BOOL					Eof(CPackFileNode* psPackFile);
	BOOL					Flush(void);
	CFileNodePackFileNode*	AddFile(char* szFullName);
	filePos					PrivateRead(CPackFileNode* psPackFile, void* pvBuffer, filePos iSize, filePos iCount);
	BOOL					PrivateSeek(CPackFileNode* psPackFile, filePos iOffset, int iSeekOrigin);
	BOOL					ChangeReadFiles(CPackFileNode* psPackFile);
	char*					ClassName(void);
	BOOL					WriteHeader(void);
	BOOL					ReadHeader(void);
	BOOL					ReadNodes(void);
	BOOL					ReadNode(void);
	BOOL					WriteNodes(void);
	int						GetNumUnwrittenNames(void);
	int						RecurseGetNumUnwrittenNames(CFileNodePackFileNode*	pcNode);
	BOOL					WriteUnwrittenNames(void);
	BOOL					RecurseWriteUnwrittenNames(CFileNodePackFileNode* pcNode, CChars* pszPath);
	BOOL					RecurseUnpack(CFileNodePackFileNode* pcNode, char* szDestination);
};


#endif // __PACK_FILES_H__

