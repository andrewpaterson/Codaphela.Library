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
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/GlobalMemory.h"
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
void CIndexedFiles::Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite)
{
	mpcDurableFileControl = pcDurableFileControl;
	mszDataExtension.Init(szDataExtension);
	mcFiles.Init(1024);

	InitIndexedFileDescriptors(szDescricptorName, szDescricptorRewrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::Kill(void)
{
	int				i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		pcIndexedFile->Kill();
	}
	mcFiles.Kill();

	mcFileDescriptors.Kill();
	mszDataExtension.Kill();
	mszDescricptorName.Kill();
	mszDescricptorRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::InitIndexedFileDescriptors(char* szDescricptorName, char* szDescricptorRewrite)
{
	mbDescriptorsRead = FALSE;

	mszDescricptorName.Init(mpcDurableFileControl->GetDirectory());
	mszDescricptorName.Append(FILE_SEPARATOR);
	mszDescricptorName.Append(szDescricptorName);

	if (mpcDurableFileControl->IsDurable())
	{
		mszDescricptorRewrite.Init(mpcDurableFileControl->GetRewriteDirectory());
		mszDescricptorRewrite.Append(FILE_SEPARATOR);
		mszDescricptorRewrite.Append(szDescricptorRewrite);
	}
	else
	{
		mszDescricptorRewrite.Init();
	}

	mcFileDescriptors.Init(mpcDurableFileControl, mszDescricptorName.Text(), mszDescricptorRewrite.Text());
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
	char						szDataFileName[MAX_DIRECTORY_LENGTH];
	char						szDataRewriteName[MAX_DIRECTORY_LENGTH];
	filePos						iRead;

	if (!mpcDurableFileControl->IsBegun())
	{
		return FALSE;
	}

	if (mcFiles.NumElements() != 0)
	{
		//Don't read 
		return FALSE;
	}

	iFileSize = mcFileDescriptors.Size();
	if (iFileSize == 0)
	{
		return mcFileDescriptors.Create();
	}

	iNumFiles = iFileSize / (sizeof(SIndexedFileDescriptor));
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
		bResult &= DataFileName(szDataFileName, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		if (bResult)
		{
			pcIndexedFile = mcFiles.Add();
			pcIndexedFile->Init(mpcDurableFileControl, pasFileDescriptors[i].iFileIndex, szDataFileName, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		}
	}
	SafeFree(pasFileDescriptors);
	mbDescriptorsRead = TRUE;
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
		psFileDescriptor->iDataSize = pcIndexedFile->miDataSize;
		psFileDescriptor->iFileIndex = pcIndexedFile->GetFileIndex();
		psFileDescriptor->iFileNum = pcIndexedFile->miFileNumber;
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
BOOL CIndexedFiles::DataFileName(char* szFile1, char* szFile2, int iDataSize, int iFileNum)
{
	CChars	szFileName;
	CChars	szRewriteName;

	szFileName.Init(mpcDurableFileControl->GetDirectory());
	szFileName.Append(FILE_SEPARATOR);
	szFileName.Append(iDataSize);
	szFileName.Append("_");
	szFileName.Append(iFileNum);
	szFileName.Append(".");
	szFileName.Append(mszDataExtension);

	szRewriteName.Init(mpcDurableFileControl->GetRewriteDirectory());
	szRewriteName.Append(FILE_SEPARATOR);
	szRewriteName.Append("_");
	szRewriteName.Append(iDataSize);
	szRewriteName.Append("_");
	szRewriteName.Append(iFileNum);
	szRewriteName.Append(".");
	szRewriteName.Append(mszDataExtension);

	if (szFileName.Length() < MAX_DIRECTORY_LENGTH)
	{
		strcpy(szFile1, szFileName.Text());
		szFileName.Kill();
		strcpy(szFile2, szRewriteName.Text());
		szRewriteName.Kill();
		return TRUE;
	}
	else
	{
		szFileName.Kill();
		szRewriteName.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::RemoveFiles(void)
{
	int							i;
	CIndexedFile*				pcIndexedFile;
	BOOL						bResult;

	bResult = TRUE;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		bResult &= pcIndexedFile->mcFile.Delete();
	}
	bResult &= mcFileDescriptors.Delete();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetOrCreateFile(int iDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int				iNumFilesWithSize;
	char			szFileName[MAX_DIRECTORY_LENGTH];
	char			szRewriteName[MAX_DIRECTORY_LENGTH];
	BOOL			bResult;
	int				iNumFiles;

	iNumFilesWithSize = 0;
	iNumFiles = mcFiles.NumElements();
	for (i = 0; i < iNumFiles; i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->miDataSize == iDataSize)
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

	DataFileName(szFileName, szRewriteName, iDataSize, iNumFilesWithSize);
	bResult = pcIndexedFile->Init(mpcDurableFileControl, mcFiles.NumElements()-1, szFileName, szRewriteName, iDataSize, iNumFilesWithSize);
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
CIndexedFile* CIndexedFiles::GetFile(int iDataSize, int iFileNum)
{
	int				i;
	CIndexedFile*	pcIndexedFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->miDataSize == iDataSize)
		{
			if (pcIndexedFile->miFileNumber == iFileNum)
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
		if (pcIndexedFile->miDataSize == iDataSize)
		{
			if (iFileNumber != -1)
			{
				return -1;
			}
			else
			{
				iFileNumber = pcIndexedFile->miFileNumber;
			}
		}
	}
	return iFileNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFiles::NumData(void)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int64			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		iTotal += pcIndexedFile->miNumDatas;
	}
	return iTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedFiles::NumData(int iDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	int64			iTotal;

	iTotal = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		if (pcIndexedFile->miDataSize == iDataSize)
		{
			iTotal += pcIndexedFile->miNumDatas;
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
		if (pcIndexedFile->miDataSize == iDataSize)
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
BOOL CIndexedFiles::Write(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	if (pcDescriptor->HasFile())
	{
		return WriteExisting(pcDescriptor, pvData);
	}
	else
	{
		return WriteNew(pcDescriptor, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::WriteNew(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iFilePos;
	int				iDataSize;

	iDataSize = pcIndexDescriptor->GetDataSize();
	if (iDataSize != 0)
	{
		pcIndexedFile = GetOrCreateFile(iDataSize);
		if (!pcIndexedFile)
		{
			return FALSE;
		}

		iFilePos = pcIndexedFile->Write(pvData);
		if (iFilePos == -1)
		{
			return FALSE;
		}

		pcIndexDescriptor->SetIndexes(pcIndexedFile->GetFileIndex(), iFilePos);
		return TRUE;
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
BOOL CIndexedFiles::WriteExisting(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;
	filePos			iResult;

	pcIndexedFile = GetFile(pcIndexDescriptor->GetFileIndex());
	if (pcIndexedFile)
	{
		iResult = pcIndexedFile->Write(pcIndexDescriptor->GetIndexInFile(), pvData);
		return iResult == 1;
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
BOOL CIndexedFiles::Read(CIndexedDataDescriptor* pcIndexDescriptor, void* pvData)
{
	CIndexedFile*	pcIndexedFile;

	pcIndexedFile = GetFile(pcIndexDescriptor->GetFileIndex());
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	return pcIndexedFile->Read(pcIndexDescriptor->GetIndexInFile(), pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::Delete(CFileIndex* pcFileIndex)
{
	CIndexedFile*	pcIndexedFile;
	int				iResult;

	pcIndexedFile = GetFile(pcFileIndex->miFile);
	if (!pcIndexedFile)
	{
		return FALSE;
	}

	iResult = pcIndexedFile->Delete(pcFileIndex->mulliFilePos);
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

