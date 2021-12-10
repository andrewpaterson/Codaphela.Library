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
#include <stdio.h>
#include <io.h>
#include "WindowsHeaders.h"
#include "WindowsError.h"
#include "IntegerHelper.h"
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetHandle(CDiskFile* pcDiskFile, HANDLE hFile)
{
	pcDiskFile->SetFile(&hFile, sizeof(HANDLE));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL IsHandle(CDiskFile* pcDiskFile, HANDLE hFile)
{
	return pcDiskFile->IsFile(&hFile, sizeof(HANDLE));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
HANDLE GetHandle(CDiskFile* pcDiskFile)
{
	HANDLE	hFile;
	void*	pvFile;

	pvFile = pcDiskFile->GetFile();
	memcpy(&hFile, pvFile, sizeof(HANDLE));

	return hFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(const char* szFileName)
{
	CAbstractFile::Init();
	SetHandle(this, INVALID_HANDLE_VALUE);
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFileName)
{
	CAbstractFile::Init();

	SetHandle(this, INVALID_HANDLE_VALUE);
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	mszFileName.Kill();
	SetHandle(this, INVALID_HANDLE_VALUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::SetFile(void* pvFile, size_t uiSize)
{
	if (uiSize <= 8)
	{
		memcpy(maFile, pvFile, uiSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::IsFile(void* pvFile, size_t uiSize)
{
	if (uiSize <= 8)
	{
		return memcmp(maFile, pvFile, uiSize) == 0;
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
void* CDiskFile::GetFile(void)
{
	return maFile;
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
	HANDLE hFile;

	if (!IsHandle(this, INVALID_HANDLE_VALUE))
	{
		Close();
	}

	if (eMode == EFM_Read)
	{
		hFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
	}
	else if (eMode == EFM_Read_Create)
	{
		hFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_Write_Create)
	{
		hFile = CreateWindowsFile(mszFileName.Text(), GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_ReadWrite)
	{
		hFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		hFile = CreateWindowsFile(mszFileName.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS);
	}
	else
	{
		hFile = INVALID_HANDLE_VALUE;
	}

	SetHandle(this, hFile);

	return hFile != INVALID_HANDLE_VALUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Close(void)
{
	BOOL	bResult;
	HANDLE	hFile;

	if (IsHandle(this, INVALID_HANDLE_VALUE))
	{
		return FALSE;
	}

	hFile = GetHandle(this);
	bResult = CloseHandle(hFile);
	if (bResult)
	{
		SetHandle(this, INVALID_HANDLE_VALUE);
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

	if (iSize == 0)
	{
		return 0;
	}

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

		bResult = ReadFile(GetHandle(this), pvBuffer, uiTruncatedLength, (LPDWORD)&uiReadLength, NULL);
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
			uiResult = SetFilePointer(GetHandle(this), (LONG)iOffset, NULL, FILE_BEGIN);
		}
		else if (iSeekOrigin == EFSO_CURRENT)
		{
			uiResult = SetFilePointer(GetHandle(this), (LONG)iOffset, NULL, FILE_CURRENT);
		}
		else if (iSeekOrigin == EFSO_END)
		{
			uiResult = SetFilePointer(GetHandle(this), (LONG)iOffset, NULL, FILE_END);
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

	if (IsOpen() && (iSize > 0) && (iCount > 0))
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
			GetHandle(this),
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
		uiResult = SetFilePointer(GetHandle(this), 0, NULL, FILE_CURRENT);
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
	return !IsHandle(this, INVALID_HANDLE_VALUE);
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
		uiFileSize = GetFileSize(GetHandle(this), NULL);
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

	uiResult = SetFilePointer(GetHandle(this), (LONG)iSize, NULL, FILE_BEGIN);
	if (uiResult == INVALID_SET_FILE_POINTER)
	{
		return FALSE;
	}
	bResult = SetEndOfFile(GetHandle(this));
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
		bResult = FlushFileBuffers(GetHandle(this));
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

