/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifdef __65C64__
#include <string.h>
#include <host.h>
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "DiskFile.h"


//The host file device's handles (Libc65 host.h).  0 is no file.
#define INVALID_HOST_HANDLE	0


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetHostHandle(CDiskFile* pcDiskFile, int iHandle)
{
	pcDiskFile->SetFile(&iHandle, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetHostHandle(CDiskFile* pcDiskFile)
{
	int		iHandle;

	memcpy(&iHandle, pcDiskFile->GetFile(), sizeof(int));
	return iHandle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(const char* szFilename)
{
	CAbstractFile::Init();
	SetHostHandle(this, INVALID_HOST_HANDLE);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFilename)
{
	CAbstractFile::Init();
	SetHostHandle(this, INVALID_HOST_HANDLE);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	if (IsOpen())
	{
		Close();
	}
	mszFilename.Kill();
	SetHostHandle(this, INVALID_HOST_HANDLE);
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
	return false;
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
// Create without truncate, as Windows OPEN_ALWAYS.
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Open(EFileMode eMode)
{
	int		iMode;
	int		iHandle;

	if (IsOpen())
	{
		Close();
	}

	if (eMode == EFM_Read)
	{
		iMode = HOST_READ;
	}
	else if (eMode == EFM_Read_Create)
	{
		iMode = HOST_READ | HOST_CREATE;
	}
	else if (eMode == EFM_Write_Create)
	{
		iMode = HOST_WRITE | HOST_CREATE;
	}
	else if (eMode == EFM_ReadWrite)
	{
		iMode = HOST_READ | HOST_WRITE;
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		iMode = HOST_READ | HOST_WRITE | HOST_CREATE;
	}
	else
	{
		return false;
	}
	iHandle = HostOpen(mszFilename.Text(), iMode);
	SetHostHandle(this, iHandle);
	return iHandle != INVALID_HOST_HANDLE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Close(void)
{
	if (!IsOpen())
	{
		return false;
	}
	if (HostClose(GetHostHandle(this)))
	{
		SetHostHandle(this, INVALID_HOST_HANDLE);
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
	if ((iSize == 0) || !IsOpen())
	{
		return 0;
	}
	return HostRead(GetHostHandle(this), pvBuffer, iSize * iCount) / iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Seek(filePos iOffset, EFileSeekOrigin eSeekOrigin)
{
	int		iOrigin;

	if (!IsOpen())
	{
		return false;
	}
	if (eSeekOrigin == EFSO_SET)
	{
		iOrigin = HOST_SEEK_SET;
	}
	else if (eSeekOrigin == EFSO_CURRENT)
	{
		iOrigin = HOST_SEEK_CUR;
	}
	else if (eSeekOrigin == EFSO_END)
	{
		iOrigin = HOST_SEEK_END;
	}
	else
	{
		return false;
	}
	return HostSeek(GetHostHandle(this), iOffset, iOrigin) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDiskFile::Write(const void* pvBuffer, size iSize, size iCount)
{
	if (!IsOpen() || (iSize == 0) || (iCount == 0))
	{
		return 0;
	}
	return HostWrite(GetHostHandle(this), pvBuffer, iSize * iCount) / iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Tell(void)
{
	filePos	iPosition;

	if (IsOpen())
	{
		iPosition = HostTell(GetHostHandle(this));
		if (iPosition != -1)
		{
			return iPosition;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::IsOpen(void)
{
	return GetHostHandle(this) != INVALID_HOST_HANDLE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Size(void)
{
	if (IsOpen())
	{
		return HostSize(GetHostHandle(this));
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The file pointer is left at the new end, as Windows SetEndOfFile leaves it.
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Truncate(filePos iSize)
{
	if (!IsOpen())
	{
		return false;
	}
	return HostTruncate(GetHostHandle(this), iSize) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Flush(void)
{
	if (IsOpen())
	{
		return HostFlush(GetHostHandle(this)) != 0;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
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
const char* CDiskFile::GetFilename(void)
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


#endif // __65C64__

