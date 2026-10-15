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
#ifdef LINUX_GNU_32
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "DiskFile.h"


#define INVALID_FILE_DESCRIPTOR	-1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetDescriptor(CDiskFile* pcDiskFile, int iFile)
{
	pcDiskFile->SetFile(&iFile, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsDescriptor(CDiskFile* pcDiskFile, int iFile)
{
	return pcDiskFile->IsFile(&iFile, sizeof(int));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetDescriptor(CDiskFile* pcDiskFile)
{
	int		iFile;
	void*	pvFile;

	pvFile = pcDiskFile->GetFile();
	memcpy(&iFile, pvFile, sizeof(int));

	return iFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(const char* szFilename)
{
	CAbstractFile::Init();
	SetDescriptor(this, INVALID_FILE_DESCRIPTOR);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFilename)
{
	CAbstractFile::Init();

	SetDescriptor(this, INVALID_FILE_DESCRIPTOR);
	mszFilename.Init(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	mszFilename.Kill();
	SetDescriptor(this, INVALID_FILE_DESCRIPTOR);
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
bool CDiskFile::Open(EFileMode eMode)
{
	int		iFile;

	if (!IsDescriptor(this, INVALID_FILE_DESCRIPTOR))
	{
		Close();
	}

	//O_CREAT without O_TRUNC matches Windows OPEN_ALWAYS.
	if (eMode == EFM_Read)
	{
		iFile = open(mszFilename.Text(), O_RDONLY);
	}
	else if (eMode == EFM_Read_Create)
	{
		iFile = open(mszFilename.Text(), O_RDONLY | O_CREAT, 0666);
	}
	else if (eMode == EFM_Write_Create)
	{
		iFile = open(mszFilename.Text(), O_WRONLY | O_CREAT, 0666);
	}
	else if (eMode == EFM_ReadWrite)
	{
		iFile = open(mszFilename.Text(), O_RDWR);
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		iFile = open(mszFilename.Text(), O_RDWR | O_CREAT, 0666);
	}
	else
	{
		iFile = INVALID_FILE_DESCRIPTOR;
	}

	SetDescriptor(this, iFile);

	return iFile != INVALID_FILE_DESCRIPTOR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Close(void)
{
	int		iResult;

	if (IsDescriptor(this, INVALID_FILE_DESCRIPTOR))
	{
		return false;
	}

	iResult = close(GetDescriptor(this));
	if (iResult == 0)
	{
		SetDescriptor(this, INVALID_FILE_DESCRIPTOR);
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
	ssize_t	iReadLength;

	if (iSize == 0)
	{
		return 0;
	}

	if (IsOpen())
	{
		iReadLength = read(GetDescriptor(this), pvBuffer, iSize * iCount);
		if (iReadLength >= 0)
		{
			return (size)iReadLength / iSize;
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
	off_t	iResult;

	if (IsOpen())
	{
		if (eSeekOrigin == EFSO_SET)
		{
			iResult = lseek(GetDescriptor(this), (off_t)iOffset, SEEK_SET);
		}
		else if (eSeekOrigin == EFSO_CURRENT)
		{
			iResult = lseek(GetDescriptor(this), (off_t)iOffset, SEEK_CUR);
		}
		else if (eSeekOrigin == EFSO_END)
		{
			iResult = lseek(GetDescriptor(this), (off_t)iOffset, SEEK_END);
		}
		else
		{
			return false;
		}
		return iResult != -1;
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
	ssize_t	iWritten;

	if (IsOpen() && (iSize != 0) && (iCount != 0))
	{
		iWritten = write(GetDescriptor(this), pvBuffer, iSize * iCount);
		if (iWritten >= 0)
		{
			return (size)iWritten / iSize;
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
	off_t	iResult;

	if (IsOpen())
	{
		iResult = lseek(GetDescriptor(this), 0, SEEK_CUR);
		if (iResult != -1)
		{
			return iResult;
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
	return !IsDescriptor(this, INVALID_FILE_DESCRIPTOR);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Size(void)
{
	struct stat	sStat;

	if (IsOpen())
	{
		if (fstat(GetDescriptor(this), &sStat) == 0)
		{
			return sStat.st_size;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Truncate(filePos iSize)
{
	off_t	iResult;

	//Windows SetEndOfFile leaves the file pointer at the new end.
	iResult = lseek(GetDescriptor(this), (off_t)iSize, SEEK_SET);
	if (iResult == -1)
	{
		return false;
	}
	return ftruncate(GetDescriptor(this), (off_t)iSize) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiskFile::Flush(void)
{
	if (IsOpen())
	{
		return fsync(GetDescriptor(this)) == 0;
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

#endif //LINUX_GNU_32

