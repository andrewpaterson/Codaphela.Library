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
	CArrayChars			aszFilenames;
	size				i;
	CChars*				pszFilename;
	CSystemFileNode*	pcNode;
	CChars				szName;

	aszFilenames.Init();
	cFileUtil.FindAllFiles(mszDirectory.Text(), &aszFilenames, true, false);

	for (i = 0; i < aszFilenames.NumElements(); i++)
	{
		pszFilename = aszFilenames.Get(i);
		cFileUtil.MakeNameFromDirectory(&szName, pszFilename, &mszDirectory);

		pcNode = mcNames.AddFile(szName.Text());
		pcNode->File()->SetFullName(pszFilename->Text());
		szName.Kill();
	}

	aszFilenames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFileSystem::GetFilename(char* szFilename)
{
	CSystemFileNode*	pcNode;

	pcNode = mcNames.GetNode(szFilename);
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
void CFileSystem::GetFiles(CArraySystemFilePtrs* pcSystemFiles)
{
	CFileSystemIterator		cIter;
	CSystemFileNode*		pcFile;

	pcFile = StartIteration(&cIter);
	while (pcFile)
	{
		pcSystemFiles->Add(&pcFile);
		pcFile = Iterate(&cIter);
	}
	StopIteration(&cIter);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileSystem::GetFiles(CArraySystemFilePtrs* pcSystemFiles, char* szExtension)
{
	CFileSystemIterator		cIter;
	CSystemFileNode*		pcNode;
	CFileNodeSystemFile*	pcFile;
	CChars					szFake;

	pcNode = StartIteration(&cIter);
	while (pcNode)
	{
		pcFile = pcNode->File();
		szFake.Fake(pcFile->GetExtension());
		if (szFake.Equals(szExtension))
		{
			pcSystemFiles->Add(&pcNode);
		}

		pcNode = Iterate(&cIter);
	}
	StopIteration(&cIter);
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
CSystemFileNode* CFileSystem::StartIteration(CFileSystemIterator* psIter)
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
CSystemFileNode* CFileSystem::Iterate(CFileSystemIterator* psIter)
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
void CFileSystem::StopIteration(CFileSystemIterator* psIter)
{
	psIter->Kill();
}

