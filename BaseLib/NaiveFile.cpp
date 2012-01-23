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
#include "NaiveFile.h"
#include "PointerFunctions.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNaiveFile::Init(void)
{
	mpvMem = NULL;
	miSize = 0;
	mszFileName.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNaiveFile::Kill(void)
{
	SafeFree(mpvMem);
	miSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNaiveFile::Read(char* szFileName)
{
	CFileBasic	mcFile;

	if (szFileName)
	{
		mszFileName.Kill();
		mszFileName.Init(szFileName);
		mcFile.Init(DiskFile(szFileName));
		if (mcFile.Open(EFM_Read))
		{
			miSize = mcFile.GetFileSize();
			
			mpvMem = malloc((int)miSize);
			mcFile.Read(mpvMem, (int)miSize, 1);
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
BOOL CNaiveFile::Write(char* szFileName)
{
	CFileBasic	mcFile;

	mcFile.Init(DiskFile(szFileName));

	if (szFileName)
	{
		mszFileName.Kill();
		mszFileName.Init(szFileName);
		if (mcFile.Open(EFM_Write_Create))
		{
			mcFile.Write(mpvMem, (int)miSize, 1);
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
int CNaiveFile::Compare(void* pvOther)
{
	int		iResult;
	
	if ((!pvOther && mpvMem) || (pvOther && !mpvMem))
	{
		return -1;
	}

	iResult = memcmp(mpvMem, pvOther, (int)miSize);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareFileToMemory(char* szFileName, void* pvMem, int iLength, CChars* psz)
{
	CNaiveFile	cNaiveFile;
	int			iResult;

	cNaiveFile.Init();
	if (!cNaiveFile.Read(szFileName))
	{
		cNaiveFile.Kill();
		return -1;
	}
	iResult = cNaiveFile.Compare(pvMem);
	cNaiveFile.Kill();
	return iResult;
}

