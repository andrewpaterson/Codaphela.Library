/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/StackMemory.h"
#include "DurableFile.h"
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Init(char* szDirectory)
{
	return Init(szDirectory, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Init(char* szDirectory, char* szRewriteDirectory, char* szFileListWrite, char* szFileListRewrite, char* szMarkWrite, char* szMarkRewrite)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;
	BOOL		bResult;

	if (StrEmpty(szDirectory))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory must be supplied.", NULL);
	}

	mbDurable = FALSE;
	if (!StrEmpty(szRewriteDirectory))
	{
		mbDurable = TRUE;
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
		szFileListWrite = "FileNames.Write";
	}
	if (!szFileListRewrite)
	{
		szFileListRewrite = "FileNames.Rewrite";
	}
	mcFileList.Init(this, szFileListWrite, szFileListRewrite);
	
	mcNameMap.Init(FALSE, TRUE);

	//bResult = ReadControlledFileList(&mcFileList);
	//if (!bResult)
	//{
	//	return FALSE;
	//}

	bResult = Check(FALSE, FALSE);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
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
BOOL CDurableFileController::Begin(void)
{
	BOOL			bResult;

	bResult = mcDurableSet.Begin();
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Begin(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile* pc;
	int				iCount;

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
BOOL CDurableFileController::End(void)
{
	return mcDurableSet.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::WriteControlledFileList(CDurableFile* pcFile)
{
	unsigned int	uiFileCount;
	BOOL			bResult;
	SMapIterator	sIter;
	BOOL			bExists;
	char*			pcName;
	int*			piWriteOrRewrite;

	pcFile->Truncate(0);

	uiFileCount = 0;
	bExists = mcNameMap.StartIteration(&sIter, (void**)&pcName, NULL, (void**)&piWriteOrRewrite, NULL);
	while (bExists)
	{
		bResult = pcFile->WriteInt(DURABLE_FILE_MAGIC);
		bResult &= pcFile->WriteInt(uiFileCount);
		bResult &= pcFile->WriteString(pcName);
		bResult &= pcFile->WriteInt(*piWriteOrRewrite);

		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " File list could not be written.", NULL);
		}

		//Uh... why did this not iterate?
		bExists = mcNameMap.Iterate(&sIter, (void**)&pcName, NULL, (void**)&piWriteOrRewrite, NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::ReadControlledFileList(CDurableFile* pcFile)
{
	unsigned int	uiFileMagic;
	BOOL			bRead;
	unsigned int	uiFileNumber;
	unsigned int	uiFileCount;
	BOOL			bResult;
	CStackMemory<>	cStack;
	int				iFileNameLength;
	char*			szFileName;
	int				iWriteOrRewrite;
	
	uiFileCount = 0;
	bRead = pcFile->ReadInt(&uiFileMagic);
	while (bRead)
	{
		if (uiFileMagic != DURABLE_FILE_MAGIC)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read DURABLE_FILE_MAGIC but read [0x", IntToString(uiFileMagic, 16), "] instead.", NULL);
		}
		
		uiFileNumber = -1;
		bResult = pcFile->ReadInt(&uiFileNumber);
		if (uiFileNumber != uiFileCount)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read file number [", IntToString(uiFileCount), "] but read number [", IntToString(uiFileNumber), "].", NULL);
		}

		iFileNameLength = 0;
		bResult = pcFile->ReadStringLength(&iFileNameLength);
		if (bResult)
		{
			szFileName = (char*)cStack.Init(iFileNameLength + 1);
			bResult = pcFile->ReadStringChars(szFileName, iFileNameLength);
			if (bResult)
			{
				bResult = pcFile->ReadInt(&iWriteOrRewrite);
				mcNameMap.Put(szFileName, iWriteOrRewrite);
			}
			cStack.Kill();
		}
		if (!bResult)
		{
			return gcLogger.Error2(__METHOD__, " File list is corrupt.  Expected to read file name for file number [", IntToString(uiFileCount), "] but could not.", NULL);
		}

		//Uh... why did this not loop?
		bRead = pcFile->ReadInt(&uiFileMagic);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Recover(void)
{
	return mcDurableSet.Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Recover(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	int				iCount;

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
BOOL CDurableFileController::Check(BOOL bThorough, BOOL bLogError)
{
	return mcDurableSet.Check(bThorough, bLogError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::AddFile(CDurableFile* pcFile)
{
	return mcDurableSet.Add(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDurableFileController::NumFiles(void)
{
	return mcDurableSet.Num();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CDurableFileController::GetFile(int iIndex)
{
	return mcDurableSet.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::IsBegun(void) { return mcDurableSet.HasBegun(); }

char* CDurableFileController::GetDirectory(void) { return mszDirectory.Text(); }

char* CDurableFileController::GetRewriteDirectory(void) { return mszRewriteDirectory.Text(); }

BOOL CDurableFileController::IsDurable(void) { return mbDurable; }

CDurableSet* CDurableFileController::GetDurableSet(void) { return &mcDurableSet; }

