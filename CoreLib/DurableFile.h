/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
class CDurableFile : public CFileIO
{
private:
	CChars						mszFilename;
	CChars						mszRewriteName;

	CFileBasic					mcPrimaryFile;
	CFileBasic					mcRewriteFile;
	
	CDiskFile					mcPrimaryDiskFile;
	CDiskFile					mcRewriteDiskFile;

	CDurableFileController*		mpcController;

	CLogFile					mcLogFile;
	EFileMode					meOpenMode;
	bool						mbLogFileBegun;
	bool						mbAddedToController;

public:
	bool		Init(CDurableFileController* pcController, char* szFilename, char* szRewriteName);
	bool		Kill(void);
	void		InitBasic(void);
	void		InitError(void);

	bool		Commit(void);
	bool		Recommit(void);

	bool		IsDurable(void);
	bool		IsBegun(void);

	filePos		Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);

	filePos		Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);

	bool		Create(void);
	bool		Exists(void);

	filePos		Tell(void);
	filePos		Size(void);
	bool		Truncate(filePos iSize);

	int			GetNumWrites(void);
	void*		GetWriteData(int iWrite);
	void		AddFile(void);

	bool		CheckIdentical(bool bThorough, bool bLogError);
	bool		Delete(void);
	bool		CopyBackupToPrimary(void);
	bool		CopyPrimaryToBackup(void);

	bool		TestGetOpenedSinceBegin(void);
	bool		IsOpen(void);
	char*		GetFilename(void);
	char*		GetRewriteName(void);

	void		Dump(void);
	CFileBasic*	DumpGetPrimaryFile(void);

protected:
	bool		Seek(EFileSeekOrigin eOrigin, filePos iDistance, bool bSeekForWrite);
	bool		OpenPrimaryForRead(void);
	bool		OpenPrimaryForWrite(void);

	bool		OpenPrimaryFile(bool bOpenForWrite);
};


#endif // !__DURABLE_FILE_H__

