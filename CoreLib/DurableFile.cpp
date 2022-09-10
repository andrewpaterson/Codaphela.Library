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
bool CDurableFile::Init(CDurableFileController* pcController, char* szFilename, char* szRewriteName)
{
	CChars		szPath;
	CFileUtil	cFileUtil;
		
	mbAddedToController = false;
	mpcController = pcController;
	InitBasic();

	if (StrEmpty(szFilename))
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
	cFileUtil.AppendToPath(&szPath, szFilename);

	mszFilename.Init(szPath);
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
			cFileUtil.AppendToPath(&szPath, szFilename);
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

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::InitBasic(void)
{
	meOpenMode = EFM_Unknown;
	mbLogFileBegun = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::InitError(void)
{
	mszFilename.Init();
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
bool CDurableFile::Kill(void)
{
	bool	bAnyOpen;

	bAnyOpen = mcPrimaryFile.IsOpen();
	mcPrimaryFile.Kill();
	mcPrimaryDiskFile.Kill();
	mszFilename.Kill();

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
	mbAddedToController = true;
	if (mpcController)
	{
		mpcController->AddFile(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Commit(void)
{
	bool		bResult;

	if (!IsBegun())
	{
		return false;
	}

	if (mcLogFile.GetNumCommands() == 0)
	{
		InitBasic();
		return true;
	}

	mcPrimaryFile.Close();

	bResult = mcLogFile.Commit();
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Commit durable file [", mszFilename.Text(), "] failed.", NULL);
	}

	InitBasic();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Recommit(void)
{
	bool		bResult;

	if (IsDurable())
	{
		if (mcRewriteFile.IsOpen())
		{
			gcLogger.Error2(__METHOD__, " Did not expect durable file [", mszRewriteName.Text(), "] to be open already.", NULL);
			return false;
		}

		if (mcLogFile.GetNumCommands() == 0)
		{
			return true;
		}

		bResult = mcLogFile.Commit(&mcRewriteDiskFile);
		if (!bResult)
		{
			return false;
		}

		mcLogFile.End();
		return true;
	}
	else
	{
		if (mcLogFile.IsBegun())
		{
			mcLogFile.End();
		}
		return true;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Seek(EFileSeekOrigin eOrigin, filePos iDistance, bool bSeekForWrite)
{
	bool	bResult;

	if (!IsBegun())
	{
		return false;
	}

	bResult = OpenPrimaryFile(bSeekForWrite);
	if (!bResult)
	{
		return false;
	}

	return mcPrimaryFile.Seek(iDistance, eOrigin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::OpenPrimaryFile(bool bOpenForWrite)
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
bool CDurableFile::OpenPrimaryForWrite(void)
{
	bool		bResult;
	bool		bFileExists;
	
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
			return true;
		}
		else
		{
			meOpenMode = EFM_ReadWrite;
			return true;
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
			return true;
		}
		else
		{
			meOpenMode = EFM_ReadWrite;
			return true;
		}
	}
	else if (meOpenMode == EFM_ReadWrite)
	{
		meOpenMode = EFM_ReadWrite;
		return true;
	}
	else
	{
		//File is in a borked mode.  This should never happen.
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::OpenPrimaryForRead(void)
{
	bool	bResult;
	bool	bFileExists;

	if (!mbLogFileBegun)
	{
		bFileExists = mcLogFile.Begin();
		if (!bFileExists)
		{
			mbLogFileBegun = false;
			meOpenMode = EFM_Unknown;
			return false;
		}
		else
		{
			mbLogFileBegun = true;
		}
	}

	if (meOpenMode == EFM_Unknown)
	{
		bResult = mcPrimaryFile.Open(EFM_Read);
		if (bResult)
		{
			meOpenMode = EFM_Read;
			return true;
		}
		else
		{
			meOpenMode = EFM_Unknown;
			return false;
		}
	}
	else if (meOpenMode == EFM_Read)
	{
		meOpenMode = EFM_Read;
		return true;
	}
	else if (meOpenMode == EFM_ReadWrite)
	{
		meOpenMode = EFM_ReadWrite;
		return true;
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
	bool bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = Seek(eOrigin, iDistance, true);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFilename.Text(), "] that cannot Seek.", NULL);
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
	bool bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot write to a CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = OpenPrimaryFile(true);
	if (!bResult)
	{
		return false;
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
	bool	bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read from CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = Seek(eOrigin, iDistance, false);
	if (!bResult)
	{
		if (meOpenMode == EFM_Unknown)
		{
			return 0;
		}
		gcLogger.Error2(__METHOD__, " Cannot read from a CDurableFile [", mszFilename.Text(), "] that cannot Seek.", NULL);
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
	bool bResult;

	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot read from CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
		return 0;
	}

	bResult = OpenPrimaryFile(false);
	if (!bResult)
	{
		return false;
	}

	return mcPrimaryFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Create(void)
{
	return OpenPrimaryFile(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Exists(void)
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
		 gcLogger.Error2(__METHOD__, " Cannot tell from CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
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
		gcLogger.Error2(__METHOD__, " Cannot size from CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
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
bool CDurableFile::Truncate(filePos iSize)
{
	if (!IsBegun())
	{
		gcLogger.Error2(__METHOD__, " Cannot truncate CDurableFile [", mszFilename.Text(), "] that is not Begun.", NULL);
		return false;
	}

	return mcPrimaryFile.Truncate(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::IsDurable(void)
{
	if (mpcController != NULL)
	{
		return mpcController->IsDurable();
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
bool CDurableFile::IsBegun(void)
{
	if (mpcController != NULL)
	{
		return mpcController->IsBegun();
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::TestGetOpenedSinceBegin(void)
{
	return mbLogFileBegun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::IsOpen(void)
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
bool CDurableFile::CheckIdentical(bool bThorough, bool bLogError)
{
	CFileUtil	cFileUtil;
	bool		bResult;

	if (IsBegun())
	{
		return false;
	}

	bResult = cFileUtil.Compare(mszFilename.Text(), mszRewriteName.Text());
	if (bLogError && !bResult)
	{
		return gcLogger.Error2(__METHOD__, " File mismatch [", mszFilename.Text(), "] and [", mszRewriteName.Text(), "].", NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::Delete(void)
{
	CFileUtil	cFileUtil;
	bool		bResult;

	if (IsBegun())
	{
		return false;
	}

	if (IsDurable())
	{
		bResult = cFileUtil.Delete(mszFilename.Text());
		bResult &= cFileUtil.Delete(mszRewriteName.Text());
		return bResult;
	}
	else
	{
		return cFileUtil.Delete(mszFilename.Text());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::CopyBackupToPrimary(void)
{
	CFileUtil	cFileUtil;

	if (IsBegun())
	{
		return false;
	}

	if (!cFileUtil.Copy(mszRewriteName.Text(), mszFilename.Text()))
	{
		return false;
	}
	return cFileUtil.Compare(mszFilename.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableFile::CopyPrimaryToBackup(void)
{
	CFileUtil	cFileUtil;

	if (IsBegun())
	{
		return false;
	}

	if (!cFileUtil.Copy(mszFilename.Text(), mszRewriteName.Text()))
	{
		return false;
	}

	return cFileUtil.Compare(mszFilename.Text(), mszRewriteName.Text());
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
char* CDurableFile::GetFilename(void)
{
	return mszFilename.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CDurableFile::GetRewriteName(void)
{
	return mszRewriteName.Text();
}

