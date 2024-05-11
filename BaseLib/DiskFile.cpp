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
bool IsHandle(CDiskFile* pcDiskFile, HANDLE hFile)
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
void CDiskFile::Init(const char* szFilename)
{
	CAbstractFile::Init();
	SetHandle(this, INVALID_HANDLE_VALUE);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFilename)
{
	CAbstractFile::Init();

	SetHandle(this, INVALID_HANDLE_VALUE);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	mszFilename.Kill();
	SetHandle(this, INVALID_HANDLE_VALUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::SetFile(void* pvFile, size uiSize)
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
bool CDiskFile::IsFile(void* pvFile, size uiSize)
{
	if (uiSize <= 8)
	{
		return memcmp(maFile, pvFile, uiSize) == 0;
	}
	else
	{
		return false;
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
HANDLE CreateWindowsFile(char* szName, uint32 uiRights, uint32 uiShare, uint32 uiCreation)
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
bool CDiskFile::Open(EFileMode eMode)
{
	HANDLE hFile;

	if (!IsHandle(this, INVALID_HANDLE_VALUE))
	{
		Close();
	}

	if (eMode == EFM_Read)
	{
		hFile = CreateWindowsFile(mszFilename.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
	}
	else if (eMode == EFM_Read_Create)
	{
		hFile = CreateWindowsFile(mszFilename.Text(), GENERIC_READ, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_Write_Create)
	{
		hFile = CreateWindowsFile(mszFilename.Text(), GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS);
	}
	else if (eMode == EFM_ReadWrite)
	{
		hFile = CreateWindowsFile(mszFilename.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING);
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		hFile = CreateWindowsFile(mszFilename.Text(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_ALWAYS);
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
bool CDiskFile::Close(void)
{
	bool	bResult;
	HANDLE	hFile;

	if (IsHandle(this, INVALID_HANDLE_VALUE))
	{
		return false;
	}

	hFile = GetHandle(this);
	bResult = CloseHandle(hFile);
	if (bResult)
	{
		SetHandle(this, INVALID_HANDLE_VALUE);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDiskFile::Read(void* pvBuffer, size iSize, size iCount)
{
	bool	bResult;
	size	iByteLength;
	size	uiTruncatedLength;
	size	uiReadLength;

	if (iSize == 0)
	{
		return 0;
	}

	if (IsOpen())
	{
		iByteLength = iSize * iCount;
		if (iByteLength <= MAX_UINT)
		{
			uiTruncatedLength = iByteLength;
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
bool CDiskFile::Seek(filePos iOffset, EFileSeekOrigin eSeekOrigin)
{
	size	uiResult;

	if (IsOpen())
	{
		if (eSeekOrigin == EFSO_SET)
		{
			uiResult = SetFilePointer(GetHandle(this), (uint32)iOffset, NULL, FILE_BEGIN);
		}
		else if (eSeekOrigin == EFSO_CURRENT)
		{
			uiResult = SetFilePointer(GetHandle(this), (uint32)iOffset, NULL, FILE_CURRENT);
		}
		else if (eSeekOrigin == EFSO_END)
		{
			uiResult = SetFilePointer(GetHandle(this), (uint32)iOffset, NULL, FILE_END);
		}
		return (uiResult != INVALID_SET_FILE_POINTER);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDiskFile::Write(const void* pvBuffer, size iSize, size iCount)
{
	size	uiWritten;
	bool	bResult;
	size	iByteLength;
	size	uiTruncatedLength;

	if (IsOpen() && (iSize != 0) && (iCount != 0))
	{
		iByteLength = iSize * iCount;
		uiTruncatedLength = iByteLength;

		bResult = WriteFile(GetHandle(this), pvBuffer, uiTruncatedLength, (LPDWORD)&uiWritten, NULL);

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
	size	uiResult;

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
bool CDiskFile::IsOpen(void)
{
	return !IsHandle(this, INVALID_HANDLE_VALUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Size(void)
{
	size	uiFileSize;

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
bool CDiskFile::Truncate(filePos iSize)
{
	size	uiResult;
	bool	bResult;

	uiResult = SetFilePointer(GetHandle(this), (uint32)iSize, NULL, FILE_BEGIN);
	if (uiResult == INVALID_SET_FILE_POINTER)
	{
		return false;
	}
	bResult = SetEndOfFile(GetHandle(this));
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Flush(void)
{
	bool	bResult;
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
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Delete(void)
{
	CFileUtil	cFileUtil;

	if (IsOpen())
	{
		return false;
	}

	return cFileUtil.Delete(mszFilename.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDiskFile::GetFilename(void)
{
	if (!mszFilename.Empty())
	{
		return mszFilename.Text();
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
	pcDiskFile->mbBasicFileMustFree = true;
	return pcDiskFile;
}

