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
#include "PointerFunctions.h"
#include "FileBasic.h"
#include "FileUtil.h"
#include "Logger.h"
#include "LogString.h"
#include "DurableFile.h"
#include "DurableSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Init(char* szMarkStartFile, char* szMarkRewriteFile)
{
	mapcFiles.Init();
	mszMarkStart.Init(szMarkStartFile);
	mszMarkRewrite.Init(szMarkRewriteFile);
	mbBegun = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Kill(void)
{
	mapcFiles.Kill();
	mszMarkStart.Kill();
	mszMarkRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Recover(void)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	//This assumes begin has NOT been called.
	if (mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot recover after begun.", NULL);
	}

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	//If neither then in theory everything is okay.
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return CheckFilesIdentical(true, true);
	}
	else 

	//Primary files were written but backup failed.
	if ((bMarkStart) && (bMarkRewrite))
	{
		gcLogger.Info2(__METHOD__, " Primary files were written but backup failed.", NULL);
		if (!CopyPrimaryToBackup())
		{
			return gcLogger.Error2(__METHOD__, " Copying primary files to backup failed.", NULL);
		}
		MarkFinish();
		return true;
	}
	else

	//Primary files were not written.  Use old backup.
	if (bMarkStart)
	{
		gcLogger.Info2(__METHOD__, " Primary files were not written.  Reverting to backup.", NULL);
		if (!CopyBackupToPrimary())
		{
			return gcLogger.Error2(__METHOD__, " Copying backup files to primary failed.", NULL);
		}
		MarkFinish();
		return true;
	}
	else

	//Something bad happened
	if (bMarkRewrite)
	{
		return gcLogger.Error2(__METHOD__, " Rewrite has occurred without write.", NULL);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Begin(void)
{
	if (mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot begin.  Already begun.", NULL);
	}

	if (CheckWriteStatus(true))
	{
		mbBegun = true;
	}
	else
	{
		mbBegun = false;
	}
	return mbBegun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::End(void)
{
	int				i;
	CDurableFile*	pcDurable;
	bool			bResult;

	if (!mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot end.  Not begun.", NULL);
	}

	bResult = true;
	MarkStart();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Commit();
	}
	ReturnOnFalse(bResult);

	bResult = true;
	MarkRewrite();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Recommit();
	}
	ReturnOnFalse(bResult);

	MarkFinish();
	mbBegun = false;

	mapcFiles.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Check(bool bThorough, bool bLogError)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return CheckFilesIdentical(bThorough, bLogError);
	}

	return CheckWriteStatus(bMarkStart, bMarkRewrite, bLogError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckWriteStatus(bool bLogError)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	return CheckWriteStatus(bMarkStart, bMarkRewrite, bLogError);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckWriteStatus(bool bMarkStart, bool bMarkRewrite, bool bLogError)
{
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return true;
	}
	else if ((bMarkStart) && (bMarkRewrite))
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Primary files were written but re-write failed.", NULL);
		}
		return false;
	}
	else

	if (bMarkStart)
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Primary files were not written.", NULL);
		}
		return false;
	}
	else
	if (bMarkRewrite)
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Rewrite has occurred without write.", NULL);
		}
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Add(CDurableFile* pcFile)
{
	int		iIndex;
	bool	bResult;

	bResult = mapcFiles.FindInSorted(&pcFile, ComparePtrPtr, &iIndex);
	if (!bResult)
	{
		mapcFiles.InsertAt(&pcFile, iIndex);
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
int CDurableSet::Num(void)
{
	return mapcFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CDurableSet::Get(int iIndex)
{
	CDurableFile*	pcDurable;

	pcDurable = *mapcFiles.Get(iIndex);
	return pcDurable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::MarkStart(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.Touch(mszMarkStart.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::MarkRewrite(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.Touch(mszMarkRewrite.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::MarkFinish(void)
{
	CFileUtil	cFileUtil;

	cFileUtil.Delete(mszMarkStart.Text());
	cFileUtil.Delete(mszMarkRewrite.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckFilesIdentical(bool bThorough, bool bLogError)
{
	int				i;
	CDurableFile*	pcDurable;
	bool			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CheckIdentical(bThorough, bLogError);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyBackupToPrimary(void)
{
	int				i;
	CDurableFile*	pcDurable;
	bool			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyBackupToPrimary();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyPrimaryToBackup(void)
{
	int				i;
	CDurableFile*	pcDurable;
	bool			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyPrimaryToBackup();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::HasBegun(void)
{
	return mbBegun;
}

