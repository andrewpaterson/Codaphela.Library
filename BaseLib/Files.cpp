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
#include "DebugOutput.h"
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "Files.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFiles::Init(char* szDirectory, char* szPackFilesExtension)
{
	CChars*						pszFullDirectory;

	mszPackFilesExtension.Init(szPackFilesExtension);
	mcFileSystem.Init(szDirectory);
	pszFullDirectory = mcFileSystem.GetFullDirectoryName();

	return AddPackFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFiles::Kill(void)
{
	uint32				i;
	CPackFileOffset*	pcPackFiles;
	
	mszPackFilesExtension.Kill();

	for (i = 0; i < mcPackFilesArray.NumElements(); i++)
	{
		pcPackFiles = mcPackFilesArray.Get(i);
		pcPackFiles->Kill();
	}

	mcPackFilesArray.Kill();
	mcFileSystem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFiles::AddPackFile(CFileNodeSystemFile* pcFileNodeSystemFile)
{
	CPackFileOffset*			pcPackFiles;
	char*						szFullName;
	CChars						szPackFileOffset;
	CChars*						pszFullDirectory;
	size						iLength;
	bool						bResult;
	uint32						iRank;

	pszFullDirectory = mcFileSystem.GetFullDirectoryName();
	iLength = mszPackFilesExtension.Length()+1;

	pcPackFiles = mcPackFilesArray.Add();
	new(pcPackFiles) CPackFileOffset();

	szFullName = pcFileNodeSystemFile->GetFullName();

	szPackFileOffset.Init(szFullName);
	szPackFileOffset.RemoveFromStart(pszFullDirectory->Length()+1);
	szPackFileOffset.RemoveFromEnd(iLength);
	szPackFileOffset.Replace('\\', '/');

	iRank = szPackFileOffset.Count('/');

	bResult = pcPackFiles->Init(szPackFileOffset.Text(), szFullName, iRank);
	szPackFileOffset.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFiles::AddPackFiles(void)
{
	CArraySystemFilePtrs		aFileNodePtrs;
	uint32						i;
	CFileNodeSystemFile*		pcFileNodeSystemFile;
	bool						bResult;

	mcPackFilesArray.Init();

	aFileNodePtrs.Init();
	mcFileSystem.GetFiles(&aFileNodePtrs, mszPackFilesExtension.Text());
	
	for (i = 0; i < aFileNodePtrs.NumElements(); i++)
	{
		pcFileNodeSystemFile = (*aFileNodePtrs.Get(i))->File();
		bResult = AddPackFile(pcFileNodeSystemFile);
		if (!bResult)
		{
			aFileNodePtrs.Kill();
			return false;
		}
	}

	mcPackFilesArray.QuickSort(&ComparePackFileOffset);

	aFileNodePtrs.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int ComparePackFileOffset(const void* arg1, const void* arg2)
{
	CPackFileOffset*	pcOffset1;
	CPackFileOffset*	pcOffset2;
	int					iResult;

	pcOffset1 = (CPackFileOffset*)arg1;
	pcOffset2 = (CPackFileOffset*)arg2;

	iResult = pcOffset2->mszOffset.Length() - pcOffset1->mszOffset.Length();
	if (iResult != 0)
	{
		return iResult;
	}
	else
	{
		return pcOffset1->mszOffset.CompareIgnoreCase(pcOffset2->mszOffset.Text());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CFiles::GetNumPackFiles(void)
{
	return mcPackFilesArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAbstractFile* CFiles::GetFile(char* szFullName)
{
	CDiskFile*	pcDiskFile;
	CPackFile*	pcPackFile;

	pcDiskFile = GetSystemFile(szFullName);
	if (pcDiskFile)
	{
		return pcDiskFile;
	}

	pcPackFile = GetPackFile(szFullName);
	if (pcPackFile)
	{
		return pcPackFile;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDiskFile* CFiles::GetSystemFile(char* szFullName)
{
	CChars					szRemain;
	CFileNodeSystemFile*	pcNode;
	char*					szName;
	CDiskFile*				pcDiskFile;

	szRemain.Init();
	pcNode = mcFileSystem.GetFileNode(szFullName, &szRemain);
	if (pcNode)
	{
		if (szRemain.Length() == 0)
		{
			szName = pcNode->GetFullName();
			pcDiskFile = DiskFile(szName);
			szRemain.Kill();
			return pcDiskFile;
		}
	}
	szRemain.Kill();
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFile* CFiles::GetPackFile(char* szFullName)
{
	CPackFileOffset*	pcPackFiles;
	size				i;
	CChars				sz;
	bool				bResult;
	CPackFile*			pcPackFile;

	sz.Fake(szFullName);
	for (i = 0; i < mcPackFilesArray.NumElements(); i++)
	{
		pcPackFiles = mcPackFilesArray.Get(i);
		bResult = sz.StartsWith(pcPackFiles->mszOffset.Text());
		if (bResult)
		{
			pcPackFile = GetPackFile(pcPackFiles, szFullName);
			if (pcPackFile)
			{
				return pcPackFile;
			}
		}
	}
	
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFile* CFiles::GetPackFile(CPackFileOffset*	pcPackFiles, char* szFullName)
{
	CFileNodePackFileNode*	pcNode;
	CPackFile*				pcPackFile;
	CChars					sz;

	sz.Init(szFullName, pcPackFiles->mszOffset.Length()+1);
	pcNode = pcPackFiles->mcPackFiles.GetNode(sz.Text());
	sz.Kill();
	if (pcNode)
	{
		pcPackFile = PackFile(&pcPackFiles->mcPackFiles, pcNode->File());
		return pcPackFile;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFiles::GetFilenames(CMapStringInt* pcFilenames)
{
	CFileIterator			cIter;
	CFileIteratorReturn*	pcReturn;
	uint32*					piValue;
	uint32					iRank;

	pcReturn = StartIteration(&cIter);
	while (pcReturn)
	{
		piValue = (uint32*)pcFilenames->CMapStringTemplate::Get(pcReturn->GetFullName());
		if (!piValue)
		{
			iRank = pcReturn->GetFileRank() << 16;
			pcFilenames->Put(pcReturn->GetFullName(), iRank + 1);
		}
		else
		{
			iRank = (*piValue) >> 16;
			if (iRank < pcReturn->GetFileRank())
			{
				iRank = pcReturn->GetFileRank() << 16;
				*piValue = iRank | (*piValue & 0xFFFF);
			}
			(*piValue)++;
		}

		pcReturn = Iterate(&cIter);
	}
	StopIteration(&cIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::StartIteration(CFileIterator* pcIter)
{
	pcIter->Init();
	if (mcPackFilesArray.IsEmpty())
	{
		pcIter->mbFileSystem = true;
	}
	return Iterate(pcIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::Iterate(CFileIterator* pcIter)
{
	if (!pcIter->mbFileSystem)
	{
		return IterateInPackFiles(pcIter);
	}
	else
	{
		return IterateOnFileSystem(pcIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::IterateInPackFiles(CFileIterator* pcIter, CFileNodePackFileNode* pcPackFileNode, CPackFileOffset* pcPackFiles)
{
	if (pcPackFileNode)
	{
		return pcIter->SetCurrent(FIRT_PackFiles, pcPackFileNode, pcPackFiles->miFileRank, pcPackFiles->mszOffset.Text());
	}
	else
	{
		pcPackFiles->mcPackFiles.StopIteration(&pcIter->mcPackFileIterator);
		pcIter->mbMoveOn = true;
		return IterateInPackFiles(pcIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::IterateInPackFiles(CFileIterator* pcIter)
{
	CPackFileOffset*			pcPackFiles;
	CFileNodePackFileNode*		pcPackFileNode;

	if (pcIter->mbMoveOn)
	{
		pcIter->mbMoveOn = false;
		pcIter->miPackFileIndex++;
		if (pcIter->miPackFileIndex < mcPackFilesArray.NumElements())
		{
			pcPackFiles = mcPackFilesArray.Get(pcIter->miPackFileIndex);
			pcPackFiles->mcPackFiles.FixParents();
			pcPackFileNode = pcPackFiles->mcPackFiles.StartIteration(&pcIter->mcPackFileIterator);
			return IterateInPackFiles(pcIter, pcPackFileNode, pcPackFiles);
		}
		else
		{
			pcIter->mbFileSystem = true;
			pcIter->mbMoveOn = true;
			return IterateOnFileSystem(pcIter);
		}
	}
	else
	{
		pcPackFiles = mcPackFilesArray.Get(pcIter->miPackFileIndex);
		pcPackFileNode = pcPackFiles->mcPackFiles.Iterate(&pcIter->mcPackFileIterator);
		return IterateInPackFiles(pcIter, pcPackFileNode, pcPackFiles);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::IterateFileSystemNode(CFileIterator* pcIter, CSystemFileNode* pcSystemFileNode)
{
	CFileUtil					cFileUtil;
	char*						szShortName;

	if (pcSystemFileNode)
	{
		szShortName = pcSystemFileNode->GetName();
		if (!cFileUtil.IsExtension(szShortName, mszPackFilesExtension.Text()))
		{
			return pcIter->SetCurrent(FIRT_FileSystem, pcSystemFileNode, FILE_SYSTEM_RANK, NULL);
		}
		else
		{
			return IterateOnFileSystem(pcIter);
		}
	}
	else
	{
		mcFileSystem.StopIteration(&pcIter->mcFileSystemIterator);
		return pcIter->SetCurrent();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileIteratorReturn* CFiles::IterateOnFileSystem(CFileIterator* pcIter)
{
	CSystemFileNode*			pcSystemFileNode;


	if (pcIter->mbMoveOn)
	{
		pcIter->mbMoveOn = false;
		pcSystemFileNode = mcFileSystem.StartIteration(&pcIter->mcFileSystemIterator);
		return IterateFileSystemNode(pcIter, pcSystemFileNode);
	}
	else
	{
		pcSystemFileNode = mcFileSystem.Iterate(&pcIter->mcFileSystemIterator);
		return IterateFileSystemNode(pcIter, pcSystemFileNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFiles::StopIteration(CFileIterator* pcIter)
{
	pcIter->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFiles* CFiles::GetPackFiles(size iIndex)
{
	CPackFileOffset*	pcOffset;

	pcOffset = mcPackFilesArray.Get(iIndex);
	return &pcOffset->mcPackFiles;
}

