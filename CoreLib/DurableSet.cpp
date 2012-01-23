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
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
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

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	//If neither then in theory everything is Okay.
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return TestFilesIdentical();
	}
	else 
	//Primary files were written but backup failed.
	if ((bMarkStart) && (bMarkRewrite))
	{
		if (!CopyPrimaryToBackup())
		{
			return FALSE;
		}
		MarkFinish();
		return TRUE;
	}
	else
	//Primary files were not written.  Use old backup.
	if (bMarkStart)
	{
		if (!CopyBackupToPrimary())
		{
			return FALSE;
		}
		MarkFinish();
		return TRUE;
	}
	else
	//Something bad happened
	if (bMarkRewrite)
	{
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Begin(void)
{
	int				i;
	CDurableFile*	pcDurable;

	mbBegun = TRUE;
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		pcDurable->Begin();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::End(void)
{
	int				i;
	CDurableFile*	pcDurable;

	MarkStart();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		pcDurable->End();
	}
	MarkRewrite();
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		pcDurable->Rewrite();
	}
	MarkFinish();
	mbBegun = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Add(CDurableFile* pcFile)
{
	mapcFiles.Add(&pcFile);
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
BOOL CDurableSet::TestFilesIdentical(void)
{
	int				i;
	CDurableFile*	pcDurable;
	BOOL			bResult;

	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->TestIdentical();
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