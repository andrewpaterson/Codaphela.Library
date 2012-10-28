/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2005 Andrew Paterson

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
#include <stdio.h>
#include "FileBasic.h"
#include "IntegerHelper.h"
#include "MemoryFile.h"
#include "MD5HashFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileBasic::Init(CAbstractFile* pcFile)
{
	mpcFile = pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileBasic::Kill(void)
{
	if (mpcFile->mbBasicFileMustFree)
	{
		mpcFile->Kill();
		free(mpcFile);
	}
	mpcFile = NULL;
}


//====================================================================================
//=
//= Function Name: CFileBasic::Open
//=
//= Parameters   : pvFile -		the name of the file to open or
//=								a pointer to the block of memory to use or
//=
//=                iMode      - the mode to open the file as.
//=                             flags are : EFM_Read, EFM_BINARY
//=
//= Returns      : true if successful, false if not.
//=
//= Description  : Opens the file specified with a specific mode.
//=
//====================================================================================
BOOL CFileBasic::Open(EFileMode eMode)
{
	return mpcFile->Open(eMode);
}


//====================================================================================
//=
//= Function Name: CFileBasic::Close
//=
//= Parameters   : -
//=
//= Returns      : true if successful, false if not.
//=
//= Description  : Closes the file (if it is already open)
//=
//====================================================================================
BOOL CFileBasic::Close()
{
	return mpcFile->Close();
}


//====================================================================================
//=
//= Function Name: CFileBasic::IsEndOfFile
//=
//= Parameters   : -
//=
//= Returns      : true if file is at the end of the file, false if not
//=
//= Description  : returns whether at the end of the file or not
//=
//====================================================================================
BOOL CFileBasic::IsEndOfFile(void)
{
	return mpcFile->Eof();
}


//====================================================================================
//=
//= Function Name: CFileBasic::Write
//=
//= Parameters   : pBuffer - pointer to a buffer of memory to write to the file
//=                iElementSize - the size of each element to write (in the buffer)
//=                iNumElements - the number of elements to write (in the buffer)
//=
//= Returns      : the number of elements actually written.
//=
//= Description  : writes a buffer from memory to a file
//=
//====================================================================================
filePos CFileBasic::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	return mpcFile->Write(pvSource, iSize, iCount);
}


//====================================================================================
//=
//= Function Name: CFileBasic::Read
//=
//= Parameters   : pBuffer - pointer to a buffer of memory to read into from the file
//=                iElementSize - the size of each element to read (in the buffer)
//=                iNumElements - the number of elements to read (in the buffer)
//=
//= Returns      : the number of elements actually read.
//=
//= Description  : Reads from file into a memory buffer
//=
//====================================================================================
filePos CFileBasic::Read(void* pvDest, filePos iSize, filePos iCount)
{
	return mpcFile->Read(pvDest, iSize, iCount);
}


//====================================================================================
//=
//= Function Name: CFileBasic::Seek
//=
//= Parameters   : iOffset - the offset to seek to within the file
//=                iOrigin - specifies the origin of the seek.
//=                          values are EFSO_SET, EFSO_END, EFSO_CURRENT
//=
//= Returns      : -
//=
//= Description  : Seeks to an offset relative to an origin within the file.
//=
//====================================================================================
BOOL CFileBasic::Seek(filePos iOffset, EFileSeekOrigin eOrigin)
{
	return mpcFile->Seek(iOffset, eOrigin);
}


//====================================================================================
//=
//= Function Name: CFileBasic::Seek
//=
//= Parameters   : iOffset - the offset to seek to within the file
//=                iOrigin - specifies the origin of the seek.
//=                          values are EFSO_SET, EFSO_END, EFSO_CURRENT
//=
//= Returns      : -
//=
//= Description  : Seeks to an offset relative to an origin within the file.
//=
//====================================================================================
BOOL CFileBasic::Seek(filePos iOffset)
{
	return mpcFile->Seek(iOffset, EFSO_SET);
}


//====================================================================================
//=
//= Function Name: CFileBasic::Flush
//=
//= Parameters   : -
//=
//= Returns      : -
//=
//= Description  : Flush the file cache to disk.
//=
//====================================================================================
void CFileBasic::Flush(void)
{
	mpcFile->Flush();
}


//====================================================================================
//=
//= Function Name: CFileBasic::IsOpen()
//=
//= Returns      : true if a file is currently open.
//=
//= Description  : Seeks to an offset relative to an origin within the file.
//=
//====================================================================================
BOOL CFileBasic::IsOpen(void)
{
	return mpcFile->IsOpen();
}


//====================================================================================
//=
//= Function Name: CFileBasic::FileLength
//=
//= Returns      : the length of the currently open file
//=
//= Description  : gets the length (in bytes) of the file
//=
//====================================================================================
filePos CFileBasic::GetFileLength(void)
{
	return mpcFile->Size();
}


//====================================================================================
//=
//= Function Name: CFileBasic::FileLength
//=
//= Returns      : the length of the currently open file
//=
//= Description  : gets the length (in bytes) of the file
//=
//====================================================================================
filePos CFileBasic::GetFileSize(void)
{
	return mpcFile->Size();
}


//====================================================================================
//=
//= Function Name: CFileBasic::FilePos
//=
//= Returns      : integer position in the file
//=
//= Description  : the position in the current file we are currently at
//=
//====================================================================================
filePos CFileBasic::GetFilePos()
{
	return mpcFile->Tell();
}


//====================================================================================
//=
//=
//====================================================================================
BOOL CFileBasic::Delete(void)
{
	return mpcFile->Delete();
}