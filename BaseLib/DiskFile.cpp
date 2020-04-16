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
#include "DiskFile.h"
#include "IntegerHelper.h"
#include "FileUtil.h"
#include "ConstructorCall.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(const char* szFileName)
{
	CAbstractFile::Init();
	mpsFileHandle = NULL;
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Init(CChars szFileName)
{
	CAbstractFile::Init();
	mpsFileHandle = NULL;
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiskFile::Kill(void)
{
	mszFileName.Kill();
	mpsFileHandle = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Open(EFileMode eMode)
{
	char	pcMode[4];

	if (mpsFileHandle != NULL)
	{
		Close();
	}

	if (IsFileModeCreate(eMode))
	{
		Create();
	}

	if ((eMode == EFM_Read) || (eMode == EFM_Read_Create))
	{
		strcpy(pcMode,"rb");
	}
	else if (eMode == EFM_Write_Create)
	{
		strcpy(pcMode,"wb");
	}
	else if ((eMode == EFM_ReadWrite) || (eMode == EFM_ReadWrite_Create))
	{
		strcpy(pcMode,"r+b");
	}

	mpsFileHandle = fopen(mszFileName.Text(), pcMode);
	return FixBool(mpsFileHandle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Close(void)
{
	int		iReturn;

	if (!mpsFileHandle)
	{
		return FALSE;
	}

	iReturn = fclose(mpsFileHandle);
	if (iReturn == 0)
	{
		//File closed safely.
		mpsFileHandle = NULL;
	}
	return iReturn == 0;
}


//====================================================================================
//=
//=
//====================================================================================
BOOL CDiskFile::Create(void)
{
	FILE*	psFileHandle;

	psFileHandle = fopen(mszFileName.Text(), "rb");
	if (psFileHandle == NULL)
	{
		psFileHandle = fopen(mszFileName.Text(), "wb");
		if (psFileHandle == NULL)
		{
			return FALSE;
		}
	}
	fclose(psFileHandle);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Read(void* pvBuffer, filePos iSize, filePos iCount)
{
	filePos		iRead;

	if (IsOpen())
	{
		iRead = (filePos)fread(pvBuffer, (size_t)iSize, (size_t)iCount, mpsFileHandle);
		return iRead;
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
	int		iResult;

	if (IsOpen())
	{
		iResult = fseek(mpsFileHandle, (size_t)iOffset, iSeekOrigin);
		return iResult == 0;
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
	filePos	iWritten;

	if (IsOpen())
	{
		iWritten = (filePos)fwrite(pvBuffer, (size_t)iSize, (size_t)iCount, mpsFileHandle);
		return iWritten;
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
	if (IsOpen())
	{
		return ftell(mpsFileHandle);
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
BOOL CDiskFile::Eof(void)
{
	if (IsOpen())
	{
		return FixBool(feof(mpsFileHandle));
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::IsOpen(void)
{
	return mpsFileHandle != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Size(void)
{
	filePos	iCurrPos;
	filePos	iFileSize;

	if (mpsFileHandle)
	{
		iCurrPos = ftell(mpsFileHandle);
		fseek(mpsFileHandle, 0, SEEK_END);
		iFileSize = ftell(mpsFileHandle);
		fseek(mpsFileHandle, (int)iCurrPos, SEEK_SET);
		return iFileSize;	
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
BOOL CDiskFile::Flush(void)
{
	if (IsOpen())
	{
		return fflush(mpsFileHandle) == 0;
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

