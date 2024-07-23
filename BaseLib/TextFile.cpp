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
#include "TextFile.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTextFile* CTextFile::Init(void)
{
	mcText.Init();
	mszFilename.Init();
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextFile::Init(char* szText)
{
	mcText.Init();
	mcText.Append(szText);
	mszFilename.Init();
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
	mszFilename.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextFile::Read(char* szFilename)
{
	CDiskFile*	pcDiskFile;
	
    if (szFilename)
	{
		mszFilename.Kill();
		mszFilename.Init(szFilename);
		pcDiskFile = DiskFile(szFilename);
		return Read(pcDiskFile);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextFile::Read(CAbstractFile* pcAbstractFile)
{
	CFileBasic	mcFile;
	filePos		iSize;

	mcFile.Init(pcAbstractFile);
	if (mcFile.Open(EFM_Read))
	{
		iSize = mcFile.Size();

		mcText.SetLength((int)iSize);
		mcFile.ReadData(mcText.Text(), (int)iSize);
		mcFile.Close();
		mcFile.Kill();
		return true;
	}
	mcFile.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextFile::Write(char* szFilename)
{
	CFileBasic	mcFile;

	if (szFilename)
	{
		mszFilename.Kill();
		mszFilename.Init(szFilename);
		mcFile.Init(DiskFile(szFilename));
		if (mcFile.Open(EFM_Write_Create))
		{
			mcFile.WriteData(mcText.Text(), mcText.Length());
			mcFile.Close();
			mcFile.Kill();
			return true;
		}
		mcFile.Kill();
	}
	return false;
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
char* CTextFile::Text(int iOffset)
{
	return mcText.Text(iOffset);
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

