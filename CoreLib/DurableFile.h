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
#include "BaseLib/ArrayPointer.h"
#include "BaseLib/FileMode.h"
#include "BaseLib/FileIO.h"
#include "BaseLib/AbstractFile.h"
#include "ArrayVariable.h"


#define COMMAND_CHUNK_SIZE	4096


//A non-durable file can never have mbBegun set TRUE.
//A durable file cannot be written too when mbBegun is NOT set.
//A durable file can be seeked or read regarless of mbBegun.


struct SDurableFileCommandWrite
{
	filePos	iPosition;
	filePos	iSize;
};


class CFileBasic;
class CDurableFile
{
friend class CDurableSet;

public:
	CChars				mszFileName;
	CChars				mszRewriteName;
	CArrayVariable		mcWrites;
	filePos				miPosition;
	filePos				miLength;

	filePos				miFileLength;
	CFileBasic*			mpcPrimaryFile;
	CFileBasic*			mpcRewriteFile;
	BOOL				mbBegun;
	BOOL				mbTouched;
	BOOL				mbDurable;

	void		Init(BOOL bDurable, char* szFileName, char* szRewriteName);
	BOOL		Kill(void);

	BOOL		Begin(void);
	BOOL		End(void);
	BOOL		Rewrite(void);
	BOOL		PrivateWrite(CFileBasic* pcFile);

	BOOL		Open(void);
	BOOL		Close(void);
	BOOL		Delete(void);
	filePos		Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount);
	filePos		Write(const void* pvSource, filePos iSize, filePos iCount);
	filePos		Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount);
	filePos		Read(void* pvDest, filePos iSize, filePos iCount);
	void		Seek(EFileSeekOrigin eOrigin, filePos iDistance);
	filePos		Tell(void);
	filePos		Size(void);
	BOOL		IsEof(void);

	filePos		ReadFromFile(void* pvDest, filePos iSize, filePos iCount);
	filePos		SizeFromFile(void);
	BOOL		TestIdentical(void);

	void		Dump(void);

protected:
	BOOL		CopyBackupToPrimary(void);
	BOOL		CopyPrimaryToBackup(void);
	BOOL		FindTouchingWriteCommands(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap);
	BOOL		Overlaps(filePos iPosition, filePos iLength, SDurableFileCommandWrite* psWrite);
	BOOL		AmalgamateOverlappingWrites(CArrayPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength);
	BOOL		FindHoles(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength);
	void		UpdateLength(void);
	void		OpenFilesForBegin(void);
	BOOL		OpenFilesForEnd(CFileBasic* pcFile);
	filePos		PrivateRead(void* pvDest, filePos iSize, filePos iCount);
};


#endif // __DURABLE_FILE_H__

