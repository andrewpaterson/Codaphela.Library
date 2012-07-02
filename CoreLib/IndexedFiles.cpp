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
	ReadIndexedFileDescriptors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::Kill(void)
{
	int							i;
	CIndexedFile*		pcFileDescriptor;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFileDescriptor = mcFiles.Get(i);
		pcFileDescriptor->Kill();
	}
	mcFiles.Kill();

	mcFileDescriptorsFile.Close();

	mszExtension.Kill();
	mszIndexName.Kill();
	mszIndexRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::InitIndexedFileDescriptors(void)
{
	BOOL	bResult;

	mszIndexName.Init(mpcDurableFileControl->mszWorkingDirectory);
	mszIndexName.Append(FILE_SEPARATOR);
	mszIndexName.Append("Files.");
	mszIndexName.Append(mszExtension);
	mszIndexRewrite.Init(mpcDurableFileControl->mszWorkingDirectory);
	mszIndexRewrite.Append(FILE_SEPARATOR);
	mszIndexRewrite.Append("_Files.");
	mszIndexRewrite.Append(mszExtension);

	mcFileDescriptorsFile.Init(mpcDurableFileControl->mbDurable, mszIndexName.Text(), mszIndexRewrite.Text());
	mpcDurableFileControl->AddFile(&mcFileDescriptorsFile);
	bResult = mcFileDescriptorsFile.Open();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::ReadIndexedFileDescriptors(void)
{
	filePos						iFileSize;
	BOOL						bResult;
	int							i;
	filePos						iNumFiles;
	SIndexedFileDescriptor*		pasFileDescriptors;
	CIndexedFile*				pcFileDescriptor;
	char						szDataFileName[65536];
	char						szDataRewriteName[65536];

	mcFiles.Init(1024);

	iFileSize = mcFileDescriptorsFile.Size();

	if (iFileSize == 0)
	{
		return;
	}

	iNumFiles = iFileSize / (sizeof(SIndexedFileDescriptor));
	pasFileDescriptors = (SIndexedFileDescriptor*)malloc((int)(iFileSize * (sizeof(SIndexedFileDescriptor))));
	mcFileDescriptorsFile.ReadFromFile(pasFileDescriptors, sizeof(SIndexedFileDescriptor), (int)iNumFiles);

	for (i = 0; i < iNumFiles; i++)
	{
		bResult = DataFileName(szDataFileName, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		if (bResult)
		{
			pcFileDescriptor = mcFiles.Add();
			pcFileDescriptor->Init(mpcDurableFileControl, pasFileDescriptors[i].iFileIndex, szDataFileName, szDataRewriteName, pasFileDescriptors[i].iDataSize, pasFileDescriptors[i].iFileNum);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedFiles::WriteIndexedFileDescriptors(void)
{
	int							i;
	CIndexedFile*				pcFileDescriptor;
	char						szDataFileName[65536];
	char						szDataRewriteName[65536];
	SIndexedFileDescriptor*		psFileDescriptor;
	void*						pvFileDescriptors;
	BOOL						bResult;

	pvFileDescriptors = malloc(mcFiles.NumElements() * sizeof(SIndexedFileDescriptor));

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFileDescriptor = mcFiles.Get(i);
		psFileDescriptor =  (SIndexedFileDescriptor*)RemapSinglePointer(pvFileDescriptors, sizeof(SIndexedFileDescriptor) * i);
		psFileDescriptor->iDataSize = pcFileDescriptor->miDataSize;
		psFileDescriptor->iFileIndex = pcFileDescriptor->miFileIndex;
		psFileDescriptor->iFileNum = pcFileDescriptor->miFileNumber;

		bResult = DataFileName(szDataFileName, szDataRewriteName, pcFileDescriptor->miDataSize, pcFileDescriptor->miFileNumber);
	}
	mcFileDescriptorsFile.Write(0, pvFileDescriptors, sizeof(SIndexedFileDescriptor), mcFiles.NumElements());

	free(pvFileDescriptors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedFiles::DataFileName(char* szFile1, char* szFile2, int iDataSize, int iFileNum)
{
	CChars	szFileName;
	CChars	szRewriteName;

	szFileName.Init(mpcDurableFileControl->mszWorkingDirectory);
	szFileName.Append(FILE_SEPARATOR);
	szFileName.Append(iDataSize);
	szFileName.Append("_");
	szFileName.Append(iFileNum);
	szFileName.Append(".");
	szFileName.Append(mszExtension);

	szRewriteName.Init(mpcDurableFileControl->mszWorkingDirectory);
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
CIndexedFile* CIndexedFiles::GetOrCreateFile(int iDataSize)
{
	int				i;
	CIndexedFile*	pcFile;
	int				iNumFiles;
	char			szFileName[65536];
	char			szRewriteName[65536];

	iNumFiles = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		if (pcFile->miDataSize == iDataSize)
		{
			if (!pcFile->IsFull())
			{
				return pcFile;
			}
			else
			{
				iNumFiles++;
			}
		}
	}

	pcFile = mcFiles.Add();
	DataFileName(szFileName, szRewriteName, iDataSize, iNumFiles);
	pcFile->Init(mpcDurableFileControl, mcFiles.NumElements()-1, szFileName, szRewriteName, iDataSize, iNumFiles);
	return pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedFile* CIndexedFiles::GetFile(int iDataSize, int iFileNum)
{
	int							i;
	CIndexedFile*		pcFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		if (pcFile->miDataSize == iDataSize)
		{
			if (pcFile->miFileNumber == iFileNum)
			{
				return pcFile;
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
	CIndexedFile*		pcFile;

	pcFile = mcFiles.SafeGet(iFileIndex);
	if (!pcFile)
	{
		return NULL;
	}
	if (pcFile->miFileIndex != iFileIndex)
	{
		return NULL;
	}
	return pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedFiles::NumInFile(int iDataSize)
{
	int							i;
	CIndexedFile*		pcFile;
	OIndex						iTotal;

	iTotal = 0LL;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		if (pcFile->miDataSize == iDataSize)
		{
			iTotal += pcFile->miNumDatas;
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
void CIndexedFiles::Dump(void)
{
	int							i;
	CIndexedFile*		pcIndexedFileDescriptor;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcIndexedFileDescriptor = mcFiles.Get(i);
		pcIndexedFileDescriptor->Dump();
	}
}

