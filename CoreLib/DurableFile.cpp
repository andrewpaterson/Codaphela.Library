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
BOOL CDurableFile::Init(CDurableFileController* pcController, char* szFileName, char* szRewriteName)
{
	CChars		szPath;
	CFileUtil	cFileUtil;
		
	mbAddedToController = FALSE;
	mpcController = pcController;
	InitBasic();

	if (StrEmpty(szFileName))
	{
		InitError();
		return gcLogger.Error2(__METHOD__, " Primary DurableFile file name may not be [NULL].", NULL);
	}

	if (pcController == NULL)
	{
		InitError();
		return gcLogger.Error2(__METHOD__, " DurableFileController may not be [NULL].", NULL);
	}

	szPath.Init(pcController->GetDirectory());
	cFileUtil.AppendToPath(&szPath, szFileName);

	mszFileName.Init(szPath);
	mcPrimaryDiskFile.Init(szPath);
	mcLogFile.Init(&mcPrimaryDiskFile);
	mcPrimaryFile.Init(&mcLogFile);

	szPath.Kill();

	if (IsDurable())
	{
		szPath.Init(pcController->GetRewriteDirectory());

		if (!StrEmpty(szRewriteName))
		{
			cFileUtil.AppendToPath(&szPath, szRewriteName);
		}
		else
		{
			cFileUtil.AppendToPath(&szPath, szFileName);
		}
		mszRewriteName.Init(szPath);
		mcRewriteDiskFile.Init(szPath);
		mcRewriteFile.Init(&mcRewriteDiskFile);

		szPath.Kill();
	}
	else
	{
		mszRewriteName.Init();
		mcRewriteDiskFile.Init(NULL);
		mcRewriteFile.Init(&mcRewriteDiskFile);
	}

	//AddFile();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::InitBasic(void)
{
	meOpenMode = EFM_Unknown;
	mbLogFileBegun = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::InitError(void)
{
	mszFileName.Init();
	mcPrimaryDiskFile.Init(NULL);
	mcLogFile.Init(&mcPrimaryDiskFile);
	mcPrimaryFile.Init(&mcLogFile);

	mszRewriteName.Init();
	mcRewriteDiskFile.Init(NULL);
	mcRewriteFile.Init(&mcRewriteDiskFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Kill(void)
{
	BOOL	bAnyOpen;

	bAnyOpen = mcPrimaryFile.IsOpen();
	mcPrimaryFile.Kill();
	mcPrimaryDiskFile.Kill();
	mszFileName.Kill();

	if (IsDurable())
	{
		bAnyOpen |= mcRewriteFile.IsOpen();
	}

	mcRewriteFile.Kill();
	mcRewriteDiskFile.Kill();
	mszRewriteName.Kill();

	mcLogFile.Kill();

	return !bAnyOpen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::AddFile(void)
{
	mbAddedToController = TRUE;
	if (mpcController)
	{
		mpcController->AddFile(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Commit(void)
{
	BOOL		bResult;

	if (!IsBegun())
	{
		return FALSE;
	}

	if (mcLogFile.GetNumCommands() == 0)
	{
		InitBasic();
		return TRUE;
	}

	mcPrimaryFile.Close();

	bResult = mcLogFile.Commit();
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Commit durable file [", mszFileName.Text(), "] failed.", NULL);
	}

	InitBasic();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Recommit(void)
{
	BOOL		bResult;

	if (IsDurable())
	{
		if (mcRewriteFile.IsOpen())
		{
			gcLogger.Error2(__METHOD__, " Did not expect durable file [", mszRewriteName.Text(), "] to be open already.", NULL);
			return FALSE;
		}

		if (mcLogFile.GetNumCommands() == 0)
		{
			return TRUE;
		}

		bResult = mcLogFile.Commit(&mcRewriteDiskFile);
		if (!bResult)
		{
			return FALSE;
		}

		mcLogFile.End();
		return TRUE;
	}
	else
	{
		if (mcLogFile.IsBegun())
		{
			mcLogFile.End();
		}
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

	return mcPrimaryFile.Seek(iDistance, eOrigin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenPrimaryFile(BOOL bOpenForWrite)
{
	AddFile();

	if (!bOpenForWrite)
	{
		return OpenPrimaryForRead();
	}
	else
	{
		return OpenPrimaryForWrite();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenPrimaryForWrite(void)
{
	BOOL		bResult;
	BOOL		bFileExists;
	
	if (!mbLogFileBegun)
	{
		bFileExists = mcLogFile.Begin();
		mbLogFileBegun = bFileExists;
	}

	if (meOpenMode == EFM_Unknown)
	{
		bResult = mcPrimaryFile.Open(EFM_ReadWrite_Create);
		mbLogFileBegun = bResult;

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
	BOOL	bFileExists;

	if (!mbLogFileBegun)
	{
		bFileExists = mcLogFile.Begin();
		if (!bFileExists)
		{
			mbLogFileBegun = FALSE;
			meOpenMode = EFM_Unknown;
			return FALSE;
		}
		else
		{
			mbLogFileBegun = TRUE;
		}
	}

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
			return FALSE;
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
		return gcLogger.Error2(__METHOD__, " File is in a borked mode.  This should never happen.", NULL);
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

	return mcPrimaryFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Create(void)
{
	return OpenPrimaryFile(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Exists(void)
{
	return OpenPrimaryForRead();
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

	return mcPrimaryFile.GetFilePos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Size(void)
{
	filePos ulliSize;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot size from CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return -1;
	}

	if (!mbLogFileBegun)
	{
		if (!mcPrimaryDiskFile.IsOpen())
		{
			if (!mcPrimaryDiskFile.Open(EFM_Read))
			{
				return 0;
			}
			else
			{
				ulliSize = mcPrimaryDiskFile.Size();
				mcPrimaryDiskFile.Close();
				return ulliSize;
			}
		}
		return mcPrimaryDiskFile.Size();
	}
	else
	{
		return mcPrimaryFile.GetFileSize();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Truncate(filePos iSize)
{
	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot truncate CDurableFile [", mszFileName.Text(), "] that is not Begun.", NULL);
		return -1;
	}

	return mcPrimaryFile.Truncate(iSize);
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
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::TestGetOpenedSinceBegin(void)
{
	return mbLogFileBegun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::IsOpen(void)
{
	return mcPrimaryDiskFile.IsOpen();
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
BOOL CDurableFile::CheckIdentical(BOOL bThorough, BOOL bLogError)
{
	CFileUtil	cFileUtil;
	BOOL		bResult;

	if (IsBegun())
	{
		return FALSE;
	}

	bResult = cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
	if (bLogError && !bResult)
	{
		return gcLogger.Error2(__METHOD__, " File mismatch [", mszFileName.Text(), "] and [", mszRewriteName.Text(), "].", NULL);
	}
	return bResult;
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
char* CDurableFile::GetFileName(void)
{
	return mszFileName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDurableFile::GetRewriteName(void)
{
	return mszRewriteName.Text();
}

