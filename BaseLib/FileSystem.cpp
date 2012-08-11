/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "FileUtil.h"
#include "FileSystem.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::Init(char* szDirectory)
{
	CFileUtil	cFileUtil;

	mcNames.Init();
	mszDirectory.Init(szDirectory);
	cFileUtil.FullPath(&mszDirectory);
	ReadDirectory();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::Kill(void)
{
	mszDirectory.Kill();
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::ReadDirectory(void)
{
	CFileUtil			cFileUtil;
	CArrayString		aszFileNames;
	int					i;
	CChars*				pszFileName;
	CSystemFileNode*	pcNode;
	CChars				szName;

	aszFileNames.Init(256);
	cFileUtil.FindAllFiles(mszDirectory.Text(), &aszFileNames, TRUE, FALSE);

	for (i = 0; i < aszFileNames.NumElements(); i++)
	{
		pszFileName = aszFileNames.Get(i);
		cFileUtil.MakeNameFromDirectory(&szName, pszFileName, &mszDirectory);

		pcNode = mcNames.AddFile(szName.Text());
		pcNode->File()->SetFullName(pszFileName->Text());
		szName.Kill();
	}

	aszFileNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFileSystem::GetFileName(char* szFileName)
{
	CSystemFileNode*	pcNode;

	pcNode = mcNames.GetNode(szFileName);
	if (pcNode)
	{
		return pcNode->File()->GetFullName();
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
CFileNodeSystemFile* CFileSystem::GetFileNode(char* szName, CChars* pszRemainingName)
{
	return mcNames.GetNearestFile(szName, pszRemainingName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::GetFileExtension(CArraySystemFileNodePtrs* paFileNodePtrs, char* szExtension)
{
	RecurseGetFileExtension(paFileNodePtrs, szExtension, mcNames.GetRoot());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* CFileSystem::GetFullDirectoryName(void)
{
	return &mszDirectory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::GetFiles(CArraySystemFilePtrs* pcSystemFiles)
{
	RecurseGetFiles(mcNames.GetRoot(), pcSystemFiles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::RecurseGetFiles(CSystemFileNode* pcNode, CArraySystemFilePtrs* pcSystemFiles)
{
	CSystemFileNode*	pcChild;
	int					i;

	if (pcNode->IsDirectory())
	{
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CSystemFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			RecurseGetFiles(pcChild, pcSystemFiles);
		}
	}
	else if (pcNode->IsFile())
	{
		pcSystemFiles->Add(&pcNode);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::RecurseGetFileExtension(CArraySystemFileNodePtrs* paFileNodePtrs, char* szExtension, CSystemFileNode* pcDirectoryNode)
{
	CSystemDirectoryNode*		pcDirectory;
	int							i;
	CSystemFileNode*			pcNode;
	CFileNodeSystemFile*		pcFile;
	CChars						szFake;

	if (pcDirectoryNode->IsDirectory())
	{
		pcDirectory = pcDirectoryNode->Directory();
		for (i = 0; i < pcDirectory->maNodeFiles.NumElements(); i++)
		{
			pcNode = (CSystemFileNode*)pcDirectory->maNodeFiles.Get(i);
			if (pcNode->IsDirectory())
			{
				RecurseGetFileExtension(paFileNodePtrs, szExtension, pcNode);
			}
			else if (pcNode->IsFile())
			{
				pcFile = pcNode->File();
				szFake.Fake(pcFile->GetExtension());
				if (szFake.Equals(szExtension))
				{
					paFileNodePtrs->Add(&pcFile);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodeSystemFile* CFileSystem::StartIteration(CFileNodeIterator* psIter)
{
	CSystemFileNode*	pcSystemFileNode;

	pcSystemFileNode = mcNames.GetRoot();
	psIter->Init();
	psIter->Push(pcSystemFileNode);
	return Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodeSystemFile* CFileSystem::Iterate(CFileNodeIterator* psIter)
{
	SFileSystemIteratorPosition*	psCurrent;
	int								iDirectoryElements;
	CSystemFileNode*				pcChild;
	
	psCurrent = psIter->Peek();
	if (!psCurrent)
	{
		return NULL;
	}

	iDirectoryElements = psCurrent->pcNode->Directory()->maNodeFiles.NumElements();
	psCurrent->iIndex++;

	if (psCurrent->iIndex < iDirectoryElements)
	{
		pcChild = (CSystemFileNode*)psCurrent->pcNode->Directory()->maNodeFiles.Get(psCurrent->iIndex);
		if (pcChild->IsDirectory())
		{
			psIter->Push(pcChild);
			return Iterate(psIter);
		}
		else
		{
			psIter->SetCurrent(pcChild);
			return psIter->Current();
		}
	}
	else
	{
		psIter->Pop();
		return Iterate(psIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::StopIteration(CFileNodeIterator* psIter)
{
	psIter->Kill();
}

