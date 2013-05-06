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
#include "TextFile.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::Init(void)
{
	mcText.Init(1024);
	mszFileName.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::Kill(void)
{
	mcText.Kill();
	KillExceptBuffer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::KillExceptBuffer(void)
{
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextFile::Read(char* szFileName)
{
	CDiskFile*	pcDiskFile;
	
    if (szFileName)
	{
		mszFileName.Kill();
		mszFileName.Init(szFileName);
		pcDiskFile = DiskFile(szFileName);
		return Read(pcDiskFile);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextFile::Read(CAbstractFile* pcAbstractFile)
{
	CFileBasic	mcFile;
	filePos		iSize;

	mcFile.Init(pcAbstractFile);
	if (mcFile.Open(EFM_Read))
	{
		iSize = mcFile.GetFileSize();

		mcText.SetLength((int)iSize);
		mcFile.ReadData(mcText.Text(), (int)iSize);
		mcFile.Close();
		mcFile.Kill();
		return TRUE;
	}
	mcFile.Kill();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTextFile::Write(char* szFileName)
{
	CFileBasic	mcFile;

	if (szFileName)
	{
		mszFileName.Kill();
		mszFileName.Init(szFileName);
		mcFile.Init(DiskFile(szFileName));
		if (mcFile.Open(EFM_Write_Create))
		{
			mcFile.WriteData(mcText.Text(), mcText.Length());
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
char* CTextFile::Text(void)
{
	return mcText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTextFile::Length(void)
{
	return mcText.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::PassifyNewlines(void)
{
	mcText.PassifyNewlines();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::Dump(void)
{
	mcText.Dump();
}

