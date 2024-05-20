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
bool CIndexedFile::Init(CDurableFileController* pcDurableFileControl, size uiFileIndex, char* szFilename, char* szRewriteName, size uiDataSize, uint32 uiFileNum)
{
	muiFileIndex = uiFileIndex;
	muiDataSize = uiDataSize;
	muiFileNumber = uiFileNum;

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
 filePos CIndexedFile::Write(void* pvData)
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
filePos CIndexedFile::Write(void* pvData, size uiCount)
{
	filePos		iFilePos;
	size		uiWritten;
	filePos		iDataIndex;

	if (uiCount == 0)
	{
		return 0;
	}

	iFilePos = mcFile.Size();
	if ((iFilePos % muiDataSize) != 0)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	uiWritten = mcFile.Write(EFSO_END, 0, pvData, muiDataSize, uiCount);
	if (uiWritten != uiCount)
	{
		return INDEXED_FILE_WRITE_ERROR;
	}

	miNumDatas += uiCount;

	iDataIndex = iFilePos / muiDataSize;
	if (iDataIndex < MAX_UINT)
	{
		return iDataIndex;
	}
	return INDEXED_FILE_WRITE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Write(filePos iIndex, void* pvData, size uiCount)
{
	size		uiWritten;
	filePos		iFileLengh;
	filePos		iPosition;

	if (uiCount == 0)
	{
		return false;
	}

	iPosition = iIndex * muiDataSize;
	uiWritten = mcFile.Write(EFSO_SET, iPosition, pvData, muiDataSize, uiCount);
	if (uiWritten != uiCount)
	{
		return false;
	}

	if (iIndex + uiCount > miNumDatas)
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
bool CIndexedFile::Read(filePos iIndex, void* pvData, size uiCount)
{
	size		iRead;
	filePos		iPosition;

	if (uiCount == 0)
	{
		return false;
	}
	if (iIndex + uiCount > miNumDatas)
	{
		return false;
	}

	iPosition = iIndex * muiDataSize;
	iRead = mcFile.Read(EFSO_SET, iPosition, pvData, muiDataSize, uiCount);
	return iRead == uiCount;
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
bool CIndexedFile::Delete(filePos iIndex, size uiCount)
{
	size			uiWritten;
	filePos			iPosition;
	void*			pvData;
	CStackMemory<>	cTemp;
	size			uiSize;

	if (uiCount == 0)
	{
		return false;
	}

	if (iIndex + uiCount > miNumDatas)
	{
		//This is wrong.  You need to loop multiple times if miNumDatas - iIndex is > than SIZE_MAX
		if (miNumDatas <= iIndex)
		{
			return true;
		}
		uiCount = (size)(miNumDatas - iIndex);
	}

	uiSize = uiCount * muiDataSize;
	iPosition = iIndex * muiDataSize;

	pvData = cTemp.Init(uiSize);
	memset(pvData, INDEX_FILE_EMPTY_CHAR, uiSize);
	uiWritten = mcFile.Write(EFSO_SET, iPosition, pvData, muiDataSize, uiCount);
	cTemp.Kill();

	return uiWritten == uiCount;
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
	filePos			i;
	filePos			iSizeOnDisk;
	filePos			iPos;
	char			pvData[80];
	size			uiReadSize;
	CFileBasic*		pcFile;
	filePos			iNumDatas;

	pcFile = mcFile.DumpGetPrimaryFile();
	iSizeOnDisk = pcFile->GetFileSize();
	iNumDatas = iSizeOnDisk / muiDataSize;

	sz.Init();
	sz.Append("File Descriptor (");
	sz.Append(muiFileIndex);
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
	sz.Append(iNumDatas);
	sz.Append(")\n\n");
	sz.Append("Data on Disk\n------------\n");

	uiReadSize = 80;
	if (muiDataSize < uiReadSize)
	{
		uiReadSize = muiDataSize;
	}

	iPos = mcFile.Tell();
	for (i = 0; i < iNumDatas; i++)
	{
		pcFile->Seek(i * muiDataSize, EFSO_SET);
		pcFile->Read(pvData, uiReadSize, 1);
		sz.AppendData(pvData, uiReadSize);
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
filePos CIndexedFile::GetUsedDataIndices(CArrayBit* pab)
{
	CStackMemory<>	cCompare;
	CStackMemory<>	cData;
	void*			pvCompare;
	filePos			i;
	void*			pv;
	bool			bUsed;
	filePos			iUsed;

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
size CIndexedFile::GetFileIndex(void) { return muiFileIndex; }
uint32 CIndexedFile::GetFileNumber(void) { return muiFileNumber; }
bool CIndexedFile::IsFileIndex(size uiFileIndex) { return muiFileIndex == uiFileIndex; }
char* CIndexedFile::GetFilename(void) { return mcFile.GetFilename(); }
char* CIndexedFile::GetRewriteName(void) { return mcFile.GetRewriteName(); }
size CIndexedFile::GetDataSize(void) { return muiDataSize; }
filePos	CIndexedFile::NumDatas(void) { return miNumDatas; }
filePos	CIndexedFile::GetFileSize(void) { return mcFile.Size(); };

