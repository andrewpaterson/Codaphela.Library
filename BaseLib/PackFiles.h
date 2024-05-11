/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	CFilenamesPackFileNode		mcNames;
	EPackFileMode				meMode;
	filePos						miNodes;
	filePos						miNextNodesPtr;
	
	CPackFileNode*				mpsLastAccessed;
	filePos						miPosition;

	CPackFileReadArray			maReads;

public:
	bool					Init(CAbstractFile* pcFile, EPackFileMode eMode);
	void					Kill(void);

	bool					IsWriteMode(void);
	bool					IsReadMode(void);

	CPackFile*				WriteOpen(char* szFilename);
	CPackFile*				ReadOpen(char* szFilename);

	bool					AddFile(CAbstractFile* pcFile, char* szFilename);
	bool					AddDirectory(char* szDirectory, char* szPackDirectory = NULL);

	CFileNodePackFileNode*	StartIteration(CPackFileIterator* psIter);
	CFileNodePackFileNode*	Iterate(CPackFileIterator* psIter);
	void					StopIteration(CPackFileIterator* psIter);

	void					GetFiles(CArrayPackFileNodePtrs* pcPackFiles);
	CFileNodePackFileNode*	GetNode(char* szFullName);

	bool					Unpack(char* szDirectory);
	void					FixParents(void);
	char*					GetFilename(void);

protected:
	bool					BeginRead(void);
	bool					BeginWrite(void);
	bool					EndRead(void);
	bool					EndWrite(void);
	bool					Close(CPackFile* pcPackFile);
	size					Read(CPackFileNode* psPackFile, void* pvBuffer, size iSize, size iCount);
	bool					Seek(CPackFileNode* psPackFile, filePos iOffset, EFileSeekOrigin iSeekOrigin);
	size					Write(CPackFileNode* psPackFile, const void* pvBuffer, size iSize, size iCount);
	filePos					Tell(CPackFileNode* psPackFile);
	bool					Flush(void);
	CFileNodePackFileNode*	AddFile(char* szFullName);
	size					PrivateRead(CPackFileNode* psPackFile, void* pvBuffer, size iSize, size iCount);
	bool					PrivateSeek(CPackFileNode* psPackFile, filePos iOffset, EFileSeekOrigin iSeekOrigin);
	bool					ChangeReadFiles(CPackFileNode* psPackFile);
	const char*				ClassName(void);
	bool					WriteHeader(void);
	bool					ReadHeader(void);
	bool					ReadNodes(void);
	bool					ReadNode(void);
	bool					WriteNodes(void);
	uint32					GetNumUnwrittenNames(void);
	uint32					RecurseGetNumUnwrittenNames(CFileNodePackFileNode*	pcNode);
	bool					WriteUnwrittenNames(void);
	bool					RecurseWriteUnwrittenNames(CFileNodePackFileNode* pcNode, CChars* pszPath);
	bool					RecurseUnpack(CFileNodePackFileNode* pcNode, char* szDestination);
};


#endif // __PACK_FILES_H__

