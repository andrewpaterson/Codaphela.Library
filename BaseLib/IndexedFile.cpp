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
#include "StackMemory.h"
#include "IndexedFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFilename, char* szRewriteName, uint32 uiDataSize, int iFileNum)
{
	miFileIndex = iFileIndex;
	muiDataSize = uiDataSize;
	miFileNumber = iFileNum;

	mcFile.Init(pcDurableFileControl, szFilename, szRewriteName);
	miNumDatas = CalculateNumDatas();
	return miNumDatas >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFile::Kill(void)
{
	mcFile.Kill();
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

	if (iFileLength % muiDataSize != 0)
	{
		//Log this error properly.
		return -1;
	}

	return (iFileLength / muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::IsFull(void)
{
	if (miNumDatas >= 0xffffff)  //16 million.
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
 uint32 CIndexedFile::Write(void* pvData)
{
	return Write(pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Write(filePos iIndex, void* pvData)
{
	return Write(iIndex, pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexedFile::Write(void* pvData, filePos iCount)
{
	filePos		iFilePos;
	filePos		iWritten;
	filePos		iDataIndex;

	if (iCount == 0)
	{
		return 0;
	}

	iFilePos = mcFile.Size();
	if ((iFilePos % muiDataSize) != 0)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	iWritten = mcFile.Write(EFSO_END, 0, pvData, muiDataSize, iCount);
	if (iWritten != (filePos)iCount)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	miNumDatas += iCount;

	iDataIndex = iFilePos / muiDataSize;
	if (iDataIndex < MAX_UINT)
	{
		return (uint32)iDataIndex;
	}
	return INDEXED_FILE_WRITE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Write(filePos iIndex, void* pvData, filePos iCount)
{
	filePos		iWritten;
	filePos		iFileLengh;
	filePos		iPosition;

	if (iCount == 0)
	{
		return false;
	}

	iPosition = iIndex * muiDataSize;
	iWritten = mcFile.Write(EFSO_SET, iPosition, pvData, muiDataSize, iCount);
	if (iWritten != (filePos)iCount)
	{
		return false;
	}

	if (iIndex + iCount > miNumDatas)
	{
		iFileLengh = mcFile.Size();
		miNumDatas = iFileLengh / muiDataSize;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Read(filePos iIndex, void* pvData)
{
	return Read(iIndex, pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Read(filePos iIndex, void* pvData, filePos iCount)
{
	filePos		iRead;
	filePos		iPosition;

	if (iCount == 0)
	{
		return false;
	}
	if (iIndex + iCount > miNumDatas)
	{
		return false;
	}

	iPosition = iIndex * muiDataSize;
	iRead = mcFile.Read(EFSO_SET, iPosition, pvData, muiDataSize, iCount);
	return iRead == iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Delete(filePos iIndex)
{
	return Delete(iIndex, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Delete(filePos iIndex, filePos iCount)
{
	filePos			iWritten;
	filePos			iPosition;
	void*			pvData;
	CStackMemory<>	cTemp;
	size_t			iSize;

	if (iCount == 0)
	{
		return false;
	}

	if (iIndex + iCount > miNumDatas)
	{
		iCount = miNumDatas - iIndex;
		if (iCount <= 0)
		{
			return true;
		}
	}

	iSize = (size_t)iCount * (size_t)muiDataSize;
	iPosition = iIndex * muiDataSize;

	pvData = cTemp.Init(iSize);
	memset(pvData, INDEX_FILE_EMPTY_CHAR, iSize);
	iWritten = mcFile.Write(EFSO_SET, iPosition, pvData, muiDataSize, iCount);
	cTemp.Kill();

	if (iWritten != (filePos)iCount)
	{
		return false;
	}

	return true;
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
	CChars			sz;
	int				i;
	filePos			iSizeOnDisk;
	filePos			iPos;
	char			pvData[80];
	uint32	iReadSize;
	CFileBasic*		pcFile;

	pcFile = mcFile.DumpGetPrimaryFile();
	iSizeOnDisk = pcFile->GetFileSize();

	sz.Init();
	sz.Append("File Descriptor (");
	sz.Append(miFileIndex);
	sz.Append(")\n------------------\n");
	sz.Append("Primary Name: ");
	sz.Append(mcFile.GetFilename());
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
	sz.Append(mcFile.Size() / muiDataSize);
	sz.Append(", F:");
	sz.Append(iSizeOnDisk/muiDataSize);
	sz.Append(")\n\n");
	sz.Append("Data on Disk\n------------\n");

	iReadSize = 80;
	if (muiDataSize < iReadSize)
	{
		iReadSize = muiDataSize;
	}

	iPos = mcFile.Tell();
	for (i = 0; i < iSizeOnDisk/muiDataSize; i++)
	{
		pcFile->Seek(i * muiDataSize, EFSO_SET);
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
int CIndexedFile::GetUsedDataIndices(CArrayBit* pab)
{
	CStackMemory<>	cCompare;
	CStackMemory<>	cData;
	void*			pvCompare;
	int				i;
	void*			pv;
	bool			bUsed;
	int				iUsed;

	pvCompare = cCompare.Init(muiDataSize);
	memset(pvCompare, INDEX_FILE_EMPTY_CHAR, muiDataSize);

	iUsed = 0;
	pv = cData.Init(muiDataSize);
	for (i = 0; i < miNumDatas; i++)
	{
		if (!Read(i, pv))
		{
			return -1;
		}

		bUsed = memcmp(pv, pvCompare, muiDataSize) != 0;
		pab->Add(bUsed);

		if (bUsed)
		{
			iUsed++;
		}
	}
	cData.Kill();
	cCompare.Kill();

	return iUsed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFile::GetFileIndex(void) { return miFileIndex; }
int CIndexedFile::GetFileNumber(void) { return miFileNumber; }
bool CIndexedFile::IsFileIndex(int iFileIndex) { return miFileIndex == iFileIndex; }
char* CIndexedFile::GetFilename(void) { return mcFile.GetFilename(); }
char* CIndexedFile::GetRewriteName(void) { return mcFile.GetRewriteName(); }
uint32 CIndexedFile::GetDataSize(void) { return muiDataSize; }
filePos	CIndexedFile::NumDatas(void) { return miNumDatas; }
filePos	CIndexedFile::GetFileSize(void) { return mcFile.Size(); };

