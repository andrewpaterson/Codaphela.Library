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
#include "IndexedFiles.h"
#include "BaseLib/PointerRemapper.h"


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
void CIndexedFiles::Init(CDurableFileController* pcDurableFileControl, char* szExtension)
{
	mpcDurableFileControl = pcDurableFileControl;
	mszExtension.Init(szExtension);

	InitIndexedFileDescriptors();
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

	mszExtension.Kill();
	mszIndexName.Kill();
	mszIndexRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::Open(void)
{
	BOOL	bResult;

	bResult = mcFileDescriptors.Open();
	if (!bResult)
	{
		return FALSE;
	}
	return ReadIndexedFileDescriptors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::Close(void)
{
	int					i;
	CIndexedFile*		pcIndexedFile;
	BOOL				bResult;

	bResult = TRUE;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFile = mcFiles.Get(i);
		bResult &= pcIndexedFile->Close();
	}

	bResult &= mcFileDescriptors.Close();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::InitIndexedFileDescriptors(void)
{
	mszIndexName.Init(mpcDurableFileControl->GetWorkingDirectory());
	mszIndexName.Append(FILE_SEPARATOR);
	mszIndexName.Append("Files.");
	mszIndexName.Append(mszExtension);
	mszIndexRewrite.Init(mpcDurableFileControl->GetWorkingDirectory());
	mszIndexRewrite.Append(FILE_SEPARATOR);
	mszIndexRewrite.Append("_Files.");
	mszIndexRewrite.Append(mszExtension);

	mcFileDescriptors.Init(mpcDurableFileControl->IsDurable(), mszIndexName.Text(), mszIndexRewrite.Text());
	mpcDurableFileControl->AddFile(&mcFileDescriptors);
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
	char						szDataFileName[65536];
	char						szDataRewriteName[65536];
	filePos						iRead;

	mcFiles.Init(1024);

	iFileSize = mcFileDescriptors.Size();
	if (iFileSize == 0)
	{
		return TRUE;
	}

	iNumFiles = iFileSize / (sizeof(SIndexedFileDescriptor));
	pasFileDescriptors = (SIndexedFileDescriptor*)malloc((int)iFileSize);
	iRead = mcFileDescriptors.ReadFromFile(pasFileDescriptors, sizeof(SIndexedFileDescriptor), iNumFiles);
	if (iRead != iNumFiles)
	{
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
			pcIndexedFile->Open(mpcDurableFileControl);
		}
	}
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
	bResult &= mcFileDescriptors.Write(0, pvFileDescriptors, sizeof(SIndexedFileDescriptor), mcFiles.NumElements());

	free(pvFileDescriptors);
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

	szFileName.Init(mpcDurableFileControl->GetWorkingDirectory());
	szFileName.Append(FILE_SEPARATOR);
	szFileName.Append(iDataSize);
	szFileName.Append("_");
	szFileName.Append(iFileNum);
	szFileName.Append(".");
	szFileName.Append(mszExtension);

	szRewriteName.Init(mpcDurableFileControl->GetWorkingDirectory());
	szRewriteName.Append(FILE_SEPARATOR);
	szRewriteName.Append("_");
	szRewriteName.Append(iDataSize);
	szRewriteName.Append("_");
	szRewriteName.Append(iFileNum);
	szRewriteName.Append(".");
	szRewriteName.Append(mszExtension);

	if (szFileName.Length() < 65536)
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
	int				iNumFiles;
	char			szFileName[65536];
	char			szRewriteName[65536];

	iNumFiles = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
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
				iNumFiles++;
			}
		}
	}

	pcIndexedFile = mcFiles.Add();
	if (!pcIndexedFile)
	{
		return NULL;
	}

	DataFileName(szFileName, szRewriteName, iDataSize, iNumFiles);
	pcIndexedFile->Init(mpcDurableFileControl, mcFiles.NumElements()-1, szFileName, szRewriteName, iDataSize, iNumFiles);
	pcIndexedFile->Open(mpcDurableFileControl);

	WriteIndexedFileDescriptors();
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

	pcIndexedFile = mcFiles.SafeGet(iFileIndex);
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
OIndex CIndexedFiles::NumInFile(int iDataSize)
{
	int				i;
	CIndexedFile*	pcIndexedFile;
	OIndex			iTotal;

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
int CIndexedFiles::NumFiles(void)
{
	return mcFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::IsDurable(void)
{
	return mcFileDescriptors.mbDurable;
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

