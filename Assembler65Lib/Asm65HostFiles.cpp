/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/PointerFunctions.h"
#include "Asm65Memory.h"
#include "Asm65HostFiles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::Init(void)
{
	mapsFiles.Init();
	mszListedDirectory.Init();
	mszDirectory.Init();
	maszListed.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The directory relative paths are in (an absolute path).  The program's current directory.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::SetDirectory(const char* szDirectory)
{
	CFileUtil	cFileUtil;

	mszDirectory.Kill();
	mszDirectory.Init(szDirectory);
	cFileUtil.FullPath(&mszDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::Kill(void)
{
	size	i;

	for (i = 0; i < mapsFiles.NumElements(); i++)
	{
		Close((uint32)(i + 1));
	}
	maszListed.Kill();
	mszDirectory.Kill();
	mszListedDirectory.Kill();
	mapsFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a command from the device's registers.  Returns the result for the result registers.
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65HostFiles::Command(CAsm65Memory* pcMemory, uint8 uiCommand, uint32* auiArguments)
{
	CFileUtil		cFileUtil;
	SAsm65HostFile*	psFile;
	CChars			szPath;
	CChars			szOther;
	uint64			uiResult;
	uint32			uiAddress;
	uint32			i;
	int64			iValue;
	EFileSeekOrigin	eOrigin;
	uint8			auiChunk[ASM65_HOST_CHUNK];
	uint32			uiChunk;
	uint32			uiRead;
	uint32			uiWritten;

	szPath.Init();
	uiResult = 0;
	switch (uiCommand)
	{
	case ASM65_HOST_OPEN:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = Open(szPath.Text(), auiArguments[1]);
		break;

	case ASM65_HOST_CLOSE:
		uiResult = Close(auiArguments[0]) ? 1 : 0;
		break;

	case ASM65_HOST_READ:
		psFile = GetFile(auiArguments[0]);
		if (psFile)
		{
			uiAddress = auiArguments[1];
			uiResult = 0;
			while (uiResult < auiArguments[2])
			{
				uiChunk = ((auiArguments[2] - (uint32)uiResult) < ASM65_HOST_CHUNK) ? (auiArguments[2] - (uint32)uiResult) : ASM65_HOST_CHUNK;
				uiRead = (uint32)psFile->cFile.Read(auiChunk, 1, uiChunk);
				for (i = 0; i < uiRead; i++)
				{
					pcMemory->SetByte(uiAddress + (uint32)uiResult + i, auiChunk[i]);
				}
				uiResult += uiRead;
				if (uiRead < uiChunk)
				{
					break;
				}
			}
		}
		break;

	case ASM65_HOST_WRITE:
		psFile = GetFile(auiArguments[0]);
		if (psFile)
		{
			if (psFile->bAppend)
			{
				psFile->cFile.Seek(0, EFSO_END);
			}
			uiAddress = auiArguments[1];
			uiResult = 0;
			while (uiResult < auiArguments[2])
			{
				uiChunk = ((auiArguments[2] - (uint32)uiResult) < ASM65_HOST_CHUNK) ? (auiArguments[2] - (uint32)uiResult) : ASM65_HOST_CHUNK;
				for (i = 0; i < uiChunk; i++)
				{
					auiChunk[i] = pcMemory->GetByte(uiAddress + (uint32)uiResult + i);
				}
				uiWritten = (uint32)psFile->cFile.Write(auiChunk, 1, uiChunk);
				uiResult += uiWritten;
				if (uiWritten < uiChunk)
				{
					break;
				}
			}
		}
		break;

	case ASM65_HOST_SEEK:
		psFile = GetFile(auiArguments[0]);
		if (psFile && (auiArguments[3] <= 2))
		{
			iValue = (int64)((uint64)auiArguments[1] | ((uint64)auiArguments[2] << 32));
			eOrigin = (auiArguments[3] == 0) ? EFSO_SET : (auiArguments[3] == 1) ? EFSO_CURRENT : EFSO_END;
			uiResult = psFile->cFile.Seek(iValue, eOrigin) ? 1 : 0;
		}
		break;

	case ASM65_HOST_TELL:
		psFile = GetFile(auiArguments[0]);
		uiResult = psFile ? (uint64)psFile->cFile.Tell() : (uint64)-1;
		break;

	case ASM65_HOST_FILE_LENGTH:
		psFile = GetFile(auiArguments[0]);
		uiResult = psFile ? (uint64)psFile->cFile.Size() : 0;
		break;

	case ASM65_HOST_TRUNCATE:
		psFile = GetFile(auiArguments[0]);
		if (psFile)
		{
			iValue = (int64)((uint64)auiArguments[1] | ((uint64)auiArguments[2] << 32));
			uiResult = psFile->cFile.Truncate(iValue) ? 1 : 0;
		}
		break;

	case ASM65_HOST_FLUSH:
		psFile = GetFile(auiArguments[0]);
		uiResult = (psFile && psFile->cFile.Flush()) ? 1 : 0;
		break;

	case ASM65_HOST_DELETE:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.Delete(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_EXISTS:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.Exists(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_SIZE_OF_FILE:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = (uint64)cFileUtil.Size(szPath.Text());
		break;

	case ASM65_HOST_MAKE_DIRECTORY:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.MakeDir(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_REMOVE_DIRECTORY:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.RemoveDir(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_IS_DIRECTORY:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.IsDirectory(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_TOUCH:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		uiResult = cFileUtil.Touch(szPath.Text()) ? 1 : 0;
		break;

	case ASM65_HOST_CURRENT_DIRECTORY:
		if (mszDirectory.Empty())
		{
			cFileUtil.CurrentDirectory(&szPath);
		}
		else
		{
			szPath.Append(mszDirectory);
		}
		uiResult = WriteString(pcMemory, auiArguments[0], auiArguments[1], szPath.Text());
		break;

	case ASM65_HOST_FIND_FILE:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		szOther.Init();
		uiResult = FindFile(szPath.Text(), auiArguments[1], &szOther);
		if (uiResult != 0)
		{
			WriteString(pcMemory, auiArguments[2], auiArguments[3], szOther.Text());
		}
		szOther.Kill();
		break;

	case ASM65_HOST_RENAME:
		ReadPath(pcMemory, auiArguments[0], &szPath);
		szOther.Init();
		ReadPath(pcMemory, auiArguments[1], &szOther);
		uiResult = (rename(szPath.Text(), szOther.Text()) == 0) ? 1 : 0;
		szOther.Kill();
		break;
	}
	szPath.Kill();
	return uiResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The zero terminated string at uiAddress (at most ASM65_HOST_MAX_PATH characters).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::ReadString(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* psz)
{
	uint32	i;
	char	c;

	for (i = 0; i < ASM65_HOST_MAX_PATH; i++)
	{
		c = (char)pcMemory->GetByte(uiAddress + i);
		if (c == 0)
		{
			break;
		}
		psz->Append(c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A path from the program: a relative one is in SetDirectory's directory.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::ReadPath(CAsm65Memory* pcMemory, uint32 uiAddress, CChars* psz)
{
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init();
	ReadString(pcMemory, uiAddress, &szPath);
	if (!mszDirectory.Empty() && !cFileUtil.IsAbsolutePath(szPath.Text()))
	{
		psz->Append(mszDirectory);
		cFileUtil.AppendToPath(psz, szPath.Text());
	}
	else
	{
		psz->Append(szPath);
	}
	szPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes sz and its zero into uiSize bytes at uiAddress.  Returns the length written (0 if it does not fit).
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65HostFiles::WriteString(CAsm65Memory* pcMemory, uint32 uiAddress, uint32 uiSize, const char* sz)
{
	uint32	uiLength;
	uint32	i;

	uiLength = (uint32)strlen(sz);
	if (uiLength + 1 > uiSize)
	{
		return 0;
	}
	for (i = 0; i <= uiLength; i++)
	{
		pcMemory->SetByte(uiAddress + i, (uint8)sz[i]);
	}
	return uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65HostFile* CAsm65HostFiles::GetFile(uint32 uiHandle)
{
	if ((uiHandle == 0) || (uiHandle > mapsFiles.NumElements()))
	{
		return NULL;
	}
	return mapsFiles.GetPtr(uiHandle - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Create opens the file if it exists (Windows OPEN_ALWAYS); truncate then empties it.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65HostFiles::Open(const char* szPath, uint32 uiMode)
{
	SAsm65HostFile*	psFile;
	EFileMode		eMode;
	bool			bRead;
	bool			bWrite;
	bool			bCreate;
	size			i;

	bRead = (uiMode & ASM65_HOST_MODE_READ) != 0;
	bWrite = (uiMode & ASM65_HOST_MODE_WRITE) != 0;
	bCreate = (uiMode & ASM65_HOST_MODE_CREATE) != 0;
	if (bRead && bWrite)
	{
		eMode = bCreate ? EFM_ReadWrite_Create : EFM_ReadWrite;
	}
	else if (bWrite)
	{
		eMode = bCreate ? EFM_Write_Create : EFM_ReadWrite;
	}
	else if (bRead)
	{
		eMode = bCreate ? EFM_Read_Create : EFM_Read;
	}
	else
	{
		return 0;
	}

	psFile = NewMalloc<SAsm65HostFile>();
	psFile->cFile.Init(szPath);
	psFile->bAppend = (uiMode & ASM65_HOST_MODE_APPEND) != 0;
	if (!psFile->cFile.Open(eMode))
	{
		psFile->cFile.Kill();
		SafeFree(psFile);
		return 0;
	}
	if (uiMode & ASM65_HOST_MODE_TRUNCATE)
	{
		psFile->cFile.Truncate(0);
	}

	for (i = 0; i < mapsFiles.NumElements(); i++)
	{
		if (mapsFiles.GetPtr(i) == NULL)
		{
			mapsFiles.SetPtr(i, psFile);
			return (uint32)(i + 1);
		}
	}
	mapsFiles.Add(psFile);
	return (uint32)mapsFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65HostFiles::Close(uint32 uiHandle)
{
	SAsm65HostFile*	psFile;

	psFile = GetFile(uiHandle);
	if (!psFile)
	{
		return false;
	}
	psFile->cFile.Close();
	psFile->cFile.Kill();
	SafeFree(psFile);
	mapsFiles.SetPtr(uiHandle - 1, NULL);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The entry uiIndex of the directory (not "." or ".."), directories and files in the order NTFS lists them.
// The directory is listed again when uiIndex is 0.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65HostFiles::FindFile(const char* szDirectory, uint32 uiIndex, CChars* pszName)
{
	char*	szEntry;

	if ((uiIndex == 0) || !mszListedDirectory.Equals(szDirectory))
	{
		ListDirectory(szDirectory);
	}
	if (uiIndex >= maszListed.NumElements())
	{
		return 0;
	}
	szEntry = maszListed.GetText(uiIndex);
	pszName->Append(&szEntry[1]);
	return (szEntry[0] == 'D') ? 2 : 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65HostFiles::ListDirectory(const char* szDirectory)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszDirectories;
	CArrayChars	aszFiles;
	CChars		szEntry;
	CChars		szName;
	size		iDirectory;
	size		iFile;
	bool		bDirectory;
	char*		szPath;

	maszListed.Kill();
	maszListed.Init();
	mszListedDirectory.Kill();
	mszListedDirectory.Init(szDirectory);

	aszDirectories.Init();
	aszFiles.Init();
	cFileUtil.FindAllDirectories(szDirectory, &aszDirectories, true);
	cFileUtil.FindAllFiles(szDirectory, &aszFiles, false, true);

	//Each list is in NTFS order; merge them.
	iDirectory = 0;
	iFile = 0;
	while ((iDirectory < aszDirectories.NumElements()) || (iFile < aszFiles.NumElements()))
	{
		if (iDirectory >= aszDirectories.NumElements())
		{
			bDirectory = false;
		}
		else if (iFile >= aszFiles.NumElements())
		{
			bDirectory = true;
		}
		else
		{
			szName.Init(aszDirectories.GetText(iDirectory));
			cFileUtil.RemovePath(&szName);
			szEntry.Init(aszFiles.GetText(iFile));
			cFileUtil.RemovePath(&szEntry);
			bDirectory = CompareFileNamesNTFS(szName.Text(), szEntry.Text()) <= 0;
			szEntry.Kill();
			szName.Kill();
		}

		szPath = bDirectory ? aszDirectories.GetText(iDirectory++) : aszFiles.GetText(iFile++);
		szName.Init(szPath);
		cFileUtil.RemovePath(&szName);
		szEntry.Init(bDirectory ? "D" : "F");
		szEntry.Append(szName);
		maszListed.Add(szEntry.Text());
		szEntry.Kill();
		szName.Kill();
	}
	aszFiles.Kill();
	aszDirectories.Kill();
}

