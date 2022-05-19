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
#include "NaiveFile.h"
#include "PointerFunctions.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNaiveFile::CNaiveFile()
{
	mpvMem = NULL;
	miSize = 0;
	mszFilename._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNaiveFile* CNaiveFile::Init(void)
{
	mpvMem = NULL;
	miSize = 0;
	mszFilename.Init();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNaiveFile* CNaiveFile::Init(void* pvMem, int iSize)
{
	Init();
	Set(pvMem, iSize);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNaiveFile::Kill(void)
{
	SafeFree(mpvMem);
	KillExceptBuffer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNaiveFile::KillExceptBuffer(void)
{
	miSize = 0;
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNaiveFile::Read(const char* szFilename)
{
	CFileBasic	mcFile;
	CDiskFile*	pcDiskFile;

	if (szFilename)
	{
		mszFilename.Kill();
		mszFilename.Init(szFilename);
		pcDiskFile = DiskFile(szFilename);
		return Read(pcDiskFile);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNaiveFile::Read(CAbstractFile* pcAbstractFile)
{
	CFileBasic	mcFile;

	if (pcAbstractFile)
	{
		mcFile.Init(pcAbstractFile);
		if (mcFile.Open(EFM_Read))
		{
			miSize = mcFile.GetFileSize();

			mpvMem = malloc((int)miSize);
			mcFile.ReadData(mpvMem, (int)miSize);
			mcFile.Close();
			mcFile.Kill();
			return TRUE;
		}
		mcFile.Kill();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNaiveFile::Write(const char* szFilename)
{
	CFileBasic	mcFile;

	mcFile.Init(DiskFile(szFilename));

	if (szFilename)
	{
		mszFilename.Kill();
		mszFilename.Init(szFilename);
		if (mcFile.Open(EFM_Write_Create))
		{
			mcFile.WriteData(mpvMem, (int)miSize);
			mcFile.Close();
			mcFile.Kill();
			return TRUE;
		}
		mcFile.Kill();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNaiveFile::Get(void)
{
	return mpvMem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CNaiveFile::Get(int iOffset)
{
	return RemapSinglePointer(mpvMem, iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNaiveFile::Set(void* pvMem, int iSize)
{
	SafeFree(mpvMem);
	mpvMem = malloc(iSize);
	miSize = iSize;
	memcpy(mpvMem, pvMem, (int)miSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNaiveFile::Size()
{
	return miSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNaiveFile::Compare(void* pvOther, int iSize)
{
	int		iResult;
	
	if ((!pvOther && mpvMem) || (pvOther && !mpvMem))
	{
		return -1;
	}

	if (iSize < miSize)
	{
		return -1;
	}
	if (iSize > miSize)
	{
		return 1;
	}

	iResult = memcmp(mpvMem, pvOther, (int)miSize);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareFileToMemory(const char* szFilename, void* pvMem, int iSize)
{
	CNaiveFile	cNaiveFile;
	int			iResult;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFilename))
	{
		cNaiveFile.Kill();
		return -1;
	}
	iResult = cNaiveFile.Compare(pvMem, iSize);
	cNaiveFile.Kill();
	return iResult;
}

