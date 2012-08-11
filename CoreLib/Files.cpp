/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/DebugOutput.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ConstructorCall.h"
#include "Files.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFiles::Init(char* szDirectory, char* szPackFilesExtension)
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
	int					i;
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
BOOL CFiles::AddPackFile(CFileNodeSystemFile* pcFileNodeSystemFile)
{
	CPackFileOffset*			pcPackFiles;
	char*						szFullName;
	CChars						szPackFileOffset;
	CChars*						pszFullDirectory;
	int							iLength;
	BOOL						bResult;
	int							iRank;

	pszFullDirectory = mcFileSystem.GetFullDirectoryName();
	iLength = mszPackFilesExtension.Length()+1;

	pcPackFiles = mcPackFilesArray.Add();
	CONSTRUCT(pcPackFiles, CPackFileOffset);

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
BOOL CFiles::AddPackFiles(void)
{
	CArraySystemFilePtrs		aFileNodePtrs;
	int							i;
	CFileNodeSystemFile*		pcFileNodeSystemFile;
	BOOL						bResult;

	mcPackFilesArray.Init();

	aFileNodePtrs.Init(8);
	mcFileSystem.GetFiles(&aFileNodePtrs, mszPackFilesExtension.Text());
	
	for (i = 0; i < aFileNodePtrs.NumElements(); i++)
	{
		pcFileNodeSystemFile = (*aFileNodePtrs.Get(i))->File();
		bResult = AddPackFile(pcFileNodeSystemFile);
		if (!bResult)
		{
			aFileNodePtrs.Kill();
			return FALSE;
		}
	}

	mcPackFilesArray.QuickSort(&ComparePackFileOffset);

	aFileNodePtrs.Kill();
	return TRUE;
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
int CFiles::GetNumPackFiles(void)
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
		szRemain.Kill();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFile* CFiles::GetPackFile(char* szFullName)
{
	CPackFileOffset*	pcPackFiles;
	int					i;
	CChars				sz;
	BOOL				bResult;
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
void CFiles::GetFileNames(CMapStringInt* pcFileNames)
{
	CPackFileOffset*			pcPackFiles;
	int							i;
	CArrayPackFileNodePtrs		acPackFileNodes;
	int							j;
	CFileNodePackFileNode*		pcPackFileNode;
	CChars						szName;
	int*						piCount;
	CArraySystemFilePtrs		acSystemFiles;
	CSystemFileNode*			pcSystemFileNode;
	CFileUtil					cFileUtil;
	char*						szShortName;

	for (i = 0; i < mcPackFilesArray.NumElements(); i++)
	{
		pcPackFiles = mcPackFilesArray.Get(i);
		
		acPackFileNodes.Init(8);

		pcPackFiles->mcPackFiles.FixParents();
		pcPackFiles->mcPackFiles.GetFiles(&acPackFileNodes);

		for (j = 0; j < acPackFileNodes.NumElements(); j++)
		{
			pcPackFileNode = *acPackFileNodes.Get(j);
			szName.Init(pcPackFiles->mszOffset.Text());
			szName.Append('/');
			pcPackFileNode->GetFullName(&szName);

			piCount = pcFileNames->GetWithKey(szName.Text());
			if (!piCount)
			{
				pcFileNames->Put(szName.Text(), 1);
			}
			else
			{
				(*piCount)++;
			}
			szName.Kill();
		}

		acPackFileNodes.Kill();
	}

	acSystemFiles.Init(8);
	mcFileSystem.GetFiles(&acSystemFiles);

	for (j = 0; j < acSystemFiles.NumElements(); j++)
	{
		pcSystemFileNode = *acSystemFiles.Get(j);

		szShortName = pcSystemFileNode->GetName();
		if (!cFileUtil.IsExtension(szShortName, mszPackFilesExtension.Text()))
		{
			szName.Init();
			pcSystemFileNode->GetFullName(&szName);

			piCount = pcFileNames->GetWithKey(szName.Text());
			if (!piCount)
			{
				pcFileNames->Put(szName.Text(), 1);
			}
			else
			{
				(*piCount)++;
			}
			szName.Kill();
		}
	}

	acSystemFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseFileNode* CFiles::StartIteration(CFileIterator* pcIter)
{
	pcIter->Init();
	if (mcPackFilesArray.IsEmpty())
	{
		pcIter->mbFileSystem = TRUE;
	}
	return Iterate(pcIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseFileNode* CFiles::Iterate(CFileIterator* pcIter)
{
	CPackFileOffset*			pcPackFiles;
	CFileUtil					cFileUtil;
	char*						szShortName;
	CFileNodePackFileNode*		pcPackFileNode;
	CSystemFileNode*			pcSystemFileNode;

	if (!pcIter->mbFileSystem)
	{
		if (pcIter->mbMoveOn)
		{
			pcIter->mbMoveOn = FALSE;
			pcIter->i++;
			if (pcIter->i < mcPackFilesArray.NumElements())
			{
				pcPackFiles = mcPackFilesArray.Get(pcIter->i);
				pcPackFiles->mcPackFiles.FixParents();
				pcPackFileNode = pcPackFiles->mcPackFiles.StartIteration(&pcIter->cPackFileIterator);
				if (pcPackFileNode)
				{
					return pcPackFileNode;
				}
				else
				{
					pcPackFiles->mcPackFiles.StopIteration(&pcIter->cPackFileIterator);
					pcIter->mbMoveOn = TRUE;
					return Iterate(pcIter);
				}
			}
			else
			{
				pcIter->mbFileSystem = TRUE;
				pcIter->mbMoveOn = TRUE;
				return Iterate(pcIter);
			}
		}
		else
		{
			pcPackFiles = mcPackFilesArray.Get(pcIter->i);
			pcPackFileNode = pcPackFiles->mcPackFiles.Iterate(&pcIter->cPackFileIterator);
			if (pcPackFileNode)
			{
				return pcPackFileNode;
			}
			else
			{
				pcPackFiles->mcPackFiles.StopIteration(&pcIter->cPackFileIterator);
				pcIter->mbMoveOn = TRUE;
				return Iterate(pcIter);
			}

		}
	}
	else
	{
		if (pcIter->mbMoveOn)
		{
			pcIter->mbMoveOn = FALSE;
			pcSystemFileNode = mcFileSystem.StartIteration(&pcIter->cFileSystemIterator);
			if (pcSystemFileNode)
			{
				szShortName = pcSystemFileNode->GetName();
				if (!cFileUtil.IsExtension(szShortName, mszPackFilesExtension.Text()))
				{
					return pcSystemFileNode;
				}
				else
				{
					return Iterate(pcIter);
				}
			}
			else
			{
				mcFileSystem.StopIteration(&pcIter->cFileSystemIterator);
				return NULL;
			}
		}
		else
		{
			pcSystemFileNode = mcFileSystem.Iterate(&pcIter->cFileSystemIterator);
			if (pcSystemFileNode)
			{
				szShortName = pcSystemFileNode->GetName();
				if (!cFileUtil.IsExtension(szShortName, mszPackFilesExtension.Text()))
				{
					return pcSystemFileNode;
				}
				else
				{
					return Iterate(pcIter);
				}
			}
			else
			{
				mcFileSystem.StopIteration(&pcIter->cFileSystemIterator);
				return NULL;
			}
		}
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

