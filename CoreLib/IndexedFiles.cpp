/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FilePathBuilder.h"
#include "IndexedFiles.h"


struct SIndexedFileDescriptor
{
	int	iFileIndex;
	int	iDataSize;
	int	iFileNum;
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
BOOL CIndexedFiles::Kill(void)
{
	int				i; 
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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::ReadIndexedFileDescriptors(void)
{
	filePos						iFileSize;
	BOOL						bResult;
	int							i;
	filePos						iNumFiles;
	SIndexedFileDescriptor*		pasFileDescriptors;
	CIndexedFile*				pcIndexedFile;
	char						szDataFilename[MAX_DIRECTORY_LENGTH];
	char						szDataRewriteName[MAX_DIRECTORY_LENGTH];
	filePos						iRead;
	int							iRemainder;

	if (!mpcDurableFileControl->IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read descriptors if the Durable Controller is not begun.", NULL);
		return FALSE;
	}

	if (mcFiles.NumElements() != 0)
	{
		//Don't read.  Should probably throw an error.
		return FALSE;
	}

	iFileSize = mcFileDescriptors.Size();
	if (iFileSize == 0)
	{
		bResult = mcFileDescriptors.Create();
		return bResult;
	}

	iNumFiles = iFileSize / (sizeof(SIndexedFileDescriptor));
	iRemainder = iFileSize % (sizeof(SIndexedFileDescriptor));
	if (iRemainder != 0)
	{
		return FALSE;
	}

	pasFileDescriptors = (SIndexedFileDescriptor*)malloc((int)iFileSize);

	iRead = mcFileDescriptors.Read(pasFileDescriptors, sizeof(SIndexedFileDescriptor), iNumFiles);

	if (iRead != iNumFiles)
	{
		SafeFree(pasFileDescriptors);
		return FALSE;
	}

	bResult = TRUE;
	for (i = 0; i < iNumFiles; i++)
	{
		bResult &= DataFilename(szDataFilename, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		if (bResult)
		{
			pcIndexedFile = mcFiles.Add();
			pcIndexedFile->Init(mpcDurableFileControl, pasFileDescriptors[i].iFileIndex, szDataFilename, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		}
	}
	SafeFree(pasFileDescriptors);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::WriteIndexedFileDescriptors(void)
{
	int							i;
	CIndexedFile*				pcIndexedFile;
	SIndexedFileDescriptor*		psFileDescriptor;
	void*						pvFileDescriptors;
	BOOL						bResult;
	filePos						iResult;

	pvFileDescriptors = malloc(mcFiles.NumElements() * sizeof(SIndexedFileDescriptor));

	bResult = TRUE;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		psFileDescriptor = (SIndexedFileDescriptor*)RemapSinglePointer(pvFileDescriptors, sizeof(SIndexedFileDescriptor) * i);
		psFileDescriptor->iDataSize = pcIndexedFile->GetDataSize();
		psFileDescriptor->iFileIndex = pcIndexedFile->GetFileIndex();
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
BOOL CIndexedFiles::DataFilename(char* szFile1, char* szFile2, int iDataSize, int iFileNum)
{
	CChars		szFilename;
	CChars		szRewriteName;
	CFileUtil	cFileUtil;
	CChars		sz;

	sz.Init();
	sz.Append(iDataSize);
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
		return TRUE;
	}
	else
	{
		szFilename.Kill();
		szRewriteName.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetOrCreateFile(unsigned int uiDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int				iNumFilesWithSize;
	char			szFilename[MAX_DIRECTORY_LENGTH];
	char			szRewriteName[MAX_DIRECTORY_LENGTH];
	BOOL			bResult;
	int				iNumFiles;

	iNumFilesWithSize = 0;
	iNumFiles = mcFiles.NumElements();
	for (i = 0; i < iNumFiles; i++)
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
	bResult = pcIndexedFile->Init(mpcDurableFileControl, mcFiles.NumElements()-1, szFilename, szRewriteName, uiDataSize, iNumFilesWithSize);
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
CIndexedFile* CIndexedFiles::GetFile(unsigned int uiDataSize, int iFileNum)
{
	int				i;
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
CIndexedFile* CIndexedFiles::GetFile(int iFileIndex)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = mcFiles.Get(iFileIndex);
	if (!pcIndexedFile)
	{
		return NULL;
	}

	if (!pcIndexedFile->IsFileIndex(iFileIndex))
	{
		return NULL;
	}

	return pcIndexedFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedFiles::GetUniqueFileNumber(int iDataSize)
{
	//This method is used for testing only.

	int				i;
	CIndexedFile*	pcIndexedFile;
	int				iFileNumber;

	iFileNumber = -1;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == iDataSize)
		{
			if (iFileNumber != -1)
			{
				return -1;
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

	int				i;
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
unsigned int CIndexedFiles::GetFileDataSize(int iFileIndex)
{
	CIndexedFile*	pcFile;

	pcFile = GetFile(iFileIndex);
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
	int				i;
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
int64 CIndexedFiles::NumDatas(int iDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int64			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == iDataSize)
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
int CIndexedFiles::NumFiles(int iDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->GetDataSize() == iDataSize)
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
int CIndexedFiles::NumFiles(void)
{
	return mcFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileDataIndex CIndexedFiles::WriteNew(void* pvData, unsigned int uiDataSize)
{
	CIndexedFile*	pcIndexedFile;
	unsigned int	uiDataIndex;
	CFileDataIndex	cDataIndex;

	if (uiDataSize != 0)
	{
		pcIndexedFile = GetOrCreateFile(uiDataSize);
		if (!pcIndexedFile)
		{
			cDataIndex.Init();
			return cDataIndex;
		}

		uiDataIndex = pcIndexedFile->Write(pvData);
		if (uiDataIndex == INDEXED_FILE_WRITE_ERROR)
		{
			cDataIndex.Init();
			return cDataIndex;
		}

		cDataIndex.Init(pcIndexedFile->GetFileIndex(), uiDataIndex);
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
BOOL CIndexedFiles::WriteExisting(CFileDataIndex* pcDataIndex, void* pvData, unsigned int uiDataSize)
{
	CIndexedFile*	pcIndexedFile;
	BOOL			bResult;

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
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::Read(CFileDataIndex* pcDataIndex, void* pvData)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = GetFile(pcDataIndex->miFile);
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	return pcIndexedFile->Read(pcDataIndex->muiIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::Delete(CFileDataIndex* pcDataIndex)
{
	CIndexedFile*	pcIndexedFile;
	int				iResult;

	pcIndexedFile = GetFile(pcDataIndex->miFile);
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	iResult = pcIndexedFile->Delete(pcDataIndex->muiIndex);
	return iResult == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::IsDurable(void)
{
	return mpcDurableFileControl->IsDurable();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::Dump(void)
{
	int				i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		pcIndexedFile->Dump();
	}
}

