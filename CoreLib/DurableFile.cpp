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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/ConstructorCall.h"
#include "DurableFile.h"
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::Init(CDurableFileController* pcController, char* szFileName, char* szRewriteName)
{
	mpcController = pcController;
	InitBasic();

	if (!StrEmpty(szFileName))
	{
		mcPrimaryDiskFile.Init(szFileName);
		mcLogFile.Init(&mcPrimaryDiskFile);
		mcPrimaryFile.Init(&mcLogFile);
		mszFileName.Init(mcPrimaryDiskFile.mszFileName);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Primary DurableFile file name is NULL.", NULL);
	}

	if (!StrEmpty(szRewriteName))
	{
		mcRewriteDiskFile.Init(szRewriteName);
		mcRewriteFile.Init(&mcRewriteDiskFile);
		mszRewriteName.Init(mcRewriteDiskFile.mszFileName);
	}
	else
	{
		mszRewriteName.Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::InitBasic(void)
{
	meOpenMode = EFM_Unknown;
	mbOpenedSinceBegin = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Kill(void)
{
	BOOL	bAnyOpen;

	bAnyOpen = mcPrimaryFile.IsOpen();
	bAnyOpen |= mcRewriteFile.IsOpen();

	mcPrimaryFile.Kill();
	mcRewriteFile.Kill();

	mcLogFile.Kill();

	mszFileName.Kill();
	mszRewriteName.Kill();

	return !bAnyOpen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenPrimaryFile(BOOL bOpenForWrite)
{
	BOOL	bFileOpened;

	if (!bOpenForWrite)
	{
		bFileOpened = OpenPrimaryForRead();
	}
	else
	{
		bFileOpened = OpenPrimaryForWrite();
	}

	if (!mbOpenedSinceBegin)
	{
		mbOpenedSinceBegin = TRUE;
		mpcController->AddFile(this);
	}

	return bFileOpened;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Commit(void)
{
	BOOL		bResult;
	CFileUtil	cFileUtil;

	if (IsDurable())
	{
		if (!IsBegun())
		{
			return FALSE;
		}

		if (mcLogFile.GetNumCommands() == 0)
		{
			mcLogFile.Close();

			InitBasic();
			return TRUE;
		}

		cFileUtil.TouchDir(mszFileName.Text());
		//bResult = mpcPrimaryFile->Open(EFM_ReadWrite_Create);
		//if (!bResult)
		//{
		//	gcLogger.Error2(__METHOD__, " Could not open durable file [", mszFileName.Text(), "] for writing during commit.", NULL);
		//	return FALSE;
		//}

		bResult = mcLogFile.Commit();
		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Commit durable file [", mszFileName.Text(), "] failed.", NULL);
			return FALSE;
		}

		InitBasic();
		mcPrimaryFile.Flush();
		mcPrimaryFile.Close();
		return TRUE;
	}
	else
	{
		InitBasic();
		mcPrimaryFile.Flush();
		mcPrimaryFile.Close();
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Recommit(void)
{
	BOOL		bResult;
	CFileUtil	cFileUtil;

	if (IsDurable())
	{
		if (mcRewriteFile.IsOpen())
		{
			gcLogger.Error2(__METHOD__, " Did not expect durable file [", mszRewriteName.Text(), "] to be open already.", NULL);
			return FALSE;
		}

		cFileUtil.TouchDir(mszRewriteName.Text());
		//bResult = mcRewriteFile.Open(EFM_ReadWrite_Create);
		//if (!bResult)
		//{
		//	gcLogger.Error2(__METHOD__, " Could not open durable file [", mszRewriteName.Text(), "] for re-write.", NULL);
		//	return FALSE;
		//}

		bResult = mcLogFile.Commit(&mcRewriteDiskFile);
		if (!bResult)
		{
			return FALSE;
		}

		//This is dodgy as fuck.  Clear the log file.  Don't kill and reinitialise it.
		mcLogFile.Kill();
		mcLogFile.Init(&mcPrimaryDiskFile);

		mcRewriteFile.Flush();
		mcRewriteFile.Close();

		return TRUE;
	}
	else
	{
		//This is dodgy as fuck.  Clear the log file.  Don't kill and reinitialise it.
		mcLogFile.Kill();
		mcLogFile.Init(&mcPrimaryDiskFile);

		return TRUE;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Seek(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite)
{
	BOOL	bResult;

	if (!IsBegun())
	{
		return FALSE;
	}

	bResult = OpenPrimaryFile(bSeekForWrite);
	if (!bResult)
	{
		return FALSE;
	}

	if (IsDurable())
	{
		return SeekDurable(eOrigin, iDistance, bSeekForWrite);
	}
	else
	{
		return SeekNonDurable(eOrigin, iDistance, bSeekForWrite);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::SeekDurable(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite)
{
	return mcLogFile.Seek(iDistance, eOrigin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::SeekNonDurable(EFileSeekOrigin eOrigin, filePos iDistance, BOOL bSeekForWrite)
{
	return mcPrimaryFile.Seek(iDistance, eOrigin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenPrimaryForWrite(void)
{
	BOOL		bResult;
	CFileUtil	cFileUtil;
	
	if (meOpenMode == EFM_Unknown)
	{
		if (IsDurable())
		{
			bResult = mcPrimaryFile.Open(EFM_ReadWrite_Create);
		}
		else
		{
			cFileUtil.TouchDir(mszFileName.Text());
			bResult = mcPrimaryFile.Open(EFM_ReadWrite_Create);
		}

		if (!bResult)
		{
			meOpenMode = EFM_Unknown;
			return TRUE;
		}
		else
		{
			meOpenMode = EFM_ReadWrite;
			return TRUE;
		}
	}
	else if (meOpenMode == EFM_Read)
	{
		if (mcPrimaryFile.IsOpen())
		{
			mcPrimaryFile.Close();
		}

		bResult = mcPrimaryFile.Open(EFM_ReadWrite);
		if (!bResult)
		{
			meOpenMode = EFM_Unknown;
			return TRUE;
		}
		else
		{
			meOpenMode = EFM_ReadWrite;
			return TRUE;
		}
	}
	else if (meOpenMode == EFM_ReadWrite)
	{
		meOpenMode = EFM_ReadWrite;
		return TRUE;
	}
	else
	{
		//File is in a borked mode.  This should never happen.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenPrimaryForRead(void)
{
	BOOL	bResult;

	if (meOpenMode == EFM_Unknown)
	{
		bResult = mcPrimaryFile.Open(EFM_Read);
		if (bResult)
		{
			meOpenMode = EFM_Read;
			return TRUE;
		}
		else
		{
			meOpenMode = EFM_Unknown;
			return TRUE;
		}
	}
	else if (meOpenMode == EFM_Read)
	{
		meOpenMode = EFM_Read;
		return TRUE;
	}
	else if (meOpenMode == EFM_ReadWrite)
	{
		meOpenMode = EFM_ReadWrite;
		return TRUE;
	}
	else
	{
		//File is in a borked mode.  This should never happen.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	return Write(EFSO_SET, iDistance, pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	BOOL bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = Seek(eOrigin, iDistance, TRUE);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFileName.Text(), "] that cannot Seek.", NULL);
		return 0;
	}

	return Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	BOOL bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = OpenPrimaryFile(TRUE);
	if (!bResult)
	{
		return FALSE;
	}

	if (IsDurable())
	{
		return WriteDurable(pvSource, iSize, iCount);
	}
	else
	{
		return WriteNonDurable(pvSource, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::WriteDurable(const void* pvSource, filePos iSize, filePos iCount)
{
	return mcLogFile.Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::WriteNonDurable(const void* pvSource, filePos iSize, filePos iCount)
{
	return mcPrimaryFile.Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	return Read(EFSO_SET, iDistance, pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	BOOL	bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read from CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = Seek(eOrigin, iDistance, FALSE);
	if (!bResult)
	{
		if (meOpenMode == EFM_Unknown)
		{
			return 0;
		}
		gcLogger.Error2(__METHOD__, " Cannot read from a CDurableFile [", mszFileName.Text(), "] that cannot Seek.", NULL);
		return 0;
	}

	return Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Read(void* pvDest, filePos iSize, filePos iCount)
{
	BOOL bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read from CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = OpenPrimaryFile(FALSE);
	if (!bResult)
	{
		return FALSE;
	}

	if (IsDurable())
	{
		return ReadDurable(pvDest, iSize, iCount);
	}
	else
	{
		return ReadNonDurable(pvDest, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::ReadDurable(void* pvDest, filePos iSize, filePos iCount)
{
	return mcLogFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::ReadNonDurable(void* pvDest, filePos iSize, filePos iCount)
{
	if (mcPrimaryFile.IsOpen())
	{
		return mcPrimaryFile.Read(pvDest, iSize, iCount);
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Tell(void)
{
	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot tell from CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return -1;
	}

	if (!OpenPrimaryFile(FALSE))
	{
		return 0;
	}

	if (IsDurable())
	{
		return mcLogFile.Tell();
	}
	else
	{
		if (mcPrimaryFile.IsOpen())
		{
			return mcPrimaryFile.GetFilePos();
		}
		else
		{
			return 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Size(void)
{
	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot size from CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return -1;
	}

	if (!OpenPrimaryFile(FALSE))
	{
		return 0;
	}

	if (IsDurable())
	{
		return SizeDurable();
	}
	else
	{
		return SizeNonDurable();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::SizeDurable(void)
{
	return mcLogFile.Size();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::SizeNonDurable(void)
{
	BOOL bResult;

	bResult = OpenPrimaryFile(FALSE);
	if (!bResult)
	{
		return -1;
	}

	if (mcPrimaryFile.IsOpen())
	{
		return mcPrimaryFile.GetFileSize();
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
BOOL CDurableFile::IsDurable(void)
{
	if (mpcController != NULL)
	{
		return mpcController->IsDurable();
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
BOOL CDurableFile::IsBegun(void)
{
	if (mpcController != NULL)
	{
		return mpcController->IsBegun();
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
filePos CDurableFile::TestGetPosition(void)
{
	return mcLogFile.Tell();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::TestGetLength(void)
{
	return mcLogFile.Size();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::TestGetOpenedSinceBegin(void)
{
	return mbOpenedSinceBegin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDurableFile::GetNumWrites(void)
{
	return mcLogFile.GetNumWrites();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDurableFile::GetWriteData(int iWrite)
{
	return mcLogFile.GetWriteData(iWrite)->GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CheckIdentical(void)
{
	CFileUtil	cFileUtil;

	if (IsBegun())
	{
		return FALSE;
	}

	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Delete(void)
{
	CFileUtil	cFileUtil;
	BOOL		bResult;

	if (IsBegun())
	{
		return FALSE;
	}

	if (IsDurable())
	{
		bResult = cFileUtil.Delete(mszFileName.Text());
		bResult &= cFileUtil.Delete(mszRewriteName.Text());
		return bResult;
	}
	else
	{
		return cFileUtil.Delete(mszFileName.Text());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CopyBackupToPrimary(void)
{
	CFileUtil	cFileUtil;

	if (IsBegun())
	{
		return FALSE;
	}

	if (!cFileUtil.Copy(mszRewriteName.Text(), mszFileName.Text()))
	{
		return FALSE;
	}
	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CopyPrimaryToBackup(void)
{
	CFileUtil	cFileUtil;

	if (IsBegun())
	{
		return FALSE;
	}

	if (!cFileUtil.Copy(mszFileName.Text(), mszRewriteName.Text()))
	{
		return FALSE;
	}

	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::Dump(void)
{
	mcLogFile.Dump();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileBasic*	CDurableFile::DumpGetPrimaryFile(void)
{
	return &mcPrimaryFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FindSecondString(char* szFileName)
{
	int		        iLen;
	int		        i;
	unsigned char*	szSecondStart;

	iLen = (int)strlen(szFileName);
	szSecondStart = (unsigned char*)RemapSinglePointer(szFileName, iLen+1);
	for (i = 0; i < 32768; i++)
	{
		if (i >= 1)
		{
			if (szSecondStart[i] == 0)
			{
				return (char*)szSecondStart;
			}
		}
		if ((szSecondStart[i] < 32) || (szSecondStart[i] >= 128))
		{
			return NULL;
		}
	}
	return NULL;
}

