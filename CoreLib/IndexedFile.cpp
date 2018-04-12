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
#include "BaseLib/StackMemory.h"
#include "IndexedFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFileName, char* szRewriteName, int iDataSize, int iFileNum)
{
	miFileIndex = iFileIndex;
	miDataSize = iDataSize;
	miFileNumber = iFileNum;

	mcFile.Init(pcDurableFileControl, szFileName, szRewriteName);
	miNumDatas = CalculateNumDatas();
	return miNumDatas >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Kill(void)
{
	// mcFile.Kill();  //Shouldn't this be called?
	miNumDatas = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexedFile::CalculateNumDatas(void)
{
	filePos		iFileLength;

	iFileLength = mcFile.Size();
	if (iFileLength == -1)
	{
		return -1;
	}

	if (iFileLength % miDataSize != 0)
	{
		//Log this error properly.
		return -1;
	}

	return (iFileLength / miDataSize);
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
 unsigned int CIndexedFile::Write(void* pvData)
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
	return iResult != INDEXED_FILE_WRITE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedFile::Write(void* pvData, filePos iCount)
{
	filePos		iFilePos;
	filePos		iWritten;
	filePos		iDataIndex;

	if (iCount == 0)
	{
		return FALSE;
	}

	iFilePos = mcFile.Size();
	if ((iFilePos % miDataSize) != 0)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	iWritten = mcFile.Write(EFSO_END, 0, pvData, miDataSize, iCount);
	if (iWritten != (filePos)iCount)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	miNumDatas += iCount;

	iDataIndex = iFilePos / miDataSize;
	if (iDataIndex < MAX_UINT)
	{
		return (unsigned int)iDataIndex;
	}
	return INDEXED_FILE_WRITE_ERROR;
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
BOOL CIndexedFile::Delete(filePos iIndex)
{
	return Delete(iIndex, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFile::Delete(filePos iIndex, filePos iCount)
{
	filePos			iWritten;
	filePos			iPosition;
	void*			pvData;
	CStackMemory<>	cTemp;
	size_t			iSize;

	if (iCount == 0)
	{
		return FALSE;
	}

	if (iIndex + iCount > miNumDatas)
	{
		iCount = miNumDatas - iIndex;
		if (iCount <= 0)
		{
			return TRUE;
		}
	}

	iSize = (size_t)iCount * (size_t)miDataSize;
	iPosition = iIndex * miDataSize;

	pvData = cTemp.Init(iSize);
	memset(pvData, 0x55, iSize);
	iWritten = mcFile.Write(EFSO_SET, iPosition, pvData, miDataSize, iCount);
	cTemp.Kill();

	if (iWritten != (filePos)iCount)
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileBasic*	CIndexedFile::DumpGetPrimaryFile(void)
{
	return mcFile.DumpGetPrimaryFile();
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
	CFileBasic*	pcFile;

	pcFile = mcFile.DumpGetPrimaryFile();
	iSizeOnDisk = pcFile->GetFileSize();

	sz.Init();
	sz.Append("File Descriptor (");
	sz.Append(miFileIndex);
	sz.Append(")\n------------------\n");
	sz.Append("Primary Name: ");
	sz.Append(mcFile.GetFileName());
	sz.AppendNewLine();
	sz.Append("Rewrite Name: ");
	sz.Append(mcFile.GetRewriteName());
	sz.AppendNewLine();
	sz.Append("Durable: ");
	sz.AppendBool(mcFile.IsDurable());
	sz.AppendNewLine();
	sz.Append("Datas (A:");
	sz.Append(miNumDatas);
	sz.Append(", D:");
	sz.Append(mcFile.Size() / miDataSize);
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
		pcFile->Seek(i * miDataSize, EFSO_SET);
		pcFile->Read(pvData, iReadSize, 1);
		sz.AppendData(pvData, iReadSize);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();


	pcFile->Seek(iPos, EFSO_SET);
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
char* CIndexedFile::GetFileName(void) { return mcFile.GetFileName(); }
char* CIndexedFile::GetRewriteName(void) { return mcFile.GetRewriteName(); }

