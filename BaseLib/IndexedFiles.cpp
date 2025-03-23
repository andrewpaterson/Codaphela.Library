/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Logger.h"
#include "LogString.h"
#include "PointerRemapper.h"
#include "GlobalMemory.h"
#include "FilePathBuilder.h"
#include "IndexedFiles.h"


struct SIndexedFileDescriptor
{
	uint32	uiFileIndex;
	size	uiDataSize;
	uint32	iFileNum;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, char* szDataExtension, char* szDescriptorName, char* szDescriptorRewrite)
{
	CFilePathBuilder	cPathName;
	CFilePathBuilder	cPathRewrite;

	mpcDurableFileControl = pcDurableFileControl;
	mszDataExtension.Init(szDataExtension);
	mszSubDirectory.Init(szSubDirectory);
	mcFiles.Init();

	mcFileDescriptors.Init(mpcDurableFileControl, cPathName.Build(NullToEmpty(szSubDirectory), szDescriptorName, NULL), cPathRewrite.Build(szSubDirectory, szDescriptorRewrite, NULL));
	cPathName.Kill();
	cPathRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::Kill(void)
{
	size			i; 
	CIndexedFile*	pcIndexedFile;

	if (mpcDurableFileControl->IsBegun())
	{
		return gcLogger.Error2(__METHOD__, " DurableFileController.End must called before Kill.", NULL);
	}

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		pcIndexedFile->Kill();
	}
	mcFiles.Kill();

	mcFileDescriptors.Kill();
	mszSubDirectory.Kill();
	mszDataExtension.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::ReadIndexedFileDescriptors(void)
{
	filePos						iFileSize;
	bool						bResult;
	size						i;
	int64						iNumFiles;
	SIndexedFileDescriptor*		pasFileDescriptors;
	CIndexedFile*				pcIndexedFile;
	char						szDataFilename[MAX_DIRECTORY_LENGTH];
	char						szDataRewriteName[MAX_DIRECTORY_LENGTH];
	size						iRead;
	size						uiRemainder;
	size						uiFileSize;
	size						uiNumFiles;

	if (!mpcDurableFileControl->IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read descriptors if the Durable Controller is not begun.", NULL);
		return false;
	}

	if (mcFiles.NumElements() != 0)
	{
		//Don't read.  Should probably throw an error.
		return false;
	}

	iFileSize = mcFileDescriptors.Size();
	if (iFileSize == 0)
	{
		bResult = mcFileDescriptors.Create();
		return bResult;
	}

	iNumFiles = iFileSize / (sizeof(SIndexedFileDescriptor));
	uiRemainder = iFileSize % (sizeof(SIndexedFileDescriptor));
	if (uiRemainder != 0)
	{
		return false;
	}

	if ((iFileSize > SIZE_MAX) || (iNumFiles > SIZE_MAX))
	{
		return false;
	}
	uiFileSize = (size)iFileSize;
	uiNumFiles = (size)iNumFiles;

	pasFileDescriptors = (SIndexedFileDescriptor*)malloc(uiFileSize);

	iRead = mcFileDescriptors.Read(pasFileDescriptors, sizeof(SIndexedFileDescriptor), uiNumFiles);

	if (iRead != uiNumFiles)
	{
		SafeFree(pasFileDescriptors);
		return false;
	}

	bResult = true;
	for (i = 0; i < uiNumFiles; i++)
	{
		bResult &= DataFilename(szDataFilename, szDataRewriteName, pasFileDescriptors[i].uiDataSize, pasFileDescriptors[i].iFileNum);
		if (bResult)
		{
			pcIndexedFile = mcFiles.Add();
			pcIndexedFile->Init(mpcDurableFileControl, pasFileDescriptors[i].uiFileIndex, szDataFilename, szDataRewriteName, pasFileDescriptors[i].uiDataSize, pasFileDescriptors[i].iFileNum);
		}
	}
	SafeFree(pasFileDescriptors);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::WriteIndexedFileDescriptors(void)
{
	size						i;
	CIndexedFile*				pcIndexedFile;
	SIndexedFileDescriptor*		psFileDescriptor;
	void*						pvFileDescriptors;
	bool						bResult;
	size						iResult;

	pvFileDescriptors = malloc(mcFiles.NumElements() * sizeof(SIndexedFileDescriptor));

	bResult = true;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		psFileDescriptor = (SIndexedFileDescriptor*)RemapSinglePointer(pvFileDescriptors, sizeof(SIndexedFileDescriptor) * i);
		psFileDescriptor->uiDataSize = pcIndexedFile->GetDataSize();
		psFileDescriptor->uiFileIndex = pcIndexedFile->GetFileIndex();
		psFileDescriptor->iFileNum = pcIndexedFile->GetFileNumber();
	}
	iResult = mcFileDescriptors.Write(0, pvFileDescriptors, sizeof(SIndexedFileDescriptor), mcFiles.NumElements());
	bResult &= (iResult != 0);

	SafeFree(pvFileDescriptors);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::DataFilename(char* szFile1, char* szFile2, size uiDataSize, uint32 iFileNum)
{
	CChars		szFilename;
	CChars		szRewriteName;
	CFileUtil	cFileUtil;
	CChars		sz;

	sz.Init();
	sz.Append(uiDataSize);
	sz.Append("_");
	sz.Append(iFileNum);
	sz.Append(".");
	sz.Append(mszDataExtension);

	szFilename.Init();
	cFileUtil.AppendToPath(&szFilename, mszSubDirectory.Text());
	cFileUtil.AppendToPath(&szFilename, sz.Text());

	sz.Insert(0, '_');

	szRewriteName.Init();
	cFileUtil.AppendToPath(&szRewriteName, mszSubDirectory.Text());
	cFileUtil.AppendToPath(&szRewriteName, sz.Text());

	sz.Kill();

	if (szFilename.Length() < MAX_DIRECTORY_LENGTH)
	{
		strcpy(szFile1, szFilename.Text());
		szFilename.Kill();
		strcpy(szFile2, szRewriteName.Text());
		szRewriteName.Kill();
		return true;
	}
	else
	{
		szFilename.Kill();
		szRewriteName.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetOrCreateFile(size uiDataSize)
{
	size			i;
	CIndexedFile*	pcIndexedFile;
	size			iNumFilesWithSize;
	char			szFilename[MAX_DIRECTORY_LENGTH];
	char			szRewriteName[MAX_DIRECTORY_LENGTH];
	bool			bResult;
	size			uiNumFiles;

	iNumFilesWithSize = 0;
	uiNumFiles = mcFiles.NumElements();
	for (i = 0; i < uiNumFiles; i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == uiDataSize)
		{
			if (!pcIndexedFile->IsFull())
			{
				return pcIndexedFile;
			}
			else
			{
				iNumFilesWithSize++;
			}
		}
	}

	pcIndexedFile = mcFiles.Add();

	if (!pcIndexedFile)
	{
		return NULL;
	}

	DataFilename(szFilename, szRewriteName, uiDataSize, iNumFilesWithSize);
	bResult = pcIndexedFile->Init(mpcDurableFileControl, mcFiles.NumElements() - 1, szFilename, szRewriteName, uiDataSize, iNumFilesWithSize);
	if (!bResult)
	{
		return NULL;
	}

	bResult = WriteIndexedFileDescriptors();
	if (!bResult)
	{
		return NULL;
	}

	return pcIndexedFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetFile(size uiDataSize, uint32 iFileNum)
{
	size			i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == uiDataSize)
		{
			if (pcIndexedFile->GetFileNumber() == iFileNum)
			{
				return pcIndexedFile;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetFile(uint32 uiFileIndex)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = mcFiles.Get(uiFileIndex);
	if (!pcIndexedFile)
	{
		return NULL;
	}

	if (!pcIndexedFile->IsFileIndex(uiFileIndex))
	{
		return NULL;
	}

	return pcIndexedFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexedFiles::GetUniqueFileNumber(size uiDataSize)
{
	//This method is used for testing only.

	size			i;
	CIndexedFile*	pcIndexedFile;
	uint32			iFileNumber;

	iFileNumber = INDEXED_FILE_NOT_FOUND;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == uiDataSize)
		{
			if (iFileNumber != INDEXED_FILE_NOT_FOUND)
			{
				return INDEXED_FILE_NOT_FOUND;
			}
			else
			{
				iFileNumber = pcIndexedFile->GetFileNumber();
			}
		}
	}
	return iFileNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::GetFiles(CArrayIndexedFilePtr* papc)
{
	//This method is used for testing only.

	size			i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		papc->Add(&pcIndexedFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIndexedFiles::GetFileDataSize(uint32 uiFileIndex)
{
	CIndexedFile*	pcFile;

	pcFile = GetFile(uiFileIndex);
	if (pcFile)
	{
		return pcFile->GetDataSize();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFiles::NumDatas(void)
{
	size			i;
	CIndexedFile*	pcIndexedFile;
	int64			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		iTotal += pcIndexedFile->NumDatas();
	}
	return iTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFiles::NumDatas(size uiDataSize)
{
	size			i;
	CIndexedFile*	pcIndexedFile;
	int64			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == uiDataSize)
		{
			iTotal += pcIndexedFile->NumDatas();
		}
	}
	return iTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedFiles::NumFiles(size uiDataSize)
{
	size			i;
	CIndexedFile*	pcIndexedFile;
	size			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == uiDataSize)
		{
			iTotal ++;
		}
	}
	return iTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedFiles::NumFiles(void)
{
	return mcFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileDataIndex CIndexedFiles::WriteNew(void* pvData, size uiDataSize)
{
	CIndexedFile*	pcIndexedFile;
	int64			iDataIndex;
	CFileDataIndex	cDataIndex;

	if (uiDataSize != 0)
	{
		pcIndexedFile = GetOrCreateFile(uiDataSize);
		if (!pcIndexedFile)
		{
			cDataIndex.Init();
			return cDataIndex;
		}

		iDataIndex = pcIndexedFile->Write(pvData);
		if ((iDataIndex == INDEXED_FILE_WRITE_ERROR) || (iDataIndex > MAX_UINT))
		{
			cDataIndex.Init();
			return cDataIndex;
		}

		cDataIndex.Init(pcIndexedFile->GetFileIndex(), (uint32)iDataIndex);
		return cDataIndex;
	}
	else
	{
		cDataIndex.Init();
		return cDataIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::WriteExisting(CFileDataIndex* pcDataIndex, void* pvData, size uiDataSize)
{
	CIndexedFile*	pcIndexedFile;
	bool			bResult;

	pcIndexedFile = GetFile(pcDataIndex->miFile);
	if (pcIndexedFile)
	{
		if (uiDataSize != pcIndexedFile->GetDataSize())
		{
			return gcLogger.Error2(__METHOD__, " Cannot write an existing index file data sized [", IntToString(pcIndexedFile->GetDataSize()), "] with a different size [", IntToString(uiDataSize), "].", NULL);
		}
		bResult = pcIndexedFile->Write(pcDataIndex->muiIndex, pvData);
		return bResult;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::Read(CFileDataIndex* pcDataIndex, void* pvData)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = GetFile(pcDataIndex->miFile);
	if (!pcIndexedFile)
	{
		return false;
	}

	return pcIndexedFile->Read(pcDataIndex->muiIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::Delete(CFileDataIndex* pcDataIndex)
{
	CIndexedFile*	pcIndexedFile;
	bool			bResult;

	pcIndexedFile = GetFile(pcDataIndex->miFile);
	if (!pcIndexedFile)
	{
		return false;
	}

	bResult = pcIndexedFile->Delete(pcDataIndex->muiIndex);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedFiles::IsDurable(void)
{
	return mpcDurableFileControl->IsDurable();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::Dump(void)
{
	size			i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		pcIndexedFile->Dump();
	}
}

