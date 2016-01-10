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
#include "BaseLib/AbstractFile.h"
#include "ArrayVariable.h"


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
	CArrayVariable				mcWrites;
	filePos						miPosition;
	filePos						miLength;  //'-1' file does not exist on disk.  '0' file is zero bytes long on disk.

	filePos						miFileLength;
	CFileBasic*					mpcPrimaryFile;
	CFileBasic*					mpcRewriteFile;
	
	EFileMode					meOpenMode;
	BOOL						mbOpenedSinceBegin;

	CDurableFileController*		mpcController;

public:
	void		Init(CDurableFileController* pcController, char* szFileName, char* szRewriteName);
	void		InitBasic(void);
	BOOL		Kill(void);

	BOOL		Commit(void);
	BOOL		Recommit(void);

	BOOL		IsDurable(void);
	BOOL		IsBegun(void);
	BOOL		IsOpen(void);

	filePos		Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);

	filePos		Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);

	filePos		Tell(void);
	filePos		Size(void);

	int			GetNumWrites(void);
	void*		GetWriteData(int iWrite);

	BOOL		CheckIdentical(void);
	BOOL		Delete(void);
	BOOL		CopyBackupToPrimary(void);
	BOOL		CopyPrimaryToBackup(void);

	filePos		TestGetPosition(void);
	filePos		TestGetLength(void);
	BOOL		TestGetOpenedSinceBegin(void);

	void		Dump(void);
	CFileBasic*	DumpGetPrimaryFile(void);

protected:
	BOOL		Seek(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite);
	BOOL		OpenPrimaryForRead(void);
	BOOL		OpenPrimaryForWrite(void);

	filePos		ReadDurable(void* pvDest, filePos iSize, filePos iCount);
	filePos		ReadNonDurable(void* pvDest, filePos iSize, filePos iCount);
	filePos		WriteDurable(const void* pvSource, filePos iSize, filePos iCount);
	filePos		WriteNonDurable(const void* pvSource, filePos iSize, filePos iCount);

	filePos		SizeDurable(void);
	filePos		SizeNonDurable(void);
	BOOL		SeekDurable(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite);
	BOOL		SeekNonDurable(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite);

	BOOL		WriteWriteCommands(CFileBasic* pcFile, char *szFileName);

	BOOL		FindTouchingWriteCommands(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap);
	BOOL		DoesWriteOverlap(SDurableFileCommandWrite* psWrite, filePos iPosition, filePos iLength);
	BOOL		AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength);
	BOOL		DoOverlappingWritesContainHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength);

	BOOL		OpenPrimaryFile(BOOL bOpenForWrite);
	filePos		ReadPrimaryFileForDurableRead(void* pvDest, filePos iSize, filePos iCount);
};


#endif // __DURABLE_FILE_H__

