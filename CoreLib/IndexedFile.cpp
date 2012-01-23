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
	filePos		iFileLengh;

	miFileIndex = iFileIndex;
	mszFileName.Init(szFileName);
	mszRewriteName.Init(szRewriteName);
	miDataSize = iDataSize;
	miFileNumber = iFileNum;

	mcFile.Init(pcDurableFileControl->mbDurable, mszFileName.Text(), mszRewriteName.Text());
	pcDurableFileControl->AddFile(&mcFile);
	mcFile.Open();

	iFileLengh = mcFile.Size();
	miNumDatas = (int)(iFileLengh / iDataSize);
	mbNew = TRUE;

	if (pcDurableFileControl->mcDurableSet.HasBegun())
	{
		mcFile.Begin();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Kill(void)
{
	miNumDatas = 0;
	mcFile.Close();
	mszFileName.Kill();
	mszRewriteName.Kill();
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
int CIndexedFile::Write(void* pvData)
{
	filePos		iIndex;
	int			iWritten;

	iIndex = mcFile.Size();
	if ((iIndex % miDataSize) != 0)
	{
		return -1;
	}
	iWritten = mcFile.Write(EFSO_END, 0, pvData, miDataSize, 1);
	miNumDatas++;

	if (iWritten != 1)
	{
		return -1;
	}

	return (int)(iIndex / miDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Write(int iIndex, void* pvData)
{
	int			iWritten;
	filePos		iFileLengh;
	filePos		iPosition;

	iPosition = iIndex*miDataSize;
	iWritten = mcFile.Write(EFSO_SET, (int)iPosition, pvData, miDataSize, 1);
	if (iWritten != 1)
	{
		return FALSE;
	}

	if (iIndex > miNumDatas)
	{
		iFileLengh = mcFile.Size();
		miNumDatas = (int)(iFileLengh / miDataSize);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Read(int iIndex, void* pvData)
{
	int				iRead;
	unsigned int	iPosition;

	if (iIndex > miNumDatas)
	{
		return FALSE;
	}

	iPosition = iIndex*miDataSize;
	iRead = mcFile.Read(EFSO_SET, iPosition, pvData, miDataSize, 1);
	if (iRead != 1)
	{
		return FALSE;
	}
	return TRUE;
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
	BOOL		bDurable;
	char		pvData[80];
	int			iReadSize;

	iSizeOnDisk = mcFile.SizeFromFile();
	bDurable = mcFile.mbDurable;
	mcFile.mbDurable = FALSE;

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
	sz.Append("New: ");
	sz.AppendBool(mbNew);
	sz.AppendNewLine();
	sz.Append("Durable: ");
	sz.AppendBool(bDurable);
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
		mcFile.Read(i*miDataSize, pvData, iReadSize, 1);
		sz.AppendData(pvData, iReadSize);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();


	mcFile.Seek(EFSO_SET, iPos);
	sz.Dump();
	sz.Kill();

	mcFile.Dump();

	mcFile.mbDurable = bDurable;
}

