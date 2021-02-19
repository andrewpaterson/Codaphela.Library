/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include <io.h>
#include "WindowsError.h"
#include "IntegerHelper.h"
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(const char* szFileName)
{
	CAbstractFile::Init();
	mhFile = INVALID_HANDLE_VALUE;
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFileName)
{
	CAbstractFile::Init();
	mhFile = INVALID_HANDLE_VALUE;
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	mszFileName.Kill();
	mhFile = INVALID_HANDLE_VALUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
HANDLE CreateWindowsFile(char* szName, unsigned int uiRights, unsigned uiShare, unsigned int uiCreation)
{
	HANDLE	hFile;

	hFile = CreateFile(szName,
		uiRights,
		uiShare,
		NULL,
		uiCreation,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	return hFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Open(EFileMode eMode)
{
	if (mhFile != INVALID_HANDLE_VALUE)
	{
		Close();
	}

	if (eMode == EFM_Read)
	{
		mhFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
	}
	else if (eMode == EFM_Read_Create)
	{
		mhFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_Write_Create)
	{
		mhFile = CreateWindowsFile(mszFileName.Text(), GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_ReadWrite)
	{
		mhFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		mhFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS);
	}
	else
	{
		mhFile = INVALID_HANDLE_VALUE;
	}

	return mhFile != INVALID_HANDLE_VALUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Close(void)
{
	BOOL	bResult;

	if (mhFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	bResult = CloseHandle(mhFile);
	if (bResult)
	{
		mhFile = INVALID_HANDLE_VALUE;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Read(void* pvBuffer, filePos iSize, filePos iCount)
{
	BOOL			bResult;
	filePos			iByteLength;
	unsigned int	uiTruncatedLength;
	unsigned int	uiReadLength;

	if (IsOpen())
	{
		iByteLength = iSize * iCount;
		if (iByteLength <= MAX_UINT)
		{
			uiTruncatedLength = (unsigned int)iByteLength;
		}
		else
		{
			uiTruncatedLength = 0;
		}

		bResult = ReadFile(mhFile, pvBuffer, uiTruncatedLength, (LPDWORD)&uiReadLength, NULL);
		if (bResult)
		{
			return uiReadLength / iSize;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	unsigned int	uiResult;

	if (IsOpen())
	{
		if (iSeekOrigin == EFSO_SET)
		{
			uiResult = SetFilePointer(mhFile, (LONG)iOffset, NULL, FILE_BEGIN);
		}
		else if (iSeekOrigin == EFSO_CURRENT)
		{
			uiResult = SetFilePointer(mhFile, (LONG)iOffset, NULL, FILE_CURRENT);
		}
		else if (iSeekOrigin == EFSO_END)
		{
			uiResult = SetFilePointer(mhFile, (LONG)iOffset, NULL, FILE_END);
		}
		return (uiResult != INVALID_SET_FILE_POINTER);
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Write(const void* pvBuffer, filePos iSize, filePos iCount)
{
	unsigned int	uiWritten;
	BOOL			bResult;
	filePos			iByteLength;
	unsigned int	uiTruncatedLength;

	if (IsOpen())
	{
		iByteLength = iSize * iCount;
		if (iByteLength <= MAX_UINT)
		{
			uiTruncatedLength = (unsigned int)iByteLength;
		}
		else
		{
			uiTruncatedLength = 0;
		}

		bResult = WriteFile(
			mhFile,
			pvBuffer,
			uiTruncatedLength,
			(LPDWORD)&uiWritten,
			NULL);

		if (bResult)
		{
			return uiWritten / iSize;
		}
		return 0;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Tell(void)
{
	unsigned int	uiResult;
	if (IsOpen())
	{
		uiResult = SetFilePointer(mhFile, 0, NULL, FILE_CURRENT);
		return uiResult;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::IsOpen(void)
{
	return mhFile != INVALID_HANDLE_VALUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Size(void)
{
	unsigned int	uiFileSize;

	if (IsOpen())
	{
		uiFileSize = GetFileSize(mhFile, NULL);
		return uiFileSize;	
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Truncate(filePos iSize)
{
	unsigned int	uiResult;
	BOOL			bResult;

	uiResult = SetFilePointer(mhFile, (LONG)iSize, NULL, FILE_BEGIN);
	if (uiResult == INVALID_SET_FILE_POINTER)
	{
		return FALSE;
	}
	bResult = SetEndOfFile(mhFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Flush(void)
{
	BOOL	bResult;
	char*	sz;

	if (IsOpen())
	{
		bResult = FlushFileBuffers(mhFile);
		if (!bResult)
		{
			sz = WindowsErrorCodeToString(GetLastError());
		}
		return bResult;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Delete(void)
{
	CFileUtil	cFileUtil;

	if (IsOpen())
	{
		return FALSE;
	}

	return cFileUtil.Delete(mszFileName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDiskFile::GetFileName(void)
{
	if (!mszFileName.Empty())
	{
		return mszFileName.Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDiskFile* DiskFile(const char* szName)
{
	CDiskFile* pcDiskFile;

	pcDiskFile = NewMalloc<CDiskFile>();
	pcDiskFile->Init(szName);
	pcDiskFile->mbBasicFileMustFree = TRUE;
	return pcDiskFile;
}

