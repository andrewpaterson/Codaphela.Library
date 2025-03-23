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
#include "Logger.h"
#include "LogString.h"
#include "StackMemory.h"
#include "DurableFile.h"
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Init(char* szDirectory)
{
	return Init(szDirectory, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Init(char* szDirectory, char* szRewriteDirectory, char* szFileListWrite, char* szFileListRewrite, char* szMarkWrite, char* szMarkRewrite)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;
	bool		bResult;

	if (StrEmpty(szDirectory))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory must be supplied.", NULL);
	}

	mbDurable = false;
	if (!StrEmpty(szRewriteDirectory))
	{
		mbDurable = true;
	}

	if (mbDurable && (StringInsensitiveCompare(szDirectory, szRewriteDirectory) == 0))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory and rewrite directory must be different.", NULL);
	}

	mszDirectory.Init(szDirectory);
	szStart.Init(szDirectory);
	if (!szMarkWrite)
	{
		szMarkWrite = "Mark1.Write";
	}
	cFileUtil.AppendToPath(&szStart, szMarkWrite);

	if (szRewriteDirectory && mbDurable)
	{
		mszRewriteDirectory.Init(szRewriteDirectory);
		szRewrite.Init(szRewriteDirectory);
	}
	else
	{
		mszRewriteDirectory.Init();
		szRewrite.Init(szDirectory);
	}
	if (!szMarkRewrite)
	{
		szMarkRewrite = "Mark2.Rewrite";
	}
	cFileUtil.AppendToPath(&szRewrite, szMarkRewrite);

	mcDurableSet.Init(szStart.Text(), szRewrite.Text());

	szRewrite.Kill();
	szStart.Kill();

	if (!szFileListWrite)
	{
		szFileListWrite = "Filenames.Write";
	}
	if (!szFileListRewrite)
	{
		szFileListRewrite = "Filenames.Rewrite";
	}
	mcFileList.Init(this, szFileListWrite, szFileListRewrite);
	
	mcNameMap.Init(false, true);

	//bResult = ReadControlledFileList(&mcFileList);
	//if (!bResult)
	//{
	//	return false;
	//}

	bResult = Check(false, false);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::Kill(void)
{
	mcFileList.Kill();
	mcNameMap.Kill();

	mcDurableSet.Kill();

	mszRewriteDirectory.Kill();
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Begin(void)
{
	bool			bResult;

	bResult = mcDurableSet.Begin();
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Begin(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	size			iCount;

	iCount = 0;
	pc = pcFirst;

	va_start(vaMarker, pcFirst);
	while (pc != NULL)
	{
		pc->AddFile();
		iCount++;
		pc = va_arg(vaMarker, CDurableFile*);
	}
	va_end(vaMarker);

	return Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::End(void)
{
	return mcDurableSet.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::WriteControlledFileList(CDurableFile* pcFile)
{
	uint32			uiFileCount;
	bool			bResult;
	SMapIterator	sIter;
	bool			bExists;
	char*			pcName;
	int32*			piWriteOrRewrite;

	pcFile->Truncate(0);

	uiFileCount = 0;
	bExists = mcNameMap.StartIteration(&sIter, (void**)&pcName, NULL, (void**)&piWriteOrRewrite, NULL);
	while (bExists)
	{
		bResult = pcFile->WriteInt32(DURABLE_FILE_MAGIC);
		bResult &= pcFile->WriteInt32(uiFileCount);
		bResult &= pcFile->WriteString(pcName);
		bResult &= pcFile->WriteInt32(*piWriteOrRewrite);

		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " File list could not be written.", NULL);
		}

		//Uh... why did this not iterate?
		bExists = mcNameMap.Iterate(&sIter, (void**)&pcName, NULL, (void**)&piWriteOrRewrite, NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::ReadControlledFileList(CDurableFile* pcFile)
{
	uint32			uiFileMagic;
	bool			bRead;
	uint32			uiFileNumber;
	uint32			uiFileCount;
	bool			bResult;
	CStackMemory<>	cStack;
	size			iFilenameLength;
	char*			szFilename;
	int32			iWriteOrRewrite; 
	
	uiFileCount = 0;
	bRead = pcFile->ReadInt32(&uiFileMagic);
	while (bRead)
	{
		if (uiFileMagic != DURABLE_FILE_MAGIC)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read DURABLE_FILE_MAGIC but read [0x", IntToString(uiFileMagic, 16), "] instead.", NULL);
		}
		
		uiFileNumber = -1;
		bResult = pcFile->ReadInt32(&uiFileNumber);
		if (uiFileNumber != uiFileCount)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read file number [", IntToString(uiFileCount), "] but read number [", IntToString(uiFileNumber), "].", NULL);
		}

		iFilenameLength = 0;
		bResult = pcFile->ReadStringLength(&iFilenameLength);
		if (bResult)
		{
			szFilename = (char*)cStack.Init(iFilenameLength + 1);
			bResult = pcFile->ReadStringChars(szFilename, iFilenameLength);
			if (bResult)
			{
				bResult = pcFile->ReadInt32(&iWriteOrRewrite);
				mcNameMap.Put(szFilename, iWriteOrRewrite);
			}
			cStack.Kill();
		}
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read file name for file number [", IntToString(uiFileCount), "] but could not.", NULL);
		}

		//Uh... why did this not loop?
		bRead = pcFile->ReadInt32(&uiFileMagic);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Recover(void)
{
	return mcDurableSet.Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Recover(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	size			iCount;

	iCount = 0;
	pc = pcFirst;

	va_start(vaMarker, pcFirst);
	while (pc != NULL)
	{
		pc->AddFile();
		iCount++;
		pc = va_arg(vaMarker, CDurableFile*);
	}
	va_end(vaMarker);

	return Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::Check(bool bThorough, bool bLogError)
{
	return mcDurableSet.Check(bThorough, bLogError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::AddFile(CDurableFile* pcFile)
{
	return mcDurableSet.Add(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDurableFileController::NumFiles(void)
{
	return mcDurableSet.Num();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CDurableFileController::GetFile(size iIndex)
{
	return mcDurableSet.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFileController::IsBegun(void) { return mcDurableSet.HasBegun(); }

char* CDurableFileController::GetDirectory(void) { return mszDirectory.Text(); }

char* CDurableFileController::GetRewriteDirectory(void) { return mszRewriteDirectory.Text(); }

bool CDurableFileController::IsDurable(void) { return mbDurable; }

CDurableSet* CDurableFileController::GetDurableSet(void) { return &mcDurableSet; }

