/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "DataTypes.h"
#include "StackMemory.h"
#include "IndexedFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Init(CDurableFileController* pcDurableFileControl, size uiFileIndex, const char* szFilename, const char* szRewriteName, lsize uiDataSize, uint32 uiFileNum)
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
 filePos CIndexedFile::Write(const void* pvData)
{
	return Write(pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Write(filePos iIndex, const void* pvData)
{
	return Write(iIndex, pvData, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexedFile::Write(const void* pvData, lsize uiCount)
{
	filePos		iFilePos;
	lsize		uiWritten;
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
	if (iDataIndex < MAX_UINT32)
	{
		return iDataIndex;
	}
	return INDEXED_FILE_WRITE_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFile::Write(filePos iIndex, const void* pvData, lsize uiCount)
{
	lsize		uiWritten;
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

	if (iIndex + (filePos)uiCount > miNumDatas)
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
bool CIndexedFile::Read(filePos iIndex, void* pvData, lsize uiCount)
{
	lsize		iRead;
	filePos		iPosition;

	if (uiCount == 0)
	{
		return false;
	}
	if (iIndex + (filePos)uiCount > miNumDatas)
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
bool CIndexedFile::Delete(filePos iIndex, lsize uiCount)
{
	lsize				uiWritten;
	filePos				iPosition;
	void*				pvData;
	CStackMemory<2 KB>	cTemp;
	lsize				uiChunkCount;
	lsize				uiChunk;

	if (uiCount == 0)
	{
		return false;
	}

	if (iIndex + (filePos)uiCount > miNumDatas)
	{
		if (miNumDatas <= iIndex)
		{
			return true;
		}
		uiCount = (lsize)(miNumDatas - iIndex);
	}

	//The datas are blanked a stack buffer's worth (at least one data) at a time.
	uiChunkCount = (lsize)(2 KB) / muiDataSize;
	if (uiChunkCount == 0)
	{
		uiChunkCount = 1;
	}
	if (uiChunkCount > uiCount)
	{
		uiChunkCount = uiCount;
	}

	pvData = cTemp.Init(uiChunkCount * muiDataSize);
	memset(pvData, INDEX_FILE_EMPTY_CHAR, uiChunkCount * muiDataSize);
	iPosition = iIndex * muiDataSize;
	while (uiCount > 0)
	{
		uiChunk = (uiCount < uiChunkCount) ? uiCount : uiChunkCount;
		uiWritten = mcFile.Write(EFSO_SET, iPosition, pvData, muiDataSize, uiChunk);
		if (uiWritten != uiChunk)
		{
			cTemp.Kill();
			return false;
		}
		iPosition += (filePos)uiChunk * muiDataSize;
		uiCount -= uiChunk;
	}
	cTemp.Kill();
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
	filePos			i;
	filePos			iSizeOnDisk;
	filePos			iPos;
	char			pvData[80];
	size			uiReadSize;
	CFileBasic*		pcFile;
	filePos			iNumDatas;

	pcFile = mcFile.DumpGetPrimaryFile();
	iSizeOnDisk = pcFile->Size();
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
		uiReadSize = (size)muiDataSize;
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
	CStackMemory<2 KB>	cCompare;
	CStackMemory<2 KB>	cData;
	void*				pvCompare;
	filePos				i;
	void*				pv;
	bool				bUsed;
	filePos				iUsed;

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
const char* CIndexedFile::GetFilename(void) { return mcFile.GetFilename(); }
char* CIndexedFile::GetRewriteName(void) { return mcFile.GetRewriteName(); }
lsize CIndexedFile::GetDataSize(void) { return muiDataSize; }
filePos	CIndexedFile::NumDatas(void) { return miNumDatas; }
filePos	CIndexedFile::GetFileSize(void) { return mcFile.Size(); };

