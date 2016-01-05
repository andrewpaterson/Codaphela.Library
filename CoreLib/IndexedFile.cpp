/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "IndexedFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFileName, char* szRewriteName, int iDataSize, int iFileNum)
{
	miFileIndex = iFileIndex;
	mszFileName.Init(szFileName);
	mszRewriteName.Init(szRewriteName);
	miDataSize = iDataSize;
	miFileNumber = iFileNum;

	mcFile.Init(pcDurableFileControl, mszFileName.Text(), mszRewriteName.Text());
	pcDurableFileControl->AddFile(&mcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Kill(void)
{
	miNumDatas = 0;
	mszFileName.Kill();
	mszRewriteName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Open(CDurableFileController* pcDurableFileControl)
{
	filePos		iFileLengh;
	BOOL		bResult;

	bResult = mcFile.Open();
	if (!bResult)
	{
		return FALSE;
	}

	iFileLengh = mcFile.Size();
	miNumDatas = (iFileLengh / miDataSize);

	bResult = TRUE;
	if (pcDurableFileControl->IsBegun())
	{
		bResult &= mcFile.Begin();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Close(void)
{
	BOOL bResult;

	bResult = mcFile.Close();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::IsFull(void)
{
	if (miNumDatas >= 0xffffff)  //16 million.
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexedFile::Write(void* pvData)
{
	return Write(pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Write(filePos iIndex, void* pvData)
{
	filePos		iResult;

	iResult = Write(iIndex, pvData, 1);
	return iResult != -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexedFile::Write(void* pvData, filePos iCount)
{
	filePos		iFilePos;
	filePos		iWritten;

	if (iCount == 0)
	{
		return FALSE;
	}

	iFilePos = mcFile.Size();
	if ((iFilePos % miDataSize) != 0)
	{
		return -1;
	}
	iWritten = mcFile.Write(EFSO_END, 0, pvData, miDataSize, iCount);
	if (iWritten != (filePos)iCount)
	{
		return -1;
	}

	miNumDatas += iCount;

	return iFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Write(filePos iIndex, void* pvData, filePos iCount)
{
	filePos		iWritten;
	filePos		iFileLengh;
	filePos		iPosition;

	if (iCount == 0)
	{
		return FALSE;
	}

	iPosition = iIndex * miDataSize;
	iWritten = mcFile.Write(EFSO_SET, iPosition, pvData, miDataSize, iCount);
	if (iWritten != (filePos)iCount)
	{
		return FALSE;
	}

	if (iIndex + iCount > miNumDatas)
	{
		iFileLengh = mcFile.Size();
		miNumDatas = iFileLengh / miDataSize;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Read(filePos iIndex, void* pvData)
{
	return Read(iIndex, pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Read(filePos iIndex, void* pvData, filePos iCount)
{
	filePos		iRead;
	filePos		iPosition;

	if (iCount == 0)
	{
		return FALSE;
	}
	if (iIndex + iCount > miNumDatas)
	{
		return FALSE;
	}

	iPosition = iIndex * miDataSize;
	iRead = mcFile.Read(EFSO_SET, iPosition, pvData, miDataSize, iCount);
	return iRead == iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileBasic*	CIndexedFile::GetPrimaryFile(void)
{
	return mcFile.mpcPrimaryFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileBasic*	CIndexedFile::GetBackupFile(void)
{
	return mcFile.mpcRewriteFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Dump(void)
{
	CChars		sz;
	int			i;
	filePos		iSizeOnDisk;
	filePos		iPos;
	char		pvData[80];
	int			iReadSize;

	iSizeOnDisk = mcFile.SizeFromFile();

	sz.Init();
	sz.Append("File Descriptor (");
	sz.Append(miFileIndex);
	sz.Append(")\n------------------\n");
	sz.Append("Primary Name: ");
	sz.Append(mszFileName);
	sz.AppendNewLine();
	sz.Append("Rewrite Name: ");
	sz.Append(mszRewriteName);
	sz.AppendNewLine();
	sz.Append("Durable: ");
	sz.AppendBool(mcFile.IsDurable());
	sz.AppendNewLine();
	sz.Append("Datas (A:");
	sz.Append(miNumDatas);
	sz.Append(", D:");
	sz.Append(mcFile.miLength/miDataSize);
	sz.Append(", F:");
	sz.Append(iSizeOnDisk/miDataSize);
	sz.Append(")\n\n");
	sz.Append("Data on Disk\n------------\n");

	iReadSize = 80;
	if (miDataSize < iReadSize)
	{
		iReadSize = miDataSize;
	}

	iPos = mcFile.Tell();
	for (i = 0; i < iSizeOnDisk/miDataSize; i++)
	{
		mcFile.Seek(EFSO_SET, i * miDataSize);
		mcFile.ReadFromFile(pvData, iReadSize, 1);
		sz.AppendData(pvData, iReadSize);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();


	mcFile.Seek(EFSO_SET, iPos);
	sz.Dump();
	sz.Kill();

	mcFile.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFile::GetFileIndex(void) { return miFileIndex; }
BOOL CIndexedFile::IsFileIndex(int iFileIndex) { return miFileIndex == iFileIndex; }
char* CIndexedFile::GetFileName(void) { return mszFileName.Text(); }
