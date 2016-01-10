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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "DurableFile.h"
#include "DurableSet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Init(char* szMarkStartFile, char* szMarkRewriteFile)
{
	mapcFiles.Init(2048);
	mszMarkStart.Init(szMarkStartFile);
	mszMarkRewrite.Init(szMarkRewriteFile);
	mbBegun = FALSE;
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
BOOL CDurableSet::Recover(void)
{
	CFileUtil	cFileUtil;
	BOOL		bMarkStart;
	BOOL		bMarkRewrite;

	//This assumes begin has NOT been called.
	if (mbBegun)
	{
		gcLogger.Error2(__METHOD__, "Cannot recover after begun.", NULL);
		return FALSE;
	}

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	//If neither then in theory everything is okay.
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return CheckFilesIdentical();
	}
	else 

	//Primary files were written but backup failed.
	if ((bMarkStart) && (bMarkRewrite))
	{
		gcLogger.Info2(__METHOD__, "Primary files were written but backup failed.", NULL);
		if (!CopyPrimaryToBackup())
		{
			gcLogger.Error2(__METHOD__, "Copying primary files to backup failed.", NULL);
			return FALSE;
		}
		MarkFinish();
		return TRUE;
	}
	else

	//Primary files were not written.  Use old backup.
	if (bMarkStart)
	{
		gcLogger.Info2(__METHOD__, "Primary files were not written.  Reverting to backup.", NULL);
		if (!CopyBackupToPrimary())
		{
			gcLogger.Error2(__METHOD__, "Copying backup files to primary failed.", NULL);
			return FALSE;
		}
		MarkFinish();
		return TRUE;
	}
	else

	//Something bad happened
	if (bMarkRewrite)
	{
		gcLogger.Error2(__METHOD__, " Something bad happened.", NULL);
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::Begin(void)
{
	if (mbBegun)
	{
		gcLogger.Error2(__METHOD__, " Cannot begin.  Already begun.", NULL);
		return FALSE;
	}
	mbBegun = TRUE;
	mapcFiles.Kill();
	mapcFiles.Init(2048);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::End(void)
{
	int				i;
	CDurableFile*	pcDurable;
	BOOL			bResult;

	if (!mbBegun)
	{
		gcLogger.Error2(__METHOD__, " Cannot end.  Not begun.", NULL);
		return FALSE;
	}

	bResult = TRUE;
	MarkStart();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Commit();
	}
	ReturnOnFalse(bResult);

	bResult = TRUE;
	MarkRewrite();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Recommit();
	}
	ReturnOnFalse(bResult);

	MarkFinish();
	mbBegun = FALSE;

	mapcFiles.Kill();
	mapcFiles.Init(2048);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::Add(CDurableFile* pcFile)
{
	int		iIndex;
	BOOL	bResult;

	bResult = mapcFiles.FindInSorted(&pcFile, ComparePtrPtr, &iIndex);
	if (!bResult)
	{
		mapcFiles.InsertAt(&pcFile, iIndex);
		return TRUE;
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
BOOL CDurableSet::CheckFilesIdentical(void)
{
	int				i;
	CDurableFile*	pcDurable;
	BOOL			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CheckIdentical();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::CopyBackupToPrimary(void)
{
	int				i;
	CDurableFile*	pcDurable;
	BOOL			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyBackupToPrimary();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::CopyPrimaryToBackup(void)
{
	int				i;
	CDurableFile*	pcDurable;
	BOOL			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyPrimaryToBackup();
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableSet::HasBegun(void)
{
	return mbBegun;
}

