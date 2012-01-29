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
#include <stdio.h>
#include "FileUtil.h"
#include "ErrorTypes.h"
#include "FileHeader.h"
#include "ArrayString.h"
#include "DiskFile.h"
#include "FileCopier.h"
#include "PackFile.h"
#include "PackFiles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::Init(CAbstractFile* pcFile, EPackFileMode eMode)
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
		return FALSE;
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
BOOL CPackFiles::IsWriteMode(void)
{
	return meMode == PFM_Write;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::IsReadMode(void)
{
	return meMode == PFM_Read;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::ChangeReadFiles(CPackFileNode* psPackFile)
{
	filePos		iReadPos;

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
	return TRUE;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPackFiles::PrivateRead(CPackFileNode* psPackFile, void* pvBuffer, int iSize, int iCount)
{
	filePos iRemaining;
	filePos	iSizeToRead;

	iRemaining = (psPackFile->FilePos() + psPackFile->Size()) - miPosition;
	iSizeToRead = iSize * iCount;

	if (iSizeToRead < iRemaining)
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
		iCount = (int)(iRemaining / iSize);
		iSizeToRead = iSize * iCount;
		miPosition += iSizeToRead;
		return mcFile.Read(pvBuffer, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPackFiles::Read(CPackFileNode* psPackFile, void* pvBuffer, int iSize, int iCount)
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
BOOL CPackFiles::PrivateSeek(CPackFileNode* psPackFile, filePos iOffset, int iSeekOrigin)
{
	filePos	iAbsoluteOffset;

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
		return FALSE;
	}

	if (iAbsoluteOffset < psPackFile->FilePos())
	{
		iAbsoluteOffset = psPackFile->FilePos();
	}
	else if (iAbsoluteOffset >= psPackFile->FilePos() + psPackFile->Size())
	{
		return FALSE;
	}

	mcFile.Seek(iAbsoluteOffset, EFSO_SET);
	miPosition = iAbsoluteOffset;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::Seek(CPackFileNode* psPackFile, filePos iOffset, int iSeekOrigin)
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
				return FALSE;
			}
			return PrivateSeek(psPackFile, iOffset, iSeekOrigin);
		}
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
int CPackFiles::Write(CPackFileNode* psPackFile, const void* pvBuffer, int iSize, int iCount)
{
	int		iWritten;

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
BOOL CPackFiles::Eof(CPackFileNode* psPackFile)
{
	filePos	iPosition;

	if (meMode == PFM_Read)
	{
		iPosition = Tell(psPackFile);
		if (iPosition == -1)
		{
			return TRUE;
		}

		if (iPosition >= psPackFile->Size())
		{
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::Flush(CPackFileNode* psPackFile)
{
	if (meMode == PFM_Write)
	{
		mcFile.Flush();
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
CPackFile* CPackFiles::WriteOpen(char* szFileName)
{
	CFileNodePackFileNode*	pcFileNode;
	CPackFile*				pcFile;
	BOOL					bResult;

	if (mpsLastAccessed != NULL)
	{
		//Can only write to one file at a time;
		return NULL;
	}

	if (meMode == PFM_Write)
	{
		pcFileNode = GetNode(szFileName);
		if (pcFileNode != NULL)
		{
			
			//Can't write to an existing file.
			return NULL;
		}

		pcFileNode = AddFile(szFileName);
		if (pcFileNode)
		{
			pcFile = PackFile(this, pcFileNode->File());
			bResult = pcFile->Open(EFM_Write_Create);
			if (!bResult)
			{
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
CPackFile* CPackFiles::ReadOpen(char* szFileName)
{
	CFileNodePackFileNode*	pcFileNode;
	CPackFile*		pcFile;
	BOOL			bResult;
	filePos			iPosition;

	if (meMode == PFM_Read)
	{
		pcFileNode = GetNode(szFileName);
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
BOOL CPackFiles::Close(CPackFile* pcPackFile)
{
	if (meMode == PFM_Write)
	{
		mcFile.Flush();
	}
	mpsLastAccessed = NULL;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::AddFile(CAbstractFile* pcFile, char* szFileName)
{
	CPackFile*		pcPackFile;
	CFileCopier		cCopier;
	BOOL			bResult;
	
	pcPackFile = WriteOpen(szFileName);
	if (!pcPackFile)
	{
		return FALSE;
	}

	pcPackFile->mbBasicFileMustFree = FALSE;
	bResult = cCopier.Copy(pcFile, pcPackFile);
	pcPackFile->Close();
	pcPackFile->Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::AddDirectory(char* szDirectory)
{
	CFileUtil		cFileUtil;
	CArrayString	aszFileNames;
	int				i;
	CChars*			pszFileName;
	CDiskFile		cDiskFile;
	CChars			szName;
	CChars			szFakeDirectory;
	BOOL			bResult;
	BOOL			bAnyFiles;

	aszFileNames.Init(256);
	bAnyFiles = cFileUtil.FindAllFiles(szDirectory, &aszFileNames, TRUE, FALSE);

	if (!bAnyFiles)
	{
		aszFileNames.Kill();
		return FALSE;
	}

	szFakeDirectory.Fake(szDirectory);

	for (i = 0; i < aszFileNames.NumElements(); i++)
	{
		pszFileName = aszFileNames.Get(i);
		cDiskFile.Init(pszFileName->Text());
		cFileUtil.MakeNameFromDirectory(&szName, pszFileName, &szFakeDirectory);
		
		bResult = AddFile(&cDiskFile, szName.Text());
		szName.Kill();
		cDiskFile.Kill();

		if (!bResult)
		{
			aszFileNames.Kill();
			return FALSE;
		}
	}

	aszFileNames.Kill();
	return TRUE;
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
BOOL CPackFiles::BeginRead(void)
{
	maReads.Init();

	ReturnOnFalse(mcFile.Open(EFM_Read));

	ReturnOnFalse(ReadHeader());
	ReturnOnFalse(ReadNodes());
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::BeginWrite(void)
{
	ReturnOnFalse(mcFile.Open(EFM_ReadWrite_Create));

	if (mcFile.GetFileSize() > 0)
	{
		ReturnOnFalse(ReadHeader());
		ReturnOnFalse(ReadNodes());

		return TRUE;
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
BOOL CPackFiles::EndRead(void)
{
	maReads.Kill();
	return mcFile.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::EndWrite(void)
{
	ReturnOnFalse(WriteNodes());
	return mcFile.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPackFiles::ClassName(void)
{
	return "CPackFiles";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::WriteHeader(void)
{
	CFileHeader	cFileHeader;

	ReturnOnFalse(cFileHeader.Save(&mcFile, ClassName()));
	ReturnOnFalse(mcFile.Write(&miNodes, sizeof(filePos), 1));

	miPosition = mcFile.GetFilePos();
	miNextNodesPtr = miPosition - sizeof(filePos);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::ReadHeader(void)
{
	CFileHeader	cFileHeader;

	ReturnOnFalse(cFileHeader.Load(&mcFile));
	ReturnOnFalse(cFileHeader.IsVersion(ENGINE_VERSION));
	ReturnOnFalse(cFileHeader.IsType(ClassName()));

	ReturnOnFalse(mcFile.ReadLong(&miNodes));

	miPosition = mcFile.GetFilePos();
	miNextNodesPtr = miPosition - sizeof(filePos);
	
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::ReadNodes(void)
{
	filePos		iPosition;
	int			i;
	int			iNumFiles;
	BOOL		bResult;

	iPosition = miPosition;
	if (miNodes != 0)
	{
		for (;;)
		{
			if (miNodes != iPosition)
			{
				mcFile.Seek(miNodes, EFSO_SET);
			}
			ReturnOnFalse(mcFile.ReadInt(&iNumFiles));

			for (i = 0; i < iNumFiles; i++)
			{
				ReturnOnFalse(ReadNode());
			}

			miNextNodesPtr = miPosition;
			bResult = mcFile.ReadLong(&miNodes);
			if (!bResult)
			{
				break;
			}
		}
		mcFile.Seek(iPosition);
		miPosition = iPosition;
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
BOOL CPackFiles::ReadNode(void)
{
	CChars			sz;
	CFileNodePackFileNode*	pcNode;

	sz.Init();

	ReturnOnFalse(mcFile.ReadString(&sz));

	pcNode = AddFile(sz.Text());
	sz.Kill();

	if (!((pcNode) && (pcNode->IsFile())))
	{
		return FALSE;
	}

	ReturnOnFalse(pcNode->File()->Load(&mcFile));

	miPosition = mcFile.GetFilePos();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::WriteNodes(void)
{
	filePos	iPosition;
	int		iNumFiles;

	if (miNextNodesPtr == 0)
	{
		return FALSE;
	}

	iPosition = mcFile.GetFileSize();

	mcFile.Seek(miNextNodesPtr, EFSO_SET);
	ReturnOnFalse(mcFile.WriteLong(iPosition));

	mcFile.Seek(iPosition, EFSO_SET);
	iNumFiles = GetNumUnwrittenNames();
	ReturnOnFalse(mcFile.WriteInt(iNumFiles));
	return WriteUnwrittenNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPackFiles::GetNumUnwrittenNames(void)
{
	return RecurseGetNumUnwrittenNames(mcNames.GetRoot());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPackFiles::RecurseGetNumUnwrittenNames(CFileNodePackFileNode* pcNode)
{
	CFileNodePackFileNode*	pcChild;
	int				i;
	int				iNumUnwritten;

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
BOOL CPackFiles::WriteUnwrittenNames(void)
{
	CChars	szPath;
	BOOL	bResult;

	szPath.Init(1024);
	bResult = RecurseWriteUnwrittenNames(mcNames.GetRoot(), &szPath);
	szPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::RecurseWriteUnwrittenNames(CFileNodePackFileNode* pcNode, CChars* pszPath)
{
	CFileNodePackFileNode*	pcChild;
	int				i;
	BOOL			bResult;
	int				iPathLength;
	CChars			szPath;

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
				return FALSE;
			}
		}
		return TRUE;
	}
	else if (pcNode->IsFile())
	{
		if (!pcNode->File()->IsNameWritten())
		{
			szPath.Init(pszPath->Text(1));
			ReturnOnFalse(mcFile.WriteString(&szPath));
			szPath.Kill();
			return pcNode->File()->Save(&mcFile);
		}
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
BOOL CPackFiles::Unpack(char* szDestination)
{
	FixParents();
	return RecurseUnpack(mcNames.GetRoot(), szDestination);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFiles::RecurseUnpack(CFileNodePackFileNode* pcNode, char* szDestination)
{
	CChars					szFileName;
	CFileNodePackFileNode*	pcChild;
	int						i;
	CPackFileNode*			pcFile;
	BOOL					bResult;
	CPackFile*				pcPackFile;
	CFileCopier				cCopier;
	CDiskFile				cDiskFile;
	CFileUtil				cFileUtil;

	if (pcNode->IsDirectory())
	{
		bResult = TRUE;
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CFileNodePackFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			bResult &= RecurseUnpack(pcChild, szDestination);
		}
		return bResult;
	}
	else if (pcNode->IsFile())
	{
		szFileName.Init(szDestination);
		szFileName.Append('/');
		pcNode->GetFullName(&szFileName);
		szFileName.Replace('/', FILE_SEPARATOR[0]);

		pcFile = pcNode->File();
		pcPackFile = PackFile(this, pcFile);

		if (!pcPackFile)
		{
			szFileName.Kill();
			return FALSE;
		}

		cDiskFile.Init(szFileName.Text());

		cFileUtil.RemoveLastFromPath(&szFileName);
		cFileUtil.MakeDir(szFileName.Text());
		bResult = cCopier.Copy(pcPackFile, &cDiskFile);

		cDiskFile.Kill();

		szFileName.Kill();
		return bResult;
	}
	return FALSE;
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
	RecurseGetFiles(mcNames.GetRoot(), pcPackFiles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFiles::RecurseGetFiles(CFileNodePackFileNode* pcNode, CArrayPackFileNodePtrs* pcPackFiles)
{
	CFileNodePackFileNode*	pcChild;
	int						i;

	if (pcNode->IsDirectory())
	{
		for (i = 0; i < pcNode->Directory()->maNodeFiles.NumElements(); i++)
		{
			pcChild = (CFileNodePackFileNode*)pcNode->Directory()->maNodeFiles.Get(i);
			RecurseGetFiles(pcChild, pcPackFiles);
		}
	}
	else if (pcNode->IsFile())
	{
		pcPackFiles->Add(&pcNode);
	}
}

