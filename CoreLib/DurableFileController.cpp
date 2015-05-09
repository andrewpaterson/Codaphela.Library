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
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::Init(char* szWorkingDirectory, BOOL bDurable)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;

	mszWorkingDirectory.Init(szWorkingDirectory);
	szStart.Init(szWorkingDirectory);
	szRewrite.Init(szWorkingDirectory);

	cFileUtil.AppendToPath(&szStart, "Mark1.Write");
	cFileUtil.AppendToPath(&szRewrite, "Mark2.Rewrite");

	mcDurableSet.Init(szStart.Text(), szRewrite.Text());
	szRewrite.Kill();
	szStart.Kill();

	mbDurable = bDurable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFileController::Kill(void)
{
	mcDurableSet.Kill();
	mszWorkingDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFileController::Begin(void)
{
	if (IsDurable())
	{
		return mcDurableSet.Begin();
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
BOOL CDurableFileController::End(void)
{
	if (IsDurable())
	{
		return mcDurableSet.End();
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
void CDurableFileController::AddFile(CDurableFile* pcFile)
{
	mcDurableSet.Add(pcFile);
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

char* CDurableFileController::GetWorkingDirectory(void) { return mszWorkingDirectory.Text(); }

BOOL CDurableFileController::IsDurable(void) { return mbDurable; }

