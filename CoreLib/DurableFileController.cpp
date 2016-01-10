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
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Init(char* szDirectory, char* szRewriteDirectory, BOOL bDurable)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;

	if (bDurable && szRewriteDirectory == NULL)
	{
		gcLogger.Error2(__METHOD__, "Rewrite directory must be supplied if controller is durable.", NULL);
		return FALSE;
	}

	mbDurable = bDurable;

	mszDirectory.Init(szDirectory);
	szStart.Init(szDirectory);
	cFileUtil.AppendToPath(&szStart, "Mark1.Write");

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
	cFileUtil.AppendToPath(&szRewrite, "Mark2.Rewrite");

	mcDurableSet.Init(szStart.Text(), szRewrite.Text());

	szRewrite.Kill();
	szStart.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::Kill(void)
{
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
	return mcDurableSet.Begin();
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
BOOL CDurableFileController::Recover(void)
{
	return mcDurableSet.Recover();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::AddFile(CDurableFile* pcFile)
{
	mcDurableSet.Add(pcFile);
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
BOOL CDurableFileController::MakeDir(char* szPathName)
{
	CFileUtil cFileUtil;

	return cFileUtil.MakeDir(szPathName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::RemoveDir(char* szPathName)
{
	CFileUtil cFileUtil;

	return cFileUtil.RemoveDir(szPathName);
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

