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
#ifndef __MD5_HASH_FILE_H__
#define __MD5_HASH_FILE_H__
#include "MD5.h"
#include "AbstractFile.h"


enum ELastMD5Operation
{
	LMD5OP_None,
	LMD5OP_Read,
	LMD5OP_Write
};


class CMD5HashFile : public CAbstractFile
{
public:
	CAbstractFile*		mpcFile;

	SMD5Context			msMD5Context;
	bool				mbResetMD5OnSeek;
	bool				mbResetMD5OnOperationChange;
	bool				mbHashing;
	ELastMD5Operation	meLastOp;

	void		Init(CAbstractFile*	pcFile);  //Don't pass through a file type of hash file again unless you enjoy being stupid.
	void		Kill(void);

	bool		Open(EFileMode eFileMode);
	bool		Close(void);
	size		Read(void* pvBuffer, size iSize, size iCount);
	bool		Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin);
	size		Write(const void* pvBuffer, size iSize, size iCount);
	filePos		Tell(void);
	bool		IsOpen(void);
	filePos		Size(void);
	bool		Truncate(filePos iSize);
	bool		Flush(void);
	bool		Delete(void);
	char*		GetFilename(void);

	void		StopHashing(void);
	void		StartHashing(void);
	void		ResetHash(void);
	void		CopyContextToDest(SMD5Context* psDestContext);
	void		CopyContextFromSource(SMD5Context* psSourceContext);
	void		CopyDigestToDest(void* pvDest);
	void		Dump(void);
};


//Helper function to make creating basic files easier.
CMD5HashFile*	MD5HashFile(CAbstractFile* pcFile);


#endif // __MD5_HASH_FILE_H__

