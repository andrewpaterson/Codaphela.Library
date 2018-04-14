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
#ifndef __DURABLE_FILE_H__
#define __DURABLE_FILE_H__
#include "BaseLib/ArrayIntAndPointer.h"
#include "BaseLib/FileMode.h"
#include "BaseLib/FileIO.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/AbstractFile.h"
#include "BaseLib/ListVariable.h"
#include "LogFile.h"


#define COMMAND_CHUNK_SIZE	4096


struct SDurableFileCommandWrite
{
	filePos	iPosition;
	filePos	iSize;
};


class CDurableFileController;
class CFileBasic;
class CDurableFile
{
private:
	CChars						mszFileName;
	CChars						mszRewriteName;

	CFileBasic					mcPrimaryFile;
	CFileBasic					mcRewriteFile;
	
	CDiskFile					mcPrimaryDiskFile;
	CDiskFile					mcRewriteDiskFile;

	CDurableFileController*		mpcController;

	CLogFile					mcLogFile;
	EFileMode					meOpenMode;
	BOOL						mbLogFileBegun;
	BOOL						mbAddedToController;

public:
	void		Init(CDurableFileController* pcController, char* szFileName, char* szRewriteName);
	BOOL		Kill(void);
	void		InitBasic(void);

	BOOL		Commit(void);
	BOOL		Recommit(void);

	BOOL		IsDurable(void);
	BOOL		IsBegun(void);

	filePos		Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);

	filePos		Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);

	BOOL		Create(void);

	filePos		Tell(void);
	filePos		Size(void);

	int			GetNumWrites(void);
	void*		GetWriteData(int iWrite);
	void		AddFile(void);

	BOOL		CheckIdentical(BOOL bThorough, BOOL bLogError);
	BOOL		Delete(void);
	BOOL		CopyBackupToPrimary(void);
	BOOL		CopyPrimaryToBackup(void);

	BOOL		TestGetOpenedSinceBegin(void);
	BOOL		IsOpen(void);
	char*		GetFileName(void);
	char*		GetRewriteName(void);

	void		Dump(void);
	CFileBasic*	DumpGetPrimaryFile(void);

protected:
	BOOL		Seek(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite);
	BOOL		OpenPrimaryForRead(void);
	BOOL		OpenPrimaryForWrite(void);

	BOOL		OpenPrimaryFile(BOOL bOpenForWrite);
};


#endif // __DURABLE_FILE_H__

