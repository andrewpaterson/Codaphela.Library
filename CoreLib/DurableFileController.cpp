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
BOOL CDurableFileController::Init(char* szDirectory, char* szRewriteDirectory)
{
	CChars		szStart;
	CChars		szRewrite;
	CFileUtil	cFileUtil;

	if (StrEmpty(szDirectory))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory must be supplied.", NULL);
	}

	mbDurable = FALSE;
	if (!StrEmpty(szRewriteDirectory))
	{
		mbDurable = TRUE;
	}

	if (mbDurable && (StrICmp(szDirectory, szRewriteDirectory) == 0))
	{
		return gcLogger.Error2(__METHOD__, " Controller directory and rewrite directory must be different.", NULL);
	}

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
BOOL CDurableFileController::Begin(CDurableFile* pcFirst, ...)
{
	va_list			vaMarker;
	CDurableFile*	pc;
	int				iCount;
	BOOL			bResult;

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

	bResult = Check();
	if (!bResult)
	{
		return FALSE;
	}

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
BOOL CDurableFileController::Check(void)
{
	return mcDurableSet.Check(TRUE);
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
BOOL CDurableFileController::IsBegun(void) { return mcDurableSet.HasBegun(); }

char* CDurableFileController::GetDirectory(void) { return mszDirectory.Text(); }

char* CDurableFileController::GetRewriteDirectory(void) { return mszRewriteDirectory.Text(); }

BOOL CDurableFileController::IsDurable(void) { return mbDurable; }

CDurableSet* CDurableFileController::GetDurableSet(void) { return &mcDurableSet; }

