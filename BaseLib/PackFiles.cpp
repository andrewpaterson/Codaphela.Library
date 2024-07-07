/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include <stdio.h>
#include "FileUtil.h"
#include "ErrorTypes.h"
#include "FileHeader.h"
#include "ArrayChars.h"
#include "DiskFile.h"
#include "FileCopier.h"
#include "PackFile.h"
#include "PackFiles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::Init(CAbstractFile* pcFile, EPackFileMode eMode)
{
	mcFile.Init(pcFile);
	meMode = eMode;
	mpsLastAccessed = NULL;
	miPosition = 0;
	miNodes = 0;
	miNextNodesPtr = 0;
	mcNames.Init();
	if (eMode == PFM_Write)
	{
		return BeginWrite();
	}
	else if (eMode == PFM_Read)
	{
		return BeginRead();
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
void CPackFiles::Kill(void)
{
	if (meMode == PFM_Read)
	{
		EndRead();
	}
	else if (meMode == PFM_Write)
	{
		EndWrite();
	}

	mcFile.Kill();
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::IsWriteMode(void)
{
	return meMode == PFM_Write;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::IsReadMode(void)
{
	return meMode == PFM_Read;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::ChangeReadFiles(CPackFileNode* psPackFile)
{
	filePos	iReadPos;

	if (mpsLastAccessed)
	{
		maReads.SetReadPos(mpsLastAccessed, miPosition);
	}
	iReadPos = maReads.GetReadPos(psPackFile);
	if (iReadPos != 0)
	{
		mcFile.Seek(iReadPos, EFSO_SET);
	}
	else
	{
		mcFile.Seek(psPackFile->FilePos(), EFSO_SET);
	}

	mpsLastAccessed = psPackFile;
	miPosition = iReadPos;
	return true;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPackFiles::PrivateRead(CPackFileNode* psPackFile, void* pvBuffer, size iSize, size iCount)
{
	size		iRemaining;
	size		iSizeToRead;
	filePos		iFilePos;

	iFilePos = psPackFile->FilePos() + psPackFile->Size() - miPosition;
	if (iFilePos < 0)
	{
		return 0;
	}

	iRemaining = (size)iFilePos;
	iSizeToRead = iSize * iCount;

	if (iSizeToRead <= iRemaining)
	{
		miPosition += iSizeToRead;
		return mcFile.Read(pvBuffer, iSize, iCount);
	}
	else if (iRemaining <= 0)
	{
		return 0;
	}
	else
	{
		iCount = iRemaining / iSize;
		iSizeToRead = iSize * iCount;
		miPosition += iSizeToRead;
		return mcFile.Read(pvBuffer, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPackFiles::Read(CPackFileNode* psPackFile, void* pvBuffer, size iSize, size iCount)
{
	if (meMode == PFM_Read)
	{
		if (mpsLastAccessed == psPackFile)
		{
			return PrivateRead(psPackFile, pvBuffer, iSize, iCount);
		}
		else
		{
			if (!ChangeReadFiles(psPackFile))
			{
				return 0;
			}
			return PrivateRead(psPackFile, pvBuffer, iSize, iCount);
		}
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
bool CPackFiles::PrivateSeek(CPackFileNode* psPackFile, filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	filePos		iAbsoluteOffset;

	if (iSeekOrigin == EFSO_SET)
	{
		iAbsoluteOffset = psPackFile->FilePos() + iOffset;
	}
	else if (iSeekOrigin == SEEK_CUR)
	{
		iAbsoluteOffset = miPosition + iOffset;
	}
	else if (iSeekOrigin == SEEK_END)
	{
		iAbsoluteOffset = psPackFile->FilePos() + psPackFile->Size() - iOffset;
	}
	else
	{
		return false;
	}

	if (iAbsoluteOffset < psPackFile->FilePos())
	{
		iAbsoluteOffset = psPackFile->FilePos();
	}
	else if (iAbsoluteOffset >= psPackFile->FilePos() + psPackFile->Size())
	{
		return false;
	}

	mcFile.Seek(iAbsoluteOffset, EFSO_SET);
	miPosition = iAbsoluteOffset;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::Seek(CPackFileNode* psPackFile, filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	//Can only seek in read mode.
	if (meMode == PFM_Read)
	{
		if (mpsLastAccessed == psPackFile)
		{
			return PrivateSeek(psPackFile, iOffset, iSeekOrigin);
		}
		else
		{
			if (!ChangeReadFiles(psPackFile))
			{
				return false;
			}
			return PrivateSeek(psPackFile, iOffset, iSeekOrigin);
		}
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
size CPackFiles::Write(CPackFileNode* psPackFile, const void* pvBuffer, size iSize, size iCount)
{
	size	iWritten;

	if (meMode == PFM_Write)
	{
		if (mpsLastAccessed == psPackFile)
		{
			if (!psPackFile->IsInitialised())
			{
				psPackFile->SetFilePos(miPosition);
			}

			iWritten = mcFile.Write(pvBuffer, iSize, iCount);
			psPackFile->IncreaseSize(iWritten * iSize);
			miPosition += iWritten * iSize;
			return iWritten;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFiles::Tell(CPackFileNode* psPackFile)
{
	if (mpsLastAccessed == psPackFile)
	{
		return miPosition - psPackFile->FilePos();
	}
	else
	{
		if (meMode == PFM_Read)
		{
			if (mpsLastAccessed == psPackFile)
			{
				return miPosition - psPackFile->FilePos();
			}
			else
			{
				if (!ChangeReadFiles(psPackFile))
				{
					return -1;
				}
				return miPosition - psPackFile->FilePos();
			}
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::Flush(void)
{
	if (meMode == PFM_Write)
	{
		mcFile.Flush();
		return true;
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
CPackFile* CPackFiles::WriteOpen(char* szFilename)
{
	CFileNodePackFileNode*	pcFileNode;
	CPackFile*				pcFile;
	bool					bResult;

	if (mpsLastAccessed != NULL)
	{
		//Can only write to one file at a time;
		return NULL;
	}

	if (meMode == PFM_Write)
	{
		pcFileNode = GetNode(szFilename);
		if (pcFileNode != NULL)
		{
			//Can't write to an existing file.
			return NULL;
		}

		pcFileNode = AddFile(szFilename);
		if (pcFileNode)
		{
			pcFile = PackFile(this, pcFileNode->File());
			bResult = pcFile->Open(EFM_Write_Create);
			if (!bResult)
			{
				free(pcFile);
				return NULL;
			}
			mpsLastAccessed = pcFileNode->File();
			return pcFile;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFile* CPackFiles::ReadOpen(char* szFilename)
{
	CFileNodePackFileNode*	pcFileNode;
	CPackFile*				pcFile;
	bool					bResult;
	filePos					iPosition;

	if (meMode == PFM_Read)
	{
		pcFileNode = GetNode(szFilename);
		if ((pcFileNode == NULL) || (!pcFileNode->IsFile()))
		{
			return NULL;
		}

		iPosition = maReads.GetReadPos(pcFileNode->File());
		if (iPosition != 0)
		{
			return NULL;
		}
		maReads.BeginReadPos(pcFileNode->File());

		pcFile = PackFile(this, pcFileNode->File());
		bResult = pcFile->Open(EFM_Read);
		if (!bResult)
		{
			return NULL;
		}

		if (mpsLastAccessed != NULL)
		{
			if (!ChangeReadFiles(pcFileNode->File()))
			{
				return NULL;
			}
		}

		return pcFile;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::Close(CPackFile* pcPackFile)
{
	if (meMode == PFM_Write)
	{
		mcFile.Flush();
	}
	mpsLastAccessed = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::AddFile(CAbstractFile* pcFile, char* szFilename)
{
	CPackFile*		pcPackFile;
	CFileCopier		cCopier;
	bool			bResult;
	
	pcPackFile = WriteOpen(szFilename);
	if (!pcPackFile)
	{
		return false;
	}

	pcPackFile->mbBasicFileMustFree = false;
	bResult = cCopier.Copy(pcFile, pcPackFile);
	pcPackFile->Close();
	pcPackFile->Kill();
	free(pcPackFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::AddDirectory(char* szDirectory, char* szPackDirectory)
{
	CFileUtil		cFileUtil;
	CArrayChars		aszFilenames;
	size			i;
	CChars*			pszFilename;
	CDiskFile		cDiskFile;
	CChars			szName;
	CChars			szNameDirectory;
	bool			bResult;
	bool			bAnyFiles;

	aszFilenames.Init();
	bAnyFiles = cFileUtil.FindAllFiles(szDirectory, &aszFilenames, true, false);

	if (!bAnyFiles)
	{
		aszFilenames.Kill();
		return false;
	}

	szNameDirectory.Init(szDirectory);
	for (i = 0; i < aszFilenames.NumElements(); i++)
	{
		pszFilename = aszFilenames.Get(i);
		cDiskFile.Init(pszFilename->Text());
		cFileUtil.MakeNameFromDirectory(&szName, pszFilename, &szNameDirectory);
		if (szPackDirectory)
		{
			szName.Insert(0, '/');
			szName.Insert(0, szPackDirectory);
		}

		bResult = AddFile(&cDiskFile, szName.Text());
		szName.Kill();
		cDiskFile.Kill();

		if (!bResult)
		{
			szNameDirectory.Kill();
			aszFilenames.Kill();
			return false;
		}
	}

	szNameDirectory.Kill();
	aszFilenames.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodePackFileNode* CPackFiles::GetNode(char* szFullName)
{
	return mcNames.GetNode(szFullName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodePackFileNode* CPackFiles::AddFile(char* szFullName)
{
	return mcNames.AddFile(szFullName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::BeginRead(void)
{
	maReads.Init();

	ReturnOnFalse(mcFile.Open(EFM_Read));

	ReturnOnFalse(ReadHeader());
	ReturnOnFalse(ReadNodes());
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::BeginWrite(void)
{
	ReturnOnFalse(mcFile.Open(EFM_ReadWrite_Create));

	if (mcFile.GetFileSize() > 0)
	{
		ReturnOnFalse(ReadHeader());
		ReturnOnFalse(ReadNodes());

		return true;
	}
	else
	{
		return WriteHeader();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::EndRead(void)
{
	maReads.Kill();
	return mcFile.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::EndWrite(void)
{
	ReturnOnFalse(WriteNodes());
	return mcFile.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPackFiles::ClassName(void)
{
	return "CPackFiles";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::WriteHeader(void)
{
	CFileHeader	cFileHeader;

	ReturnOnFalse(cFileHeader.Save(&mcFile, PACK_FILE_WRITER, PACK_FILE_VERSION));
	ReturnOnFalse(mcFile.Write(&miNodes, sizeof(filePos), 1));

	miPosition = mcFile.GetFilePos();
	miNextNodesPtr = miPosition - sizeof(filePos);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::ReadHeader(void)
{
	CFileHeader	cFileHeader;

	ReturnOnFalse(cFileHeader.Load(&mcFile, PACK_FILE_WRITER, PACK_FILE_VERSION));
	ReturnOnFalse(mcFile.ReadInt64(&miNodes));

	miPosition = mcFile.GetFilePos();
	miNextNodesPtr = miPosition - sizeof(filePos);
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::ReadNodes(void)
{
	filePos		iPosition;
	size		i;
	uint32		uiNumFiles;
	bool		bResult;

	iPosition = miPosition;
	if (miNodes != 0)
	{
		for (;;)
		{
			if (miNodes != iPosition)
			{
				mcFile.Seek(miNodes, EFSO_SET);
			}
			ReturnOnFalse(mcFile.ReadInt32(&uiNumFiles));

			for (i = 0; i < uiNumFiles; i++)
			{
				ReturnOnFalse(ReadNode());
			}

			miNextNodesPtr = miPosition;
			bResult = mcFile.ReadInt64(&miNodes);
			if (!bResult)
			{
				break;
			}
		}
		mcFile.Seek(iPosition);
		miPosition = iPosition;
		return true;
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
bool CPackFiles::ReadNode(void)
{
	CChars					sz;
	CFileNodePackFileNode*	pcNode;


	if (!sz.ReadString(&mcFile))
	{
		sz.Kill();
		return false;
	}

	pcNode = AddFile(sz.Text());
	sz.Kill();

	if (!((pcNode) && (pcNode->IsFile())))
	{
		return false;
	}

	ReturnOnFalse(pcNode->File()->Load(&mcFile));

	miPosition = mcFile.GetFilePos();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::WriteNodes(void)
{
	filePos		iPosition;
	uint32		iNumFiles;

	if (miNextNodesPtr == 0)
	{
		return false;
	}

	iPosition = mcFile.GetFileSize();

	mcFile.Seek(miNextNodesPtr, EFSO_SET);
	ReturnOnFalse(mcFile.WriteInt64(iPosition));

	mcFile.Seek(iPosition, EFSO_SET);
	iNumFiles = GetNumUnwrittenNames();
	ReturnOnFalse(mcFile.WriteInt32(iNumFiles));
	return WriteUnwrittenNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CPackFiles::GetNumUnwrittenNames(void)
{
	return RecurseGetNumUnwrittenNames(mcNames.GetRoot());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CPackFiles::RecurseGetNumUnwrittenNames(CFileNodePackFileNode* pcNode)
{
	CFileNodePackFileNode*	pcChild;
	size					i;
	uint32					iNumUnwritten;

	if (pcNode->IsDirectory())
	{
		iNumUnwritten = 0;
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CFileNodePackFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			iNumUnwritten += RecurseGetNumUnwrittenNames(pcChild);
		}
		return iNumUnwritten;
	}
	else if (pcNode->IsFile())
	{
		if (!pcNode->File()->IsNameWritten())
		{
			return 1;
		}
		else
		{
			return 0;
		}
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
bool CPackFiles::WriteUnwrittenNames(void)
{
	CChars	szPath;
	bool	bResult;

	szPath.Init();
	bResult = RecurseWriteUnwrittenNames(mcNames.GetRoot(), &szPath);
	szPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::RecurseWriteUnwrittenNames(CFileNodePackFileNode* pcNode, CChars* pszPath)
{
	CFileNodePackFileNode*	pcChild;
	size					i;
	bool					bResult;
	size					iPathLength;
	CChars					szPath;

	if (pcNode->IsDirectory())
	{
		iPathLength = pszPath->Length();
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CFileNodePackFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			pszPath->Append("/");
			pszPath->Append(pcChild->GetName());
			bResult = RecurseWriteUnwrittenNames(pcChild, pszPath);
			pszPath->SetLength(iPathLength);

			if (!bResult)
			{
				return false;
			}
		}
		return true;
	}
	else if (pcNode->IsFile())
	{
		if (!pcNode->File()->IsNameWritten())
		{
			szPath.Init(pszPath->Text(1));
			ReturnOnFalse(szPath.WriteString(&mcFile));
			szPath.Kill();
			return pcNode->File()->Save(&mcFile);
		}
		return true;
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
bool CPackFiles::Unpack(char* szDestination)
{
	FixParents();
	return RecurseUnpack(mcNames.GetRoot(), szDestination);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPackFiles::RecurseUnpack(CFileNodePackFileNode* pcNode, char* szDestination)
{
	CChars					szFilename;
	CFileNodePackFileNode*	pcChild;
	size					i;
	CPackFileNode*			pcFile;
	bool					bResult;
	CPackFile*				pcPackFile;
	CFileCopier				cCopier;
	CDiskFile				cDiskFile;
	CFileUtil				cFileUtil;

	if (pcNode->IsDirectory())
	{
		bResult = true;
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CFileNodePackFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			bResult &= RecurseUnpack(pcChild, szDestination);
		}
		return bResult;
	}
	else if (pcNode->IsFile())
	{
		szFilename.Init(szDestination);
		szFilename.Append('/');
		pcNode->GetFullName(&szFilename);
		szFilename.Replace('/', FILE_SEPARATOR[0]);

		pcFile = pcNode->File();
		pcPackFile = PackFile(this, pcFile);

		if (!pcPackFile)
		{
			szFilename.Kill();
			return false;
		}

		cDiskFile.Init(szFilename.Text());

		cFileUtil.RemoveLastFromPath(&szFilename);
		cFileUtil.MakeDir(szFilename.Text());
		bResult = cCopier.Copy(pcPackFile, &cDiskFile);

		cDiskFile.Kill();

		szFilename.Kill();
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFiles::FixParents(void)
{
	mcNames.FixParents();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFiles::GetFiles(CArrayPackFileNodePtrs* pcPackFiles)
{
	CPackFileIterator		cIter;
	CFileNodePackFileNode*	pcFile;

	pcFile = StartIteration(&cIter);
	while (pcFile)
	{
		pcPackFiles->Add(&pcFile);
		pcFile = Iterate(&cIter);
	}
	StopIteration(&cIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodePackFileNode* CPackFiles::StartIteration(CPackFileIterator* psIter)
{
	CFileNodePackFileNode*	pcFileNodePackFileNode;

	pcFileNodePackFileNode = mcNames.GetRoot();
	psIter->Init();
	psIter->Push(pcFileNodePackFileNode);
	return Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileNodePackFileNode* CPackFiles::Iterate(CPackFileIterator* psIter)
{
	SPackFileIteratorPosition*	psCurrent;
	size						iDirectoryElements;
	CFileNodePackFileNode*		pcChild;

	psCurrent = psIter->Peek();
	if (!psCurrent)
	{
		return NULL;
	}

	iDirectoryElements = psCurrent->pcNode->Directory()->maNodeFiles.NumElements();
	psCurrent->iIndex++;

	if (psCurrent->iIndex < iDirectoryElements)
	{
		pcChild = (CFileNodePackFileNode*)psCurrent->pcNode->Directory()->maNodeFiles.Get(psCurrent->iIndex);
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
void CPackFiles::StopIteration(CPackFileIterator* psIter)
{
	psIter->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPackFiles::GetFilename(void)
{
	return mcFile.GetFilename();
}

