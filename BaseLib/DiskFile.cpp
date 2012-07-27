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
void CDiskFile::Init(char* szFileName)
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

	if (eMode == EFM_Read)
	{
		strcpy(pcMode,"rb");
	}
	else if (eMode == EFM_Read_Create)
	{
		Create();
		strcpy(pcMode,"rb");
	}
	else if (eMode == EFM_Write_Create)
	{
		strcpy(pcMode,"wb");
	}
	else if (eMode == EFM_ReadWrite)
	{
		strcpy(pcMode,"r+b");
	}
	else if (eMode == EFM_ReadWrite_Create)
	{
		Create();
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
		//File closed safetly.
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

	iRead = (filePos)fread(pvBuffer, (size_t)iSize, (size_t)iCount, mpsFileHandle);
	return iRead;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Seek(filePos iOffset, int iSeekOrigin)
{
	return FixBool(fseek(mpsFileHandle, (size_t)iOffset, iSeekOrigin));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Write(const void* pvBuffer, filePos iSize, filePos iCount)
{
	filePos	iWritten;

	iWritten = (filePos)fwrite(pvBuffer, (size_t)iSize, (size_t)iCount, mpsFileHandle);
	return iWritten;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDiskFile::Tell(void)
{
	return ftell(mpsFileHandle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDiskFile::Eof(void)
{
	return FixBool(feof(mpsFileHandle));
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
	return fflush(mpsFileHandle) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDiskFile* DiskFile(char* szName)
{
	CDiskFile* pcDiskFile;

	pcDiskFile = Malloc(CDiskFile);
	pcDiskFile->Init(szName);
	pcDiskFile->mbBasicFileMustFree = TRUE;
	return pcDiskFile;
}

